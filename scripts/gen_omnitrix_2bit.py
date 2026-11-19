#!/usr/bin/env python3
"""Generate a crisp 2-bit (4-level) Omnitrix bitmap C header with green base."""

import argparse
import math
from pathlib import Path
from PIL import Image, ImageDraw

# 2-bit index mapping (0..3):
#   0 (0b00): Black       -> Straight boundary lines on hourglass
#   1 (0b01): Green       -> Base background & hourglass emblem
#   2 (0b10): Light Grey  -> Optional trim ring
#   3 (0b11): Grey        -> Classic Omnitrix dial / watch face
IDX_BLACK = 0
IDX_GREY = 3
IDX_LIGHT_GREY = 2
IDX_GREEN = 1


def print_console_shape(indices: list[int], width: int, height: int, max_cols: int = 80, max_rows: int = 40) -> None:
    """Print an ASCII preview of the rendered 2-bit symbol to the console."""
    chars = {
        IDX_BLACK: "#",
        IDX_GREY: "+",
        IDX_LIGHT_GREY: ".",
        IDX_GREEN: "@",
    }
    step_x = max(1, math.ceil(width / max_cols))
    step_y = max(1, math.ceil(height / max_rows))

    print("\n--- Console Shape Preview (#=Black, +=Grey, .=LightGrey, @=Green) ---")
    for y in range(0, height, step_y):
        row_offset = y * width
        line = "".join(chars.get(indices[row_offset + x], "?") for x in range(0, width, step_x))
        print(line)
    print("----------------------------------------------------------------------\n")


def render_omnitrix_indices(width: int, height: int, boundary_px: int = 7) -> tuple[list[int], dict[str, tuple[float, float]]]:
    """Render a 2-bit (0..3) Omnitrix symbol and return (indices, line_coords_in_output_px)."""
    scale = 4
    w_hi = width * scale
    h_hi = height * scale
    cx = w_hi / 2.0
    cy = h_hi / 2.0
    r_max = min(cx, cy)

    target_px = max(1, boundary_px)
    border_w = max(4, int(round(target_px * scale)))

    # 1. Start with full green base (IDX_GREEN)
    img_hi = Image.new("L", (w_hi, h_hi), IDX_GREEN)
    draw = ImageDraw.Draw(img_hi)

    # 2. Grey dial surround
    r_outer_grey = r_max * 0.93
    r_core_grey = r_max * 0.85

    draw.ellipse(
        [cx - r_outer_grey, cy - r_outer_grey, cx + r_outer_grey, cy + r_outer_grey],
        fill=IDX_GREY,
    )
    draw.ellipse(
        [cx - r_core_grey, cy - r_core_grey, cx + r_core_grey, cy + r_core_grey],
        fill=IDX_GREY,
    )

    # 3. Build curved hourglass halves using circular arcs on the outer sides
    r_arc = r_max * 0.95
    arc_span = math.radians(52)
    steps = 500

    waist_x = r_max * 0.1
    waist_y = r_max * 0.0

    left_poly = []
    for i in range(steps + 1):
        ang = (math.pi - arc_span) + (2.0 * arc_span) * (i / steps)
        px = cx + r_arc * math.cos(ang)
        py = cy + r_arc * math.sin(ang)
        left_poly.append((px, py))
    left_poly.append((cx - waist_x, cy + waist_y))
    left_poly.append((cx - waist_x, cy - waist_y))

    right_poly = []
    for i in range(steps + 1):
        ang = -arc_span + (2.0 * arc_span) * (i / steps)
        px = cx + r_arc * math.cos(ang)
        py = cy + r_arc * math.sin(ang)
        right_poly.append((px, py))
    right_poly.append((cx + waist_x, cy + waist_y))
    right_poly.append((cx + waist_x, cy - waist_y))

    # Draw green hourglass halves first
    draw.polygon(left_poly, fill=IDX_GREEN)
    draw.polygon(right_poly, fill=IDX_GREEN)

    # Draw black boundary ONLY along the straight edges (skipping 0..steps-1 which is the outer arc)
    for poly in (left_poly, right_poly):
        for i in range(steps, len(poly)):
            draw.line([poly[i], poly[(i + 1) % len(poly)]], fill=IDX_BLACK, width=border_w)

    # Convert the 4 key straight-line boundary endpoints back into output pixel space
    coords_px = {
        "top_left": (left_poly[0][0] / scale, left_poly[0][1] / scale),
        "bottom_left": (left_poly[steps][0] / scale, left_poly[steps][1] / scale),
        "top_right": (right_poly[0][0] / scale, right_poly[0][1] / scale),
        "bottom_right": (right_poly[steps][0] / scale, right_poly[steps][1] / scale),
        "waist_left": ((cx - waist_x) / scale, cy / scale),
        "waist_right": ((cx + waist_x) / scale, cy / scale),
        "center": (cx / scale, cy / scale),
    }

    resample_nearest = getattr(getattr(Image, "Resampling", Image), "NEAREST")
    img_out = img_hi.resize((width, height), resample_nearest)
    return list(img_out.getdata()), coords_px


