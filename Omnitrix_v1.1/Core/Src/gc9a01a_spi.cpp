/*
 * gc9a01a_spi.c
 *
 *  Created on: 04-May-2026
 *      Author: ashish
 */

#define FREERTOS

#include <iostream>
#include <cstring>
#include <gc9a01a_spi.hpp>

#ifdef FREERTOS

	#include "FreeRTOS.h"
	#include "task.h"
	#include "limits"
#else

#endif

#define DMA_BUF_PIXELS   (240 * 8)          // 8 rows — tunable
#define DMA_BUF_BYTES    (DMA_BUF_PIXELS * 2)


constexpr uint16_t X_OFFSET = 0;
constexpr uint16_t Y_OFFSET = 80;



static uint8_t  dma_buf[2][DMA_BUF_BYTES];
#ifdef FREERTOS

TaskHandle_t xGuiTaskHandle = NULL;

#else

volatile uint8_t dma_busy = false;      // set in launch, cleared in callback

#endif
// A simple 5x7 font representation of 'A'
static const uint8_t font_5x7_A[] = {
    0b00100, //   #
    0b01010, //  # #
    0b01010, //  # #
    0b01110, //  ###
    0b10001, // #   #
    0b10001, // #   #
    0b10001  // #   #
};

void GC9A01A::init()
{
    rst_high(); HAL_Delay(10);
    rst_low();  HAL_Delay(10);
    rst_high(); HAL_Delay(120);

    write_cmd(0xEF);                                    // Begin inter-register unlock sequence
    write_cmd(0xEB);
    uint8_t d = 0x14; write_data(&d, 1);               // Unlock step 1

    write_cmd(0xFE);                                    // Unlock step 2
    write_cmd(0xEF);                                    // Unlock step 3 — gates second register bank

    write_cmd(0xEB);
    d = 0x14; write_data(&d, 1);                       // Confirm unlock

    write_cmd(0x84);
    d = 0x40; write_data(&d, 1);                       // Power control: VCORE level

    write_cmd(0x85);
    d = 0xFF; write_data(&d, 1);                       // Power control: VGHL level

    write_cmd(0x86);
    d = 0xFF; write_data(&d, 1);                       // Power control: VGH clamp

    write_cmd(0x87);
    d = 0xFF; write_data(&d, 1);                       // Power control: VGL clamp

    write_cmd(0x88);
    d = 0x0A; write_data(&d, 1);                       // Power control: source bias

    write_cmd(0x89);
    d = 0x21; write_data(&d, 1);                       // Power control: gate bias

    write_cmd(0x8A);
    d = 0x00; write_data(&d, 1);                       // Power control: reserved

    write_cmd(0x8B);
    d = 0x80; write_data(&d, 1);                       // Power control: pump clock divider

    write_cmd(0x8C);
    d = 0x01; write_data(&d, 1);                       // Power control: pump phase 1

    write_cmd(0x8D);
    d = 0x01; write_data(&d, 1);                       // Power control: pump phase 2

    write_cmd(0x8E);
    d = 0xFF; write_data(&d, 1);                       // Power control: VCOM high level

    write_cmd(0x8F);
    d = 0xFF; write_data(&d, 1);                       // Power control: VCOM low level

    write_cmd(0xB6);
    uint8_t b6[2] = {0x00, 0x20};
    write_data(b6, 2);                                 // Display function: non-display area handling

    write_cmd(0x36);
    d = 0xC8; write_data(&d, 1);                       // MADCTL: MV+MX bits — maps round panel scan to logical XY

    write_cmd(0x3A);
    d = 0x05; write_data(&d, 1);                       // Pixel format: 0x05 = RGB565 (GC9A01 specific, 0x55 is wrong here)

    write_cmd(0x90);
    uint8_t d90[4] = {0x08, 0x08, 0x08, 0x08};
    write_data(d90, 4);                                // Gate driver: output timing per line

    write_cmd(0xBD);
    d = 0x06; write_data(&d, 1);                       // Gate driver: EQ timing

    write_cmd(0xBC);
    d = 0x00; write_data(&d, 1);                       // Gate driver: EQ period

    write_cmd(0xFF);
    uint8_t dff[3] = {0x60, 0x01, 0x04};
    write_data(dff, 3);                                // Page select: unlock extended register bank

    write_cmd(0xC3);
    d = 0x13; write_data(&d, 1);                       // Power: VREG1 voltage (positive gamma reference)

    write_cmd(0xC4);
    d = 0x13; write_data(&d, 1);                       // Power: VREG2 voltage (negative gamma reference)

    write_cmd(0xC9);
    d = 0x22; write_data(&d, 1);                       // Power: VCOM voltage — affects brightness/contrast

    write_cmd(0xBE);
    d = 0x11; write_data(&d, 1);                       // Gate driver: output slew rate

    write_cmd(0xE1);
    uint8_t e1[2] = {0x10, 0x0E};
    write_data(e1, 2);                                 // Gate driver: pre-charge timing

    write_cmd(0xDF);
    uint8_t ddf[3] = {0x21, 0x0C, 0x02};
    write_data(ddf, 3);                                // Source driver: bias current control

    write_cmd(0xF0);
    uint8_t f0[6] = {0x45, 0x09, 0x08, 0x08, 0x26, 0x2A};
    write_data(f0, 6);                                 // Gamma positive polarity curve (panel-specific)

    write_cmd(0xF1);
    uint8_t f1[6] = {0x43, 0x70, 0x72, 0x36, 0x37, 0x6F};
    write_data(f1, 6);                                 // Gamma negative polarity curve (panel-specific)

    write_cmd(0xF2);
    uint8_t f2[6] = {0x45, 0x09, 0x08, 0x08, 0x26, 0x2A};
    write_data(f2, 6);                                 // Gamma positive polarity curve 2 (redundant, same as F0)

    write_cmd(0xF3);
    uint8_t f3[6] = {0x43, 0x70, 0x72, 0x36, 0x37, 0x6F};
    write_data(f3, 6);                                 // Gamma negative polarity curve 2 (redundant, same as F1)

    write_cmd(0xED);
    uint8_t ed[2] = {0x1B, 0x0B};
    write_data(ed, 2);                                 // Source driver: precharge level

    write_cmd(0xAE);
    d = 0x77; write_data(&d, 1);                       // Gate driver: charge sharing period

    write_cmd(0xCD);
    d = 0x63; write_data(&d, 1);                       // Source driver: output impedance

    write_cmd(0x70);
    uint8_t d70[9] = {0x07,0x07,0x04,0x0E,0x0F,0x09,0x07,0x08,0x03};
    write_data(d70, 9);                                // Gate timing: panel-specific scan waveform

    write_cmd(0xE8);
    d = 0x34; write_data(&d, 1);                       // Source driver: power saving mode

    write_cmd(0x62);
    uint8_t d62[12] = {0x18,0x0D,0x71,0xED,0x70,0x70,
                       0x18,0x0F,0x71,0xEF,0x70,0x70};
    write_data(d62, 12);                               // Gate scan: address mapping for round panel top half

    write_cmd(0x63);
    uint8_t d63[12] = {0x18,0x11,0x71,0xF1,0x70,0x70,
                       0x18,0x13,0x71,0xF3,0x70,0x70};
    write_data(d63, 12);                               // Gate scan: address mapping for round panel bottom half

    write_cmd(0x64);
    uint8_t d64[7] = {0x28, 0x29, 0xF1, 0x01, 0xF1, 0x00, 0x07};
    write_data(d64, 7);                                // Gate scan: diagonal region address mapping

    write_cmd(0x66);
    uint8_t d66[10] = {0x3C,0x00,0xCD,0x67,0x45,0x45,0x10,0x00,0x00,0x00};
    write_data(d66, 10);                               // Gate scan: left edge address mapping

    write_cmd(0x67);
    uint8_t d67[10] = {0x00,0x3C,0x00,0x00,0x00,0x01,0x54,0x10,0x32,0x98};
    write_data(d67, 10);                               // Gate scan: right edge address mapping

    write_cmd(0x74);
    uint8_t d74[7] = {0x10, 0x85, 0x80, 0x00, 0x00, 0x4E, 0x00};
    write_data(d74, 7);                                // Source timing: output waveform shape

    write_cmd(0x98);
    uint8_t d98[2] = {0x3E, 0x07};
    write_data(d98, 2);                                // Source driver: VCOM amplitude

    write_cmd(0x35);                                   // Tearing effect line ON — TE pin sync signal
    write_cmd(0x21);                                   // Display inversion ON — required by this panel's glass

    write_cmd(0x11);                                   // Sleep out — starts internal oscillator
    HAL_Delay(120);                                    // Mandatory 120ms wait after sleep out

    write_cmd(0x29);                                   // Display on
    HAL_Delay(20);

    fill_screen(0x0000);                               // Clear to black
    vTaskDelay(10);
}
void GC9A01A::cs_low()
{
    HAL_GPIO_WritePin(
        ChipSelect.Port,
        ChipSelect.Pin,
        GPIO_PIN_RESET
    );
}

