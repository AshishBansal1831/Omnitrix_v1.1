/*
 * ipc_task.c
 *
 *  Created on: 09-Jul-2026
 *      Author: ashish
 */
#define FREERTOS

#include <limits.h> // Provides ULONG_MAX
#include "spi.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include "ipc_task.h"
#include "cmd_handler.h"
#include "InterfaceIO.h"

// Define Notification Bitmasks
#define NOTIFY_BIT_BUF1_READY  (1UL << 0)
#define NOTIFY_BIT_BUF2_READY  (1UL << 1)

void IpcQueueCom::cpy(uint8_t data[], uint8_t len)
{
	std::lock_guard<FreeRtosMutex> lock(mutx);
	memcpy(buffer, data, len);
}

extern UartIO UartPort;

// Task & Queue Handles
TaskHandle_t IPC_Taskhandle = NULL;
volatile QueueHandle_t CliToIpcQueue = NULL;

IpcQueueCom IpcCom;

// SPI Double Buffering for DMA
alignas(4) uint8_t dma_ipc_rx_buffer1[sizeof(Packet)];
alignas(4) uint8_t dma_ipc_rx_buffer2[sizeof(Packet)];

uint8_t *dma_ipc_rx_active_buf = dma_ipc_rx_buffer1;



struct CliToIpcData
{
    IpcID Id;
    uint8_t Size;
    uint8_t *payload;
};

void ipc_task(void *)
{
    IPC_Taskhandle = xTaskGetCurrentTaskHandle();

    // Use safe thread-local stack memory instead of dynamic 'new'
    static uint8_t *local_recv_buffer;
    CliToIpcData Data;
    uint32_t ulNotificationValue = 0;

    Packet* pkt = nullptr;
    IpcCom.mutx.init();

    // Start initial SPI DMA Receive
    HAL_SPI_Receive_DMA(&hspi2, dma_ipc_rx_buffer1, sizeof(Packet));

    while(1)
    {
        // 2. Wait for SPI Interrupts or CLI Queue (Max 10ms block time)
        if(xTaskNotifyWait(0x00, ULONG_MAX, &ulNotificationValue, pdMS_TO_TICKS(10)) == pdPASS)
        {
        	pkt = nullptr;
            // Process Buffer 1
            if(ulNotificationValue & NOTIFY_BIT_BUF1_READY)
            {
                pkt = reinterpret_cast<Packet*>(dma_ipc_rx_buffer1);
            }
            else if(ulNotificationValue & NOTIFY_BIT_BUF2_READY)
            {
                pkt = reinterpret_cast<Packet*>(dma_ipc_rx_buffer2);
            }
            else if(ulNotificationValue & (1 << 2))
            {
            	local_recv_buffer = IpcCom.buffer;
            	Data.Id  	 = static_cast<IpcID>(local_recv_buffer[0]);
				Data.Size 	 = local_recv_buffer[1];
				Data.payload = local_recv_buffer + 2;

				UartPort.printf("ID : %d Size : %d\r\n", Data.Id, Data.Size);

				switch(Data.Id)
				{
					case IpcID::WIFI_ADD:
						send_payload(packet_type_t::ADD_WIFI_CREDENTIAL, Data.payload, Data.Size);
						break;

					default:
						break;
				}
            }


            if(pkt != nullptr)
            {

            }
        }
    }
}

extern "C" void HAL_SPI_RxCpltCallback(SPI_HandleTypeDef *hspi)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    if(hspi->Instance == SPI2)
    {
        uint32_t notify_bit = 0;

        if(dma_ipc_rx_active_buf == dma_ipc_rx_buffer1)
        {
            notify_bit = NOTIFY_BIT_BUF1_READY;
            dma_ipc_rx_active_buf = dma_ipc_rx_buffer2; // Swap to Buffer 2
        }
        else
        {
            notify_bit = NOTIFY_BIT_BUF2_READY;
            dma_ipc_rx_active_buf = dma_ipc_rx_buffer1; // Swap to Buffer 1
        }

        // Restart DMA immediately on the idle buffer
        HAL_SPI_Receive_DMA(hspi, dma_ipc_rx_active_buf, sizeof(Packet));

        // Notify IPC task of completed buffer
        if(IPC_Taskhandle != NULL)
        {
            xTaskNotifyFromISR(
                IPC_Taskhandle,
                notify_bit,
                eSetBits,
                &xHigherPriorityTaskWoken
            );

            portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
        }
    }
}
