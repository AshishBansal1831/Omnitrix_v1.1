/*
 * gui_task.cpp
 *
 *  Created on: 18-Jul-2026
 *      Author: ashish
 */
#include "gc9a01a_spi.hpp"
#include "Omnitrix.hpp"
#include "FreeRTOS.h"
#include "task.h"

extern GC9A01A tft;
extern const uint8_t omnitrix_data[];
extern TaskHandle_t xGuiTaskHandle;
extern void handleOmniWatch();

void gui_task(void *arguments)
{
    xGuiTaskHandle = xTaskGetCurrentTaskHandle();
    xTaskNotifyStateClear(NULL);

    tft.init();
    vTaskDelay(pdMS_TO_TICKS(10));

    // Keep the task alive or let it process background UI updates
    while(1)
    {
    	handleOmniWatch();
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}
