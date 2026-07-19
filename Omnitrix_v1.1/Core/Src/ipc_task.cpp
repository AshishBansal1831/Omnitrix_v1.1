/*
 * ipc_task.c
 *
 *  Created on: 09-Jul-2026
 *      Author: ashish
 */
#define FREERTOS

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include "ipc_task.h"


TaskHandle_t IPC_Taskhandle;
QueueHandle_t SendQueue;

uint8_t dma_rx_buffer1[MAX_BUFFER_SZIE];
uint8_t dma_rx_buffer2[MAX_BUFFER_SZIE];

uint8_t *dma_rx_active_buf = dma_rx_buffer1;

void ipc_task(void *)
{
	IPC_Taskhandle = xTaskGetCurrentTaskHandle();
	SendQueue = xQueueCreate(MAX_BUFFER_SZIE, sizeof(char));

	// use this buffer for internal usage for decoding recevied data
	uint8_t *local_recv_buffer = new uint8_t[MAX_BUFFER_SZIE];
	if(local_recv_buffer == nullptr)
	{
		while(1)
		{
			vTaskDelay(portMAX_DELAY);
		}
	}

	// TODO (optional) Add an init here so that when the device wakes up, it can retrieve saved info and store here if needed

	while(1)
	{
		vTaskDelay(1000);
	}
}

