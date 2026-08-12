/*
 * ipc_task.c
 *
 *  Created on: 09-Jul-2026
 *      Author: ashish
 */
#define FREERTOS

#include "spi.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include "ipc_task.h"
#include "cmd_handler.h"


TaskHandle_t IPC_Taskhandle;
volatile QueueHandle_t CliToIpcQueue;

uint8_t dma_rx_buffer1[MAX_BUFFER_SZIE];
uint8_t dma_rx_buffer2[MAX_BUFFER_SZIE];

uint8_t *dma_rx_active_buf = dma_rx_buffer1;

extern uint8_t cmd_out_buffer[CMD_BUFFER_SIZE];
extern size_t cmd_index;

enum class PacketID : uint16_t
{
	AWAKE,
	REQUEST_DATA,
	TIME_DATE_DAY, // not implemented in gui, it will get the data and store it internally.
	WEATHER_STATUS,

	WIFI_STATUS,
	WIFI_ADD_NEW,
	WIFI_CONNECT,
	WIFI_REMOVE,
	WIFI_DISCONNECT,

	MQTT_STATUS,
	MQTT_NEW,
	MQTT_CONNECT,
	MQTT_REMOVE
};


void ipc_task(void *)
{
	IPC_Taskhandle = xTaskGetCurrentTaskHandle();
	struct CliToIpcData
	{
		IpcID Id;
		uint8_t Size;
		uint8_t *payload;
	};

	CliToIpcData Data;

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
		if( xQueueReceive(CliToIpcQueue, local_recv_buffer, 10)==pdPASS)
		{
			Data.Id = (IpcID)local_recv_buffer[0];
			Data.Size = local_recv_buffer[1];
			Data.payload = local_recv_buffer + 2; // points to payload

			switch(Data.Id)
			{
			case IpcID::WIFI_ADD :
				// incomplete. TODO Need to add the packetID field too.
				make_packet(Data.payload, Data.Size);
				HAL_SPI_Transmit_DMA(&hspi2, cmd_out_buffer, cmd_index);
				break;

			default:
				break;
			}

		}
	}
}

