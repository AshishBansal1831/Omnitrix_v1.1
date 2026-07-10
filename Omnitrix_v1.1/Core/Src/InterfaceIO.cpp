/*
 * InterfaceIO.cpp
 *
 *  Created on: 10-Jul-2026
 *      Author: ashish
 */

#include "InterfaceIO.h"
#include "usart.h"
#include "stm32f4xx_ll_usart.h"
#include "usb_device.h"
#include "usbd_cdc_if.h"
#include "stream_buffer.h"

StreamBufferHandle_t UsbCdcStreamBuffer = NULL;

/*******************UartIO Handling***********************/
void UartIO::init(QueueHandle_t Q/*, SemaphoreHandle_t RS*/)
{
	Queue = Q;
//	RxSem = RS;

	MX_USART1_UART_Init();

    LL_USART_EnableIT_RXNE(USART1);
    NVIC_SetPriority(USART1_IRQn, 6);
    NVIC_EnableIRQ(USART1_IRQn);
}

uint8_t UartIO::receive_byte()
{
	uint8_t received_char = 0;
	xQueueReceive(RxSem, &received_char, portMAX_DELAY);
	return received_char;
}

void UartIO::write_byte(uint8_t byte)
{
    while (!LL_USART_IsActiveFlag_TXE(USART1))
    {
    	// Empty waiting
    }
    LL_USART_TransmitData8(USART1, byte);
}

void UartIO::write_buffer(uint8_t *data, uint16_t Length)
{
	for(uint16_t Iter = 0; Iter < Length; ++Iter)
	{
		write_byte(data[Iter]);
	}
}



/*******************UsbCdcIO Handling***********************/
void UsbCdcIO::init(QueueHandle_t Q/*, SemaphoreHandle_t RS */)
{
	if (UsbCdcStreamBuffer == NULL)
	{
		UsbCdcStreamBuffer = xStreamBufferCreate(256, 1);
	}
}

uint8_t UsbCdcIO::receive_byte()
{
	uint8_t rxByte = 0;

	if (UsbCdcStreamBuffer != NULL) {
		// This blocks completely (portMAX_DELAY) until a byte is available.
		// When CDC_Receive_FS pushes data, this instantly wakes up and returns it.
		xStreamBufferReceive(UsbCdcStreamBuffer, &rxByte, 1, portMAX_DELAY);
	}

	return rxByte;
}

void UsbCdcIO::write_byte(uint8_t byte)
{
	while(CDC_Transmit_FS(&byte, 1) == USBD_BUSY);
}

void UsbCdcIO::write_buffer(uint8_t *data, uint16_t Length)
{
	while(CDC_Transmit_FS(data, Length) == USBD_BUSY);
}
