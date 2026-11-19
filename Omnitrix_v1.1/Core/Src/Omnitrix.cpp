/*
 * Omnitrix.cpp
 *
 *  Created on: 17-Jun-2026
 *      Author: ashish
 */

#include <stdio.h>
#include "Omnitrix.hpp"
#include "gc9a01a_spi.hpp"
#include "gpio.h"
#include "omnitrix_anim.h"
#include "alien_4_arms_2bit.h"
#include "ben_mummyf6v_2bit.h"
#include "ben_wolf_2bit.h"
#include "benvictor_2bit.h"
#include "cannon_bolt_2bit.h"
#include "diamond_headf7h_2bit.h"
#include "dittof7o_2bit.h"
#include "eye_guyf7w_2bit.h"
#include "ghostfreak_2bit.h"
#include "grey_matter_2bit.h"
#include "heatblast_2bit.h"
#include "ripjaw_2bit.h"
#include "stink_flyf9g_2bit.h"
#include "upchuckf9n_2bit.h"
#include "upgradef9r_2bit.h"
#include "way_big_2bit.h"
#include "wild_mut_2bit.h"
#include "wild_vine_2bit.h"
#include "xlzr8_2bit.h"
#include "omnitrix_2bit.h"
//#define FREERTOS

#ifdef FREERTOS
#include "FreeRTOS.h"
#include "task.h"

#define DisplayDelay(x)	vTaskDelay(x)

#else

#define DisplayDelay(x)	HalDelay(x)

#endif

extern GC9A01A tft;
//extern const uint8_t omnitrix_data[];

const uint8_t* const AlienSchDisplay[] =
{
	heatblast_2bit_map,
	alien_4_arms_2bit_map,
	xlzr8_2bit_map,
	diamond_headf7h_2bit_map,
	grey_matter_2bit_map,
	way_big_2bit_map,
//	ben_mummyf6v_2bit_map,
//	ben_wolf_2bit_map,
//	benvictor_2bit_map,
	cannon_bolt_2bit_map,
	dittof7o_2bit_map,
	eye_guyf7w_2bit_map,
	ghostfreak_2bit_map,
	ripjaw_2bit_map,
//	stink_flyf9g_2bit_map,
	upchuckf9n_2bit_map,
	upgradef9r_2bit_map,
	wild_mut_2bit_map,
//	wild_vine_2bit_map,
};

#define ALIEN_SCH_DISPLAY_COUNT (sizeof(AlienSchDisplay) / sizeof(AlienSchDisplay[0]))

/* Map 2-bit values 0..3 to your display's native pixel format (e.g., RGB565) */
//static const uint16_t palette_rgb565[4] = {
//    0x0000U, /* 0: Black      */
//    0x4208U, /* 1: Dark Grey  */
//    0xA514U, /* 2: Light Grey */
//    0x07E0U, /* 3: Green      */
//};

static inline uint16_t swap_u16(uint16_t c)
{
    return (uint16_t)((c >> 8) | (c << 8));
}

// Pre-swap your 4 palette entries once before calling draw_bitmap_2bit:
uint16_t palette_rgb565[4] = {
    swap_u16(0x0000U), // 0: Black
    swap_u16(0x4208U), // 1: Dark Grey
    swap_u16(0xA514U), // 2: Light Grey
    swap_u16(0x07E0U), // 3: Green
};

/* Classic Omnitrix palette for indices 0..3:
 *   0: Black
 *   1: Grey
 *   2: Light Grey
 *   3: Green
 */
const uint16_t omnitrix_classic_palette[4] = {
	swap_u16(0x0000U), /* 0: Black      */
	swap_u16(0x7BEFU), /* 1: Grey       */
	swap_u16(0xC618U), /* 2: Light Grey */
	swap_u16(0x07E0U), /* 3: Green      */
};

constexpr uint16_t SoftwareDebounceDelay = 100;
constexpr uint8_t MaxLoopCnt = 5;
UserInput Input = UserInput::NONE;

#ifdef FREERTOS

extern TaskHandle_t xGuiTaskHandle;

enum GpioSetBit : uint8_t
{
	ACTIVITY_SET,
	LEFT_BUT_SET,
	RIGHT_BUT_SET
};

inline void FlushGpioNotifications()
{
    uint32_t ulDummy;
    // Clear all pending bits and reset the notification state to un-signaled instantly
    xTaskNotifyWait(0xFFFFFFFF, 0xFFFFFFFF, &ulDummy, 0);
}

uint8_t GpioInput()
{
	uint8_t Mask = 0;

	BaseType_t xResult;
	uint32_t ulNotificationValue;

	do
	{
		/* Block until a notification arrives.
		           Don't clear bits on entry (0x00), clear all bits on exit (0xFFFFFFFF) */
		xResult = xTaskNotifyWait(0x00, 0xFFFFFFFF, &ulNotificationValue, portMAX_DELAY);
	}while(xResult != pdPASS);

	if(ulNotificationValue & ACTIVITY_BUTTON_Pin)
	{
		Mask |= 1 << (GpioSetBit::ACTIVITY_SET);
	}

	if(ulNotificationValue & LEFT_ROT_BUTTON_Pin)
	{
		Mask |= 1 << (GpioSetBit::LEFT_BUT_SET);
	}

	if(ulNotificationValue & RIGHT_ROT_BUTTON_Pin)
	{
		Mask |= 1 << (GpioSetBit::RIGHT_BUT_SET);
	}

	return Mask; // in case no gpio is set or a gpio is handled
}

#endif