void GC9A01A::cs_high()
{
    HAL_GPIO_WritePin(
        ChipSelect.Port,
        ChipSelect.Pin,
        GPIO_PIN_SET
    );
}

void GC9A01A::dc_cmd()
{
    HAL_GPIO_WritePin(
        DataCommand.Port,
        DataCommand.Pin,
        GPIO_PIN_RESET
    );
}

void GC9A01A::dc_data()
{
    HAL_GPIO_WritePin(
        DataCommand.Port,
        DataCommand.Pin,
        GPIO_PIN_SET
    );
}

void GC9A01A::write_cmd(uint8_t cmd)
{
    cs_low();

    dc_cmd();

    HAL_SPI_Transmit(
        &Handle,
        &cmd,
        1,
        HAL_MAX_DELAY
    );

    cs_high();
}

void GC9A01A::write_data(
    const uint8_t *data,
    size_t size
)
{
    cs_low();

    dc_data();

    HAL_SPI_Transmit(
        &Handle,
        (uint8_t*)data,
        size,
        HAL_MAX_DELAY
    );

    cs_high();
}

void GC9A01A::set_window(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2) {
    // Apply internal hardware offsets
    uint8_t data[4];
    cs_low();

    // Column Address Set
    dc_cmd();
    uint8_t caset = 0x2A;
    HAL_SPI_Transmit(&Handle, &caset, 1, HAL_MAX_DELAY);
    dc_data();
    data[0] = x1 >> 8; data[1] = x1 & 0xFF;
    data[2] = x2 >> 8; data[3] = x2 & 0xFF;
    HAL_SPI_Transmit(&Handle, data, 4, HAL_MAX_DELAY);

    // Row Address Set
    dc_cmd();
    uint8_t raset = 0x2B;
    HAL_SPI_Transmit(&Handle, &raset, 1, HAL_MAX_DELAY);
    dc_data();
    data[0] = y1 >> 8; data[1] = y1 & 0xFF;
    data[2] = y2 >> 8; data[3] = y2 & 0xFF;
    HAL_SPI_Transmit(&Handle, data, 4, HAL_MAX_DELAY);

}

