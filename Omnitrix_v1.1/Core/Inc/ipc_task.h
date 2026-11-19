/*
 * ipc_task.h
 *
 *  Created on: 20-Jul-2026
 *      Author: ashish
 */

#ifndef INC_IPC_TASK_H_
#define INC_IPC_TASK_H_

#include <stdint.h>
#include "cpp_mutex.hpp"

constexpr uint8_t MAX_BUFFER_SIZE = 64;

enum IpcID : uint8_t
{
	ESP_AWAKE,

	WIFI_STATUS,
	WIFI_CONNECT,
	WIFI_ADD,
	WIFI_REMOVE,
	WIFI_DISCONNECT,

	MQTT_STATUS,
	MQTT_NEW,
	MQTT_CONNECT,
	MQTT_REMOVE
};

enum IpcTask_Notify_Mask : uint8_t
{
	CLI_TO_IPC_NOTIFY = 1 << 0,

};

// Define an IPC Message Envelope
struct IpcMessage
{
    IpcID id;
    uint8_t length;
    uint8_t payload[64]; // Fixed inline storage (no heap/pointers needed!)
};


struct IpcQueueCom
{
	uint8_t buffer[64];
	uint16_t Length;
	FreeRtosMutex mutx;

	void cpy_from(uint8_t data[], uint8_t len);
	uint16_t  cpy_to(uint8_t data[]);
};


#endif /* INC_IPC_TASK_H_ */