void handleOmniWatch()
{
	static States CurrentState = States::CHANGE_TO_NORMAL_SCREEN;
	uint8_t PinRead = 0;
	static uint8_t CurrentAlienCntr = 0;

	static const uint16_t WhiteClr = GC9A01A::rgb565(255, 255, 255);
	static const uint16_t RedClr = GC9A01A::rgb565(255, 0, 0);
	static const uint16_t GreenClr = GC9A01A::rgb565(0, 255, 0);

	switch(CurrentState)
	{
	case States::CHANGE_TO_NORMAL_SCREEN :
		// Display the symbol and move to IDLE state
		tft.draw_bitmap_2bit(0, 0,  omnitrix_2bit_map , 240,240,omnitrix_classic_palette);//GreenClr);
		CurrentState = States::IDLE;
		break;

	case States::IDLE:
		PinRead = GpioInput(); //HAL_GPIO_ReadPin(ACTIVITY_BUTTON_GPIO_Port, ACTIVITY_BUTTON_Pin);
		if(PinRead & 1 << (ACTIVITY_SET))
		{
			FlushGpioNotifications();
			CurrentState = States::ACTIVATING;
		}
		break;

	case States::ACTIVATING:
//		tft.draw_bitmap_2bit(0, 0,  omnitrix_2bit_map , 240,240,omnitrix_classic_palette);//GreenClr);
		tft.fill_screen(GreenClr);
		DisplayDelay(500);
		// Loop through each frame of the animation
//		for (uint8_t frame = 0; frame < 3; frame++)
//		{
////			printf("Frame %d\r\n", frame);
//			tft.draw_omnitrix_growth_frame(left_triangle_growth[frame], 0x00, GreenClr);
//			HAL_Delay(100);
//		}
//		for (uint8_t frame = 0; frame < 10; frame++)
//		{
////			printf("Frame %d\r\n", frame);
//			tft.draw_omnitrix_cross_frame(omnitrix_cross_bounds[frame], GreenClr, 0x00);
//			HAL_Delay(200);
//		}

		//Animation is needed here
		CurrentAlienCntr = 0;
		CurrentState = States::DISPLAY_ALIENS;
		break;

	case States::DISPLAY_ALIENS:

		switch(Input)
		{
		case UserInput::LEFT:
			if(CurrentAlienCntr == 0)
			{
				CurrentAlienCntr = sizeof(AlienSchDisplay) / sizeof(AlienSchDisplay[0]) - 1;
			}
			else
			{
				CurrentAlienCntr--;
			}
			break;

		case UserInput::RIGHT:
			CurrentAlienCntr++;
			if(CurrentAlienCntr == sizeof(AlienSchDisplay) / sizeof(AlienSchDisplay[0]))
			{
				CurrentAlienCntr = 0;
			}
			break;

		case UserInput::NONE: // Intentional Blank
		case UserInput::ACTION:  // Intentional Blank
			break;
		}
		tft.draw_bitmap_2bit(0, 0, AlienSchDisplay[CurrentAlienCntr], 240, 240, palette_rgb565);
		Input = UserInput::NONE;
		CurrentState = States::HANLDE_INPUT;
		break;

	case States::HANLDE_INPUT:
		PinRead = GpioInput();// HAL_GPIO_ReadPin(LEFT_ROT_BUTTON_GPIO_Port, LEFT_ROT_BUTTON_Pin);
		if(PinRead & 1 << LEFT_BUT_SET)
		{
			DisplayDelay(SoftwareDebounceDelay);
			FlushGpioNotifications();
			Input = UserInput::LEFT;
			CurrentState = States::DISPLAY_ALIENS;
			break;
		}

//		PinRead = HAL_GPIO_ReadPin(RIGHT_ROT_BUTTON_GPIO_Port, RIGHT_ROT_BUTTON_Pin);
		if(PinRead & 1 << RIGHT_BUT_SET)
		{
			DisplayDelay(SoftwareDebounceDelay);
			FlushGpioNotifications();
			Input = UserInput::RIGHT;
			CurrentState = States::DISPLAY_ALIENS;
			break;
		}

//		PinRead = HAL_GPIO_ReadPin(ACTIVITY_BUTTON_GPIO_Port, ACTIVITY_BUTTON_Pin);
		if(PinRead & 1 << ACTIVITY_SET)
		{
			DisplayDelay(SoftwareDebounceDelay);
			FlushGpioNotifications();
			Input = UserInput::ACTION;
			CurrentState = States::HANDLE_ALIENS;
			break;
		}
		break;

	case States::HANDLE_ALIENS:
		tft.draw_bitmap_2bit(0, 0,  omnitrix_2bit_map , 240,240,omnitrix_classic_palette);//WhiteClr);
		PinRead = GpioInput();
//		CurrentState = States::DISCHARGING;
		if(PinRead & 1 << ACTIVITY_SET)
		{
			DisplayDelay(SoftwareDebounceDelay);
			FlushGpioNotifications();
			CurrentState = States::DISCHARGING;
			break;
		}
		break;

	case States::DISCHARGING:
		for(uint8_t Iter = 0; Iter < MaxLoopCnt; ++Iter)
		{
			tft.draw_bitmap_2bit(0, 0,  omnitrix_2bit_map , 240,240,omnitrix_classic_palette);//WhiteClr);
			DisplayDelay(500);
			tft.draw_bitmap_2bit(0, 0,  omnitrix_2bit_map , 240,240,omnitrix_classic_palette);//RedClr);
			DisplayDelay(500);
		}
		tft.fill_screen(RedClr);
		DisplayDelay(1500);
		tft.draw_bitmap_2bit(0, 0,  omnitrix_2bit_map , 240,240,omnitrix_classic_palette);//RedClr);
		CurrentState = States::RECHARGING;
		break;

	case States::RECHARGING:
		DisplayDelay(10000);
		CurrentState = States::CHANGE_TO_NORMAL_SCREEN;
		break;
	}
}