void GC9A01A::begin_write()
{
    uint8_t cmd = 0x2C;

    dc_cmd();

    HAL_SPI_Transmit(
        &Handle,
        &cmd,
        1,
        HAL_MAX_DELAY
    );

    dc_data();
}
void GC9A01A::write_color(uint16_t color, uint32_t count)
{
    // Keep this blocking — it's only called for single pixels
    // and from draw_fast_hline/vline where count is small
    static uint8_t buf[WIDTH_RES * 2];
    const uint16_t swapped = (color >> 8) | (color << 8);

    uint32_t pixels = (count > WIDTH_RES) ? WIDTH_RES : count;
    uint16_t *ptr = (uint16_t *)buf;
    for (uint32_t i = 0; i < pixels; i++) *ptr++ = swapped;

    HAL_SPI_Transmit(&Handle, buf, pixels * 2, HAL_MAX_DELAY);
}

void GC9A01A::fill_screen(uint16_t color)
{
//    cs_low();
//
//	set_window(
//		0,
//		0,
//		WIDTH_RES,
//		HEIGHT_RES
//	);
//
//	begin_write();

    // Pixel burst -- CAN'T USE FILL RECT BECAUSE IT IS SLOW FOR FILL SCREEN
	fill_rect(0, 0, WIDTH_RES, HEIGHT_RES, color);

//    cs_high();  // CS goes high ONCE, after everything
}


