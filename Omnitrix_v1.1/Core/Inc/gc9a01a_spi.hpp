/*
 * gc9a01a_spi.h
 *
 *  Created on: 04-May-2026
 *      Author: ashish
 */

#ifndef INC_GC9A01A_SPI_HPP_
#define INC_GC9A01A_SPI_HPP_

#include <stdint.h>
#include <stddef.h>
#include <cmath>

extern "C"
{
#include "gpio.h"
#include "spi.h"
}

constexpr uint16_t HEIGHT_RES = 240;
constexpr uint16_t WIDTH_RES  = 240;

struct Bitmap_t
{
	uint16_t Width = 0;
	uint16_t Height = 0;

	const uint16_t *Bitmap = nullptr;
};

struct LineAlgorithm
{
	uint16_t x,y; // Current Cordinate
	uint16_t fx,fy; // final x and y

	int16_t dx  =  0;
	int16_t dy  =  0;
	int16_t sx  = 0;
	int16_t sy  = 0;
	int16_t err = 0;              // dx minus dy
	int16_t e2;

	LineAlgorithm(uint16_t StartX, uint16_t StartY, uint16_t FinalX, uint16_t FinalY)
		: x(StartX) , y(StartY),
		fx(FinalX) , fy(FinalY)
	{
		dx  =  std::abs(fx - x);
		dy  =  std::abs(fy - y);   // positive
		sx  = x < fx ? 1 : -1;
		sy  = y < fy ? 1 : -1;
		err = dx - dy;              // dx minus dy
	}

	void init(uint16_t StartX, uint16_t StartY, uint16_t FinalX, uint16_t FinalY)
	{
		x = StartX; y = StartY;
		fx = FinalX; fy = FinalY;

		dx  =  std::abs(fx - x);
		dy  =  std::abs(fy - y);   // positive
		sx  = x < fx ? 1 : -1;
		sy  = y < fy ? 1 : -1;
		err = dx - dy;              // dx minus dy
	}

	bool run_iter()
	{
		if (x == fx && y == fy) return true;

		e2 = 2 * err;

		if (e2 > -dy)
		{    // positive dy convention
			err -= dy;
			x  += sx;
		}

		if (e2 < dx)
		{
			err += dx;
			y  += sy;
		}

		return false;
	}

};

enum Alignment
{
	TOP = 1 << 0,
	BOTTOM = 1 << 1,
	LEFT = 1 << 2,
	RIGHT = 1 << 3,
	CENTER = 1 << 4,
};

class GC9A01A
{
    SPI_HandleTypeDef &Handle;

    struct gpios
    {
        GPIO_TypeDef *Port;
        uint16_t Pin;
    };

    gpios ChipSelect;
    gpios DataCommand;
    gpios ResetPin;

public:
    void cs_low();
    void cs_high();

    void dc_cmd();
    void dc_data();

    void rst_low()
    {
        HAL_GPIO_WritePin(
			ResetPin.Port,
			ResetPin.Pin,
            GPIO_PIN_RESET
        );
    }

    void rst_high()
    {
    	HAL_GPIO_WritePin(
			ResetPin.Port,
			ResetPin.Pin,
            GPIO_PIN_SET
        );
    }


    static constexpr uint16_t rgb565(uint8_t r, uint8_t g, uint8_t b) {
        return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
    }

    GC9A01A(
        SPI_HandleTypeDef &handlRef,
        GPIO_TypeDef *CsPort, uint16_t CsPin,
        GPIO_TypeDef *DcPort, uint16_t DcPin,
		GPIO_TypeDef *RstPort, uint16_t RstPin
    )
        : Handle(handlRef),
          ChipSelect{CsPort, CsPin},
          DataCommand{DcPort, DcPin},
		  ResetPin{RstPort,RstPin}
    {}

    void init();

    void write_cmd(uint8_t cmd);
    void write_data(const uint8_t *data, size_t size);

    void set_window(
        uint16_t x1,
        uint16_t y1,
        uint16_t x2,
        uint16_t y2
    );

    void write_cmd_data(
        uint8_t cmd,
        const uint8_t* data,
        size_t size
    );

    void begin_write();
    void write_color(
        uint16_t color,
        uint32_t count
    );

    void fill_screen(uint16_t color);

    void draw_pixel(
        uint16_t x,
        uint16_t y,
        uint16_t color
    );

    void draw_fast_hline(
        uint16_t x,
        uint16_t y,
        uint16_t width,
        uint16_t color
    );

    void draw_fast_vline(
		uint16_t x,
		uint16_t y,
		uint16_t width,
		uint16_t color
	);

    void fill_rect(
        uint16_t x,
        uint16_t y,
        uint16_t w,
        uint16_t h,
        uint16_t color
    );

    void fill_circle(int16_t x0, int16_t y0, int16_t r, uint16_t color);
    void benchmark_write_color();
    void draw_line(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t );
    void draw_circle(int16_t x0, int16_t y0, int16_t r, uint16_t color);
    void fill_triangle(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t x3, uint16_t y3, uint16_t Color);
    void draw_triangle(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t x3, uint16_t y3, uint16_t Color);

    void draw_font5x7(uint16_t x, uint16_t y, char ch, uint16_t Color);
    void draw_bitmap(uint16_t x, uint16_t y, const uint16_t *bitmap, uint16_t w, uint16_t h);
    void draw_bitmap(uint16_t x, uint16_t y, const Bitmap_t &);
    void draw_bitmap(uint8_t AlignFlag, const Bitmap_t &);

    void draw_bitmap_1bit(
    	    uint16_t x, uint16_t y,
    	    const uint8_t *bitmap,
    	    uint16_t w, uint16_t h,
    	    uint16_t fg_color,   // alien color
    	    uint16_t bg_color);   // background color

    inline void dma_wait();
    void dma_send(uint8_t *buf, uint32_t bytes);
    void draw_omnitrix_growth_frame(const uint8_t row_bounds[240], uint16_t triangle_color, uint16_t bg_color);
    void draw_omnitrix_cross_frame(const uint8_t row_bounds[240], uint16_t triangle_color, uint16_t bg_color);
};

#endif