def pack_indices_2bit(indices: list[int], width: int, height: int, lsb_first: bool = False) -> bytes:
    packed = bytearray()
    for y in range(height):
        row_offset = y * width
        row_byte = 0
        bit_count = 0
        for x in range(width):
            val = indices[row_offset + x] & 0x03
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

    return bytes(packed)


def write_header(
    out_path: Path,
    symbol_name: str,
    data: bytes,
    width: int,
    height: int,
    lsb_first: bool,
    coords_px: dict[str, tuple[float, float]],
    boundary_px: int,
) -> None:
    ident = symbol_name.strip().lower()
    guard = f"{ident.upper()}_2BIT_H"
    arr_name = f"{ident}_2bit_map"
    coord_type = f"{ident.capitalize()}Point_t"
    coord_arr = f"{ident}_boundary_coords"
    order_str = "LSB-first (pixel 0 at bits 1..0)" if lsb_first else "MSB-first (pixel 0 at bits 7..6)"

    tl = coords_px["top_left"]
    bl = coords_px["bottom_left"]
    tr = coords_px["top_right"]
    br = coords_px["bottom_right"]
    wl = coords_px["waist_left"]
    wr = coords_px["waist_right"]
    ctr = coords_px["center"]

    lines = [
        f"#ifndef {guard}",
        f"#define {guard}",
        "",
        "#include <stdint.h>",
        "#include <stddef.h>",
        "",
        f"/* Symbol: {ident} | Size: {width}x{height} | 2-bit (4 levels: 0=Black, 1=Green, 2=LightGrey, 3=Grey) | {order_str} */",
        f"#define {ident.upper()}_WIDTH        {width}U",
        f"#define {ident.upper()}_HEIGHT       {height}U",
        f"#define {ident.upper()}_SIZE         {len(data)}U",
        f"#define {ident.upper()}_BOUNDARY_PX  {boundary_px}U",
        "",
        "/* Straight boundary line coordinates (in output pixel space) for procedural/keyframe math */",
        f"typedef struct {{",
        f"    float x;",
        f"    float y;",
        f"}} {coord_type};",
        "",
        f"enum {{",
        f"    {ident.upper()}_COORD_TOP_LEFT = 0,",
        f"    {ident.upper()}_COORD_BOTTOM_LEFT,",
        f"    {ident.upper()}_COORD_TOP_RIGHT,",
        f"    {ident.upper()}_COORD_BOTTOM_RIGHT,",
        f"    {ident.upper()}_COORD_COUNT",
        f"}};",
        "",
        f"static const {coord_type} {coord_arr}[{ident.upper()}_COORD_COUNT] = {{",
        f"    {{ {tl[0]:.2f}f, {tl[1]:.2f}f }}, /* TOP_LEFT     */",
        f"    {{ {bl[0]:.2f}f, {bl[1]:.2f}f }}, /* BOTTOM_LEFT  */",
        f"    {{ {tr[0]:.2f}f, {tr[1]:.2f}f }}, /* TOP_RIGHT    */",
        f"    {{ {br[0]:.2f}f, {br[1]:.2f}f }}, /* BOTTOM_RIGHT */",
        f"}};",
        "",
        f"#define {ident.upper()}_CENTER_X      {ctr[0]:.2f}f",
        f"#define {ident.upper()}_CENTER_Y      {ctr[1]:.2f}f",
        f"#define {ident.upper()}_WAIST_LEFT_X  {wl[0]:.2f}f",
        f"#define {ident.upper()}_WAIST_RIGHT_X {wr[0]:.2f}f",
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
        description="Generate a 2-bit (4-level) Omnitrix bitmap C header from a green base."
    )
    parser.add_argument(
        "-o", "--output",
        type=Path,
        default=Path("omnitrix_2bit.h"),
        help="Output .h file path (default: omnitrix_2bit.h)",
    )
    parser.add_argument(
        "--name",
        type=str,
        default="omnitrix",
        help="Symbol identifier prefix (default: omnitrix)",
    )
    parser.add_argument(
        "--size",
        type=str,
        default="240x240",
        help="Output WxH dimensions (default: 240x240)",
    )
    parser.add_argument(
        "--boundary-px",
        type=int,
        default=7,
        help="Straight boundary thickness in output pixels (default: 7)",
    )
    parser.add_argument(
        "--lsb",
        action="store_true",
        help="Pack LSB-first instead of MSB-first",
    )
    args = parser.parse_args()

    w_str, h_str = args.size.lower().split("x", 1)
    width, height = int(w_str), int(h_str)

    indices, coords_px = render_omnitrix_indices(width, height, boundary_px=args.boundary_px)
    print_console_shape(indices, width, height)
    packed = pack_indices_2bit(indices, width, height, lsb_first=args.lsb)
    write_header(
        args.output,
        args.name,
        packed,
        width,
        height,
        lsb_first=args.lsb,
        coords_px=coords_px,
        boundary_px=args.boundary_px,
    )
    print(f"Wrote {args.output} ({width}x{height}, {len(packed)} bytes)")


if __name__ == "__main__":
    main()