void GC9A01A::draw_pixel(
    uint16_t x,
    uint16_t y,
    uint16_t color
)
{
    cs_low();

    set_window(x, y, x, y);

    begin_write();

    write_color(color, 1);

    cs_high();
}

void GC9A01A::draw_fast_hline(
    uint16_t x,
    uint16_t y,
    uint16_t width,
    uint16_t color
)
{
    cs_low();

    // Bounds check — silently clip
    set_window(
        x,
        y,
        x + width - 1,
        y
    );

    begin_write();

    write_color(color, width);

//    cs_high();
}

void GC9A01A::draw_fast_vline(
    uint16_t x,
    uint16_t y,
    uint16_t height,
    uint16_t color
)
{
    cs_low();

    set_window(
        x,
        y,
        x,
        y + height - 1
    );

    begin_write();

    write_color(color, height);

    cs_high();
}

void GC9A01A::fill_rect(uint16_t x, uint16_t y,
                         uint16_t w, uint16_t h,
                         uint16_t color)
{
    const uint16_t swapped = (color >> 8) | (color << 8);

    // Pre-fill both buffers — CPU does this once
    uint32_t fill_pixels = w * 8;
    if (fill_pixels > DMA_BUF_PIXELS) fill_pixels = DMA_BUF_PIXELS;

    for (int b = 0; b < 2; b++) {
        uint16_t *ptr = (uint16_t *)dma_buf[b];
        for (uint32_t i = 0; i < fill_pixels; i++) *ptr++ = swapped;
    }

    cs_low();
    set_window(x, y, x + w - 1, y + h - 1);
    begin_write();

    uint32_t total  = (uint32_t)w * h;
    uint32_t sent   = 0;
    uint8_t  buf_idx = 0;

    while (sent < total) {
        uint32_t chunk = total - sent;
        if (chunk > fill_pixels) chunk = fill_pixels;

        // Wait for previous DMA on this buffer slot, then fire
        if (sent > 0)
        {
			dma_wait();
		}
#ifndef FREERTOS
        dma_busy = true;
#endif
        HAL_SPI_Transmit_DMA(&Handle, dma_buf[buf_idx], chunk * 2);

        buf_idx ^= 1;   // flip buffer index
        sent    += chunk;

        // CPU is free here to prepare the next buffer if content varies
        // For solid fill: nothing to do, both buffers already filled
    }

    dma_wait();     // ensure last transfer completes before CS high
    cs_high();
}

void GC9A01A::draw_line(uint16_t x1, uint16_t y1,
                         uint16_t x2, uint16_t y2, uint16_t color)
{
    if (x1 == x2) {
        draw_fast_vline(x1,
            y1 < y2 ? y1 : y2,
            (y1 < y2 ? y2-y1 : y1-y2) + 1, color);
        return;
    }
    if (y1 == y2) {
        draw_fast_hline(
            x1 < x2 ? x1 : x2, y1,
            (x1 < x2 ? x2-x1 : x1-x2) + 1, color);
        return;
    }

    LineAlgorithm drawLine(x1, y1, x2, y2);

    while(true) {
        draw_pixel(drawLine.x, drawLine.y, color);
        if(drawLine.run_iter() == true) break;
    }
}


void GC9A01A::draw_circle(int16_t x0, int16_t y0, int16_t r, uint16_t color) {
    int16_t x = 0;
    int16_t y = r;
    int16_t d = 3 - 2 * r; // Initial decision parameter

    auto plot_8_points = [&](int16_t x, int16_t y) {
        draw_pixel(x0 + x, y0 + y, color);
        draw_pixel(x0 - x, y0 + y, color);
        draw_pixel(x0 + x, y0 - y, color);
        draw_pixel(x0 - x, y0 - y, color);
        if (x != y) {  // skip duplicates at 45 degrees
            draw_pixel(x0 + y, y0 + x, color);
            draw_pixel(x0 - y, y0 + x, color);
            draw_pixel(x0 + y, y0 - x, color);
            draw_pixel(x0 - y, y0 - x, color);
        }
    };

    plot_8_points(x, y);

    while (y >= x)
    {
        x++;

        // Check the decision parameter
        if (d > 0)
        {
            y--;
            d = d + 4 * (x - y) + 10;
        }
        else
        {
            d = d + 4 * x + 6;
        }

        plot_8_points(x, y);
    }
}

