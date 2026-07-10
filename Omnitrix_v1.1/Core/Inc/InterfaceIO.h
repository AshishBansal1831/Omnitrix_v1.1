/*
 * InterfaceIO.h
 *
 *  Created on: 10-Jul-2026
 *      Author: ashish
 */

#ifndef INTERFACEIO_H_
#define INTERFACEIO_H_

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
	SemaphoreHandle_t RxSem; // Data Receive  Semaphore

public:

	virtual void init(QueueHandle_t Q) = 0;
	virtual uint8_t receive_byte() = 0;
	virtual void write_byte(uint8_t byte) = 0;
	virtual void write_buffer(uint8_t *data, uint16_t Length) = 0;
};

class UartIO : public InterfaceIO
{
public :
	// Explicit Lifecycle/Setup override
	void init(QueueHandle_t Q/*,
			SemaphoreHandle_t RS*/) override;

	// Declaring the concrete overrides
	uint8_t receive_byte() override;
	void write_byte(uint8_t byte) override;
	void write_buffer(uint8_t *data, uint16_t Length);
};

class UsbCdcIO : public InterfaceIO
{
public :
	// Explicit Lifecycle/Setup override
	void init(QueueHandle_t Q
			/*SemaphoreHandle_t RS*/) override;

	// Declaring the concrete overrides
	uint8_t receive_byte() override;
	void write_byte(uint8_t byte) override;
	void write_buffer(uint8_t *data, uint16_t Length);
};

#endif /* INTERFACEIO_H_ */
