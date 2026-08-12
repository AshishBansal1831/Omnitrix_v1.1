/*
 * settings_task.c
 *
 *  Created on: Jun 1, 2025
 *      Author: Ashish Bansal
 */

#if 0
#include <usart.h>
#include <string.h>
#include "FreeRTOS.h"
#include "queue.h"
#include "settings_task.hpp"

volatile xQueueHandle CliToIpcQueue;

void setting_task(void*)
{
    Settings queue_settings;
    uint32_t NewBaudRate = 0;

    while (1)
    {
        if (xQueueReceive(CliToIpcQueue, (uint8_t*) &queue_settings, portMAX_DELAY) != pdPASS)
        {
            continue;
        }

        switch (queue_settings.config_id)
        {
            case Config_t::UART_CONFIG :

                memcpy(&NewBaudRate, queue_settings.Buffer, sizeof(NewBaudRate));

                huart1.Init.BaudRate = NewBaudRate;
                HAL_UART_Init(&huart1);
                break;

            case Config_t::WIFI_SETTINGS :

                break;

            case Config_t::MQTT_SETTINGS :
            	break;
            default:
                break;
        }
    }
}
#endif