void GC9A01A::fill_circle(int16_t x0, int16_t y0, int16_t r, uint16_t color)
{
    int16_t x = 0;
    int16_t y = r;
    int16_t d = 3 - 2 * r;

    auto draw_circle_lines = [&](int16_t x, int16_t y) {
        // Compute clipped start and width for each hline
        auto safe_hline = [&](int16_t sx, int16_t sy, int16_t len) {
            // Clip left
            if (sx < 0) {
                len += sx;  // reduce length by overhang
                sx = 0;
            }
            // Clip right
            if (sx + len > (int16_t)WIDTH_RES)
                len = WIDTH_RES - sx;
            // Clip top/bottom
            if (sy < 0 || sy >= (int16_t)HEIGHT_RES)
                return;
            // Skip degenerate
            if (len <= 0)
                return;

            draw_fast_hline((uint16_t)sx, (uint16_t)sy,
                            (uint16_t)len, color);
        };

        safe_hline(x0 - x, y0 + y, 2*x + 1);
        safe_hline(x0 - x, y0 - y, 2*x + 1);
        safe_hline(x0 - y, y0 + x, 2*y + 1);
        if (x != 0)
            safe_hline(x0 - y, y0 - x, 2*y + 1);
    };

    while (y >= x) {
        draw_circle_lines(x, y);
        x++;

        if (d > 0) {
            y--;
            d = d + 4 * (x - y) + 10;
        } else {
            d = d + 4 * x + 6;
        }
    }
}

void GC9A01A::fill_triangle(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t x3, uint16_t y3, uint16_t Color) {
    // 1. Sort vertices by Y (A is top, C is bottom)
    if (y2 < y1) { std::swap(y1, y2); std::swap(x1, x2); }
    if (y3 < y1) { std::swap(y1, y3); std::swap(x1, x3); }
    if (y3 < y2) { std::swap(y2, y3); std::swap(x2, x3); }

    if (y1 == y3) return; // Zero height triangle

    // 2. Calculate slopes (dx/dy)
    // We use float or fixed-point to avoid the "149-149" overlap issue
    float m1 = (float)(x2 - x1) / (y2 - y1); // Side A-B
    float m2 = (float)(x3 - x1) / (y3 - y1); // Side A-C
    float m3 = (float)(x3 - x2) / (y3 - y2); // Side B-C

    // 3. Upper part (A to B)
    for (int y = y1; y <= y2; y++) {
        int sx = x1 + (int)((y - y1) * m1);
        int ex = x1 + (int)((y - y1) * m2);
        if (sx > ex) std::swap(sx, ex);
        draw_fast_hline(sx, y, ex - sx + 1, Color);
    }

    // 4. Lower part (B to C)
    // We skip this if y2 == y3 (flat bottom)
    if (y2 != y3) {
        for (int y = y2 + 1; y <= y3; y++) {
            int sx = x2 + (int)((y - y2) * m3);
            int ex = x1 + (int)((y - y1) * m2); // Side A-C still continues here
            if (sx > ex) std::swap(sx, ex);
            draw_fast_hline(sx, y, ex - sx + 1, Color);
        }
    }
}

// 1. Core Function (Signature updated to w, then h)
void GC9A01A::draw_bitmap(uint16_t x, uint16_t y, const uint16_t *bitmap, uint16_t w, uint16_t h)
{
	cs_low();
	set_window(x, y, x + w - 1, y + h - 1);      // FIXED: X pairs with W, Y pairs with H
	begin_write();
	HAL_SPI_Transmit(&Handle, (uint8_t *)bitmap, w * h * 2, HAL_MAX_DELAY);
	cs_high();
}

// 2. Struct Wrapper
void GC9A01A::draw_bitmap(uint16_t x, uint16_t y, const Bitmap_t &bitmap)
{
	// FIXED: Passing Width then Height to match the new signature above
	draw_bitmap(x, y, bitmap.Bitmap, bitmap.Width, bitmap.Height);
}

