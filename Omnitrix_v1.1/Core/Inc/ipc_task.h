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


struct manage_dma_tx
{
	uint8_t *active_buf;
	uint16_t recv_length;
};

struct IpcQueueCom
{
	uint8_t buffer[64];
	FreeRtosMutex mutx;

	void cpy(uint8_t data[], uint8_t len);
};


#endif /* INC_IPC_TASK_H_ */
