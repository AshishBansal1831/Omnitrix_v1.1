/*
 * InterfaceIO.h
 *
 *  Created on: 10-Jul-2026
 *      Author: ashish
 */

#ifndef INTERFACEIO_H_
#define INTERFACEIO_H_

#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include "FreeRTOS.h"
#include "queue.h"
#include "semphr.h"


/*
 * Virtual Base Function, will be used as pure virtual function so UartIO and UsbCdcIO can be passed around.
 */
class InterfaceIO
{
protected:
	QueueHandle_t Queue;
//	SemaphoreHandle_t RxSem; // Data Receive  Semaphore

public:

	virtual void init(QueueHandle_t Q) = 0;
	virtual uint8_t receive_byte() = 0;
	virtual uint8_t receive_byte(uint8_t *byte, uint32_t TimeOut) = 0; // added this
	virtual void write_byte(uint8_t byte) = 0;
	virtual void write_buffer(uint8_t *data, uint16_t Length) = 0;
	void print(const char *ptr)
	{
		write_buffer((uint8_t*)ptr, strlen(ptr));
	}
	void printn(const char *ptr, const size_t len)
	{
		write_buffer((uint8_t*)ptr, len);
	}
	void printf(const char *format, ...)
	{
		char print_buffer[100];
		va_list args;
		va_start(args, format);

		int written = vsnprintf(print_buffer, sizeof(print_buffer), format, args);

		va_end(args);

		if (written > 0)
		{
			// Optimize: use printn since vsnprintf already calculated the exact string length
			printn(print_buffer, static_cast<size_t>(written));
		}
	}
};

class UartIO : public InterfaceIO
{
public :
	// Explicit Lifecycle/Setup override
	void init(QueueHandle_t Q/*,
			SemaphoreHandle_t RS*/) override;

	// Declaring the concrete overrides
	uint8_t receive_byte() override;
	uint8_t receive_byte(uint8_t *byte, uint32_t TimeOut) override;
	void write_byte(uint8_t byte) override;
	void write_buffer(uint8_t *data, uint16_t Length) override;
};

class UsbCdcIO : public InterfaceIO
{
public :
	// Explicit Lifecycle/Setup override
	void init(QueueHandle_t Q
			/*, SemaphoreHandle_t RS*/) override;

	// Declaring the concrete overrides
	uint8_t receive_byte() override;
	uint8_t receive_byte(uint8_t *byte, uint32_t TimeOut) override;
	void write_byte(uint8_t byte) override;
	void write_buffer(uint8_t *data, uint16_t Length) override;
};

#endif /* INTERFACEIO_H_ */
