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

constexpr uint8_t FRAME_SIZE = 64;

void IpcQueueCom::cpy_from(uint8_t data[], uint8_t len)
{
	std::lock_guard<FreeRtosMutex> lock(mutx);
	Length = len;
	memcpy(buffer, data, len);
}

uint16_t IpcQueueCom::cpy_to(uint8_t data[])
{
	std::lock_guard<FreeRtosMutex> lock(mutx);
	uint16_t read_len = Length;
	Length = 0;
	memcpy(data, buffer, read_len);
	return read_len;
}

extern UartIO UartPort;

// Task & Queue Handles
TaskHandle_t IPC_Taskhandle = NULL;
volatile QueueHandle_t CliToIpcQueue = NULL;
volatile QueueHandle_t SpiRxIntToIpcQueue = NULL;

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

uint8_t SpiRxByte = 0;

static uint8_t spi_tx_buffer[FRAME_SIZE];
static uint8_t spi_rx_buffer[FRAME_SIZE];
size_t spi_tx_len = 0;
volatile bool is_tx_pending = false;
static const uint8_t dummy_tx_buffer[FRAME_SIZE] = {0};

static void decode_packet(Packet &packet);
bool send_payload(packet_type_t type, const uint8_t *payload, uint8_t len);

void set_esp_ready(bool active) {
    HAL_GPIO_WritePin(ESP_READY_GPIO_Port, ESP_READY_Pin,
                      active ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

extern Time local_time;
extern FreeRtosMutex timeMutex;

void ipc_task(void *)
{
    IPC_Taskhandle = xTaskGetCurrentTaskHandle();

    IpcMessage cli_msg;
    uint8_t spi_byte = 0;
    Packet recv_spi_pkt;

    SpiRxIntToIpcQueue = xQueueCreate(64, sizeof(char));
    CliToIpcQueue = xQueueCreate(2, sizeof(IpcMessage));

    // Start listening for SPI

    while(1)
    {
        // -------------------------------------------------------------
        // 1. Process Pending CLI Messages from Queue (Non-blocking)
        // -------------------------------------------------------------
        if (CliToIpcQueue != NULL && xQueueReceive(CliToIpcQueue, &cli_msg, 0) == pdPASS)
        {
            UartPort.printf("IPC Recv Cmd ID: %d, Len: %d\r\n", cli_msg.id, cli_msg.length);

            switch(cli_msg.id)
            {
                case IpcID::WIFI_ADD:
                    send_payload(packet_type_t::ADD_WIFI_CREDENTIAL, cli_msg.payload, cli_msg.length);
                    break;

                default:
                    break;
            }
        }

        // -------------------------------------------------------------
        // 2. Process Incoming SPI Bytes from ISR Queue (10ms wait block)
        // -------------------------------------------------------------
        if (SpiRxIntToIpcQueue != NULL && xQueueReceive(SpiRxIntToIpcQueue, &spi_byte, pdMS_TO_TICKS(10)) == pdPASS)
        {
            // Feed byte to state machine parser
        	if(recv_spi_pkt.check_cmd(spi_byte) == check_cmd_status_t::RECV_PKT)
        	{
        		decode_packet(recv_spi_pkt);
        	}
        }
    }
}

// Prepares the hardware and signals the Master
bool send_payload(packet_type_t type, const uint8_t *payload, uint8_t len)
{
    if (is_tx_pending) {
        return false; // Busy sending previous frame
    }

    // 1. Clear and format the frame
    memset(spi_tx_buffer, 0, sizeof(spi_tx_buffer));
    spi_tx_buffer[0] = 0xA5; // Sync Byte 1
    spi_tx_buffer[1] = 0x5A; // Sync Byte 2
    spi_tx_buffer[2] = static_cast<uint8_t>(type);
    spi_tx_buffer[3] = len;

    if (payload && len > 0) {
        memcpy(&spi_tx_buffer[4], payload, len);
    }

    is_tx_pending = true;

    // 2. Pre-load SPI peripheral buffer BEFORE pulling pin HIGH
    HAL_SPI_TransmitReceive_IT(&hspi2, spi_tx_buffer, spi_rx_buffer, sizeof(spi_tx_buffer));

    // 3. Signal the ESP8266: "Hardware is loaded, clock out 64 bytes!"
    set_esp_ready(true);

    return true;
}


extern "C" void HAL_SPI_RxCpltCallback(SPI_HandleTypeDef *hspi)
{
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    if(hspi->Instance == SPI2)
    {
    	xQueueSendFromISR(SpiRxIntToIpcQueue, &SpiRxByte, &xHigherPriorityTaskWoken);
    	HAL_SPI_Receive_IT(&hspi2, &SpiRxByte, sizeof(SpiRxByte));
    }
}


extern "C" void HAL_SPI_TxRxCpltCallback(SPI_HandleTypeDef *hspi)
{
    if (hspi->Instance == SPI2)
    {
        BaseType_t xHigherPriorityTaskWoken = pdFALSE;

        // 1. Lower READY line
        set_esp_ready(false);
        is_tx_pending = false;

        // 2. If valid packet header received from ESP, queue it for processing
		xQueueSendFromISR(SpiRxIntToIpcQueue, spi_rx_buffer, &xHigherPriorityTaskWoken);

        // 3. Re-arm idle listener for the next transaction
        HAL_SPI_TransmitReceive_IT(&hspi2, (uint8_t*)dummy_tx_buffer, spi_rx_buffer, FRAME_SIZE);

        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
}

static void decode_packet(Packet &packet)
{
	switch(packet.PacketID)
	{
	case packet_type_t::REQUEST:
		if(packet.Payload[0] == 0)
		{
			// send the apyload created
		}
		break;

	case packet_type_t::ACK:
		if(packet.Payload[0] == 1)
		{
			// Notify the cli_task that ACK has been received
			break;
		}

	case packet_type_t::HEARTBEAT :
		{
			// set global status flag
			// for now use the mutex
			break;
		}
	case packet_type_t::TIME_DATE_DATA :
		{
			std::lock_guard<FreeRtosMutex> lock(timeMutex);
			memcpy(&local_time, packet.Payload, sizeof(local_time));
		}
		break;


	default:
		break;
	}
}
