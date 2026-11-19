#!/usr/bin/env python3
"""Batch-convert alien silhouette images into 2-bit (4-level) C headers for STM32F4."""

import argparse
import math
import re
from pathlib import Path
from PIL import Image


GREY_PALETTE = [
    (0, 0, 0),        # 0: Black
    (64, 64, 64),     # 1: Dark Grey
    (160, 160, 160),  # 2: Light Grey
]

SUPPORTED_EXTS = {".jpg", ".jpeg", ".png", ".webp", ".bmp"}


def clean_alien_name(stem: str, keep_raw_name: bool = False) -> str:
    name = stem.strip().lower()
    if not keep_raw_name:
        # Strip trailing short hex/alphanumeric suffixes like "f83", "f7h", "fa0"
        name = re.sub(r"(?:_|-)?[a-f0-9]{3}$", "", name)
    ident = re.sub(r"[^a-z0-9_]", "_", name)
    ident = re.sub(r"_+", "_", ident).strip("_")
    if not ident or ident[0].isdigit():
        ident = f"alien_{ident}" if ident else "alien_unknown"
    return ident


def rgb_to_2bit_index(rgb: tuple[int, int, int]) -> int:
    r, g, b = rgb
    # Gate green (index 3) strictly so edge/compression noise doesn't turn green
    if g > r + 40 and g > b + 40 and g > 80:
        return 3

    best_idx = 0
    best_dist = math.inf
    for idx, (pr, pg, pb) in enumerate(GREY_PALETTE):
        dr = r - pr
        dg = g - pg
        db = b - pb
        dist = 2 * dr * dr + 4 * dg * dg + 3 * db * db
        if dist < best_dist:
            best_dist = dist
            best_idx = idx
    return best_idx


def pack_2bit_bitmap(
    img: Image.Image,
    lsb_first: bool = False,
    alpha_threshold: int = 128,
    transparent_idx: int = 0,
) -> tuple[bytes, int, int]:
    rgba = img.convert("RGBA")
    width, height = rgba.size
    pixels = rgba.load()
    packed = bytearray()

    for y in range(height):
        row_byte = 0
        bit_count = 0
        for x in range(width):
            r, g, b, a = pixels[x, y]
            if a < alpha_threshold:
                val = transparent_idx & 0x03
            else:
                val = rgb_to_2bit_index((r, g, b)) & 0x03

            if lsb_first:
                row_byte |= val << (bit_count * 2)
            else:
                row_byte |= val << (6 - bit_count * 2)

            bit_count += 1
            if bit_count == 4:
                packed.append(row_byte)
                row_byte = 0
                bit_count = 0

        if bit_count > 0:
            packed.append(row_byte)

    return bytes(packed), width, height


def write_header(
    out_path: Path,
    alien_ident: str,
    source_name: str,
    data: bytes,
    width: int,
    height: int,
    lsb_first: bool,
) -> None:
    guard = f"{alien_ident.upper()}_2BIT_H"
    arr_name = f"{alien_ident}_2bit_map"
    order_str = "LSB-first (pixel 0 at bits 1..0)" if lsb_first else "MSB-first (pixel 0 at bits 7..6)"

    lines = [
        f"#ifndef {guard}",
        f"#define {guard}",
        "",
        "#include <stdint.h>",
        "#include <stddef.h>",
        "",
        f"/* Source: {source_name} | Size: {width}x{height} | 2-bit (4 colors) | {order_str} */",
        f"#define {alien_ident.upper()}_WIDTH  {width}U",
        f"#define {alien_ident.upper()}_HEIGHT {height}U",
        f"#define {alien_ident.upper()}_SIZE   {len(data)}U",
        "",
        f"static const uint8_t {arr_name}[{len(data)}] = {{",
    ]

    for i in range(0, len(data), 12):
        chunk = data[i : i + 12]
        hex_bytes = ", ".join(f"0x{b:02X}" for b in chunk)
        comma = "," if i + 12 < len(data) else ""
        lines.append(f"    {hex_bytes}{comma}")

    lines.extend([
        "};",
        "",
        f"#endif /* {guard} */",
        "",
    ])

    out_path.write_text("\n".join(lines), encoding="utf-8")


def main() -> None:
    parser = argparse.ArgumentParser(
        description="Batch-convert alien silhouette images into separate 2-bit C headers."
    )
    parser.add_argument(
        "-i", "--input-dir",
        type=Path,
        default=Path("silhoutes"),
        help="Input directory containing alien images (default: silhoutes)",
    )
    parser.add_argument(
        "-o", "--output-dir",
        type=Path,
        default=Path("aliens_2bit_h"),
        help="Output directory for generated .h files (default: aliens_2bit_h)",
    )
    parser.add_argument(
        "--resize",
        type=str,
        default=None,
        help="Optional WxH resize before conversion, e.g. 64x64",
    )
    parser.add_argument(
        "--lsb",
        action="store_true",
        help="Pack LSB-first instead of MSB-first",
    )
    parser.add_argument(
        "--keep-raw-name",
        action="store_true",
        help="Keep full filename stem instead of stripping trailing 3-char hash suffixes",
    )
    args = parser.parse_args()

    if not args.input_dir.is_dir():
        raise SystemExit(f"Input folder not found: {args.input_dir}")

    args.output_dir.mkdir(parents=True, exist_ok=True)

    resample_filter = getattr(getattr(Image, "Resampling", Image), "NEAREST")
    target_size: tuple[int, int] | None = None
    if args.resize:
        w_str, h_str = args.resize.lower().split("x", 1)
        target_size = (int(w_str), int(h_str))

    files = sorted(
        p for p in args.input_dir.iterdir()
        if p.is_file() and p.suffix.lower() in SUPPORTED_EXTS
    )

    if not files:
        raise SystemExit(f"No supported image files found in {args.input_dir}")

    used_names: dict[str, int] = {}
    for src_path in files:
        base_ident = clean_alien_name(src_path.stem, keep_raw_name=args.keep_raw_name)
        count = used_names.get(base_ident, 0) + 1
        used_names[base_ident] = count
        ident = base_ident if count == 1 else f"{base_ident}_{count}"

        with Image.open(src_path) as raw_img:
            img = raw_img.copy()

        if target_size is not None:
            img = img.resize(target_size, resample_filter)

        packed, width, height = pack_2bit_bitmap(img, lsb_first=args.lsb)
        out_header = args.output_dir / f"{ident}_2bit.h"
        write_header(out_header, ident, src_path.name, packed, width, height, lsb_first=args.lsb)
        print(f"{src_path.name:22s} -> {out_header} ({width}x{height}, {len(packed)} bytes)")


if __name__ == "__main__":
    main()