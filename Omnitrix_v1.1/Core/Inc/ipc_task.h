/*
 * ipc_task.h
 *
 *  Created on: 20-Jul-2026
 *      Author: ashish
 */

#ifndef INC_IPC_TASK_H_
#define INC_IPC_TASK_H_

#include <stdint.h>

constexpr uint8_t MAX_BUFFER_SZIE = 64;

struct manage_dma_tx
{
	uint8_t *active_buf;
	uint16_t recv_length;
};


#endif /* INC_IPC_TASK_H_ */