// 3. Alignment Wrapper
void GC9A01A::draw_bitmap(uint8_t AlignFlag, const Bitmap_t &bitmap)
{

	// Default to Center for both axes
	uint16_t x_pos = (WIDTH_RES - bitmap.Width) / 2;
	uint16_t y_pos = (HEIGHT_RES - bitmap.Height) / 2;

	// Overwrite X if Left or Right is specified
	if(AlignFlag & Alignment::LEFT)       x_pos = 0;
	else if(AlignFlag & Alignment::RIGHT) x_pos = WIDTH_RES - bitmap.Width;

	// Overwrite Y if Top or Bottom is specified
	if(AlignFlag & Alignment::TOP)        y_pos = 0;
	else if(AlignFlag & Alignment::BOTTOM)y_pos = HEIGHT_RES - bitmap.Height;


	draw_bitmap(x_pos, y_pos, bitmap);
}


void GC9A01A::draw_bitmap_1bit(
    uint16_t x, uint16_t y,
    const uint8_t *bitmap,
    uint16_t w, uint16_t h,
    uint16_t fg_color,
    uint16_t bg_color)
{
    const uint16_t fg_sw = (fg_color >> 8) | (fg_color << 8);
    const uint16_t bg_sw = (bg_color >> 8) | (bg_color << 8);

    cs_low();
    set_window(x, y, x + w - 1, y + h - 1);
    begin_write();

    uint8_t buf_idx = 0;

    for (uint16_t row = 0; row < h; row++) {
        // SAFETY: Wait for the specific buffer slot to become available
        // before the CPU modifies it.
        if (row > 0) {
            dma_wait();
        }

        uint16_t *ptr = (uint16_t *)dma_buf[buf_idx];

        for (uint16_t col = 0; col < w; col++) {
            uint32_t bit_pos = row * w + col;
            bool is_fg = (bitmap[bit_pos / 8] >> (7 - (bit_pos % 8))) & 1;
            *ptr++ = is_fg ? fg_sw : bg_sw;
        }

        // Fire the asynchronous transfer for this row
#ifndef FREERTOS
        dma_busy = true;
#endif
        HAL_SPI_Transmit_DMA(&Handle, dma_buf[buf_idx], w * 2);

        buf_idx ^= 1;   // Flip buffer slot for the next row loop
    }

    // Ensure the very last row transmission finishes before pulling CS high
    dma_wait();
    cs_high();
}


void GC9A01A::draw_triangle(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t x3, uint16_t y3, uint16_t Color)
{
	draw_line(x1, y1, x2, y2, Color);
	draw_line(x1, y1, x3, y3, Color);
	draw_line(x2, y2, x3, y3, Color);
}


inline void GC9A01A::dma_wait()
{
#ifdef FREERTOS

	BaseType_t xResult;
	uint32_t ulNotificationValue;

	do
	{
		xResult = xTaskNotifyWait(
			0x00,               // Do not clear bits on entry
			INT32_MAX,          // Clear all bits on exit
			&ulNotificationValue,
			portMAX_DELAY  // 100ms timeout guard (just in case hardware locks up)
		);
	}while(xResult != pdPASS);
#else
    while (dma_busy);   // spin until previous DMA done
#endif
}

void GC9A01A::dma_send(uint8_t *buf, uint32_t bytes)
{
    dma_wait();
#ifdef FREERTOS

#else
    dma_busy = true;
#endif
    HAL_SPI_Transmit_DMA(&Handle, buf, bytes);
    // returns immediately — callback clears dma_busy
}


extern "C" void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi)
{
#ifdef FREERTOS
	if (hspi->Instance == SPI1) // Match your display SPI instance
	{
		BaseType_t xHigherPriorityTaskWoken = pdFALSE;

		if (xGuiTaskHandle != NULL)
		{
			// Use xTaskNotifyFromISR to match xTaskNotifyWait
			xTaskNotifyFromISR(
					xGuiTaskHandle,
					0x01,               // Set Bit 0
					eSetBits,
					&xHigherPriorityTaskWoken
			);
		}

		// Context switch inside the ISR if the GUI task has a higher priority than the interrupted task
		portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
	}
#else
    // Called by DMA IRQ when transfer complete
    dma_busy = 0;
#endif
}

