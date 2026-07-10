/*
 * cmd_handler.h
 *
 *  Created on: 09-Jul-2026
 *      Author: ashish
 */

#ifndef INC_CMD_HANDLER_H_
#define INC_CMD_HANDLER_H_


#ifdef __cplusplus
extern "C" {
#endif
#define CMD_BUFFER_SIZE 550

enum PACKET_TYPE
{
  DATA,
  CONFIG,
  REQUEST,
  ACK,
  EVENT_LOG,
  BOOTLOADER,

};

uint16_t make_packet(uint8_t *payload, uint16_t length);
uint8_t check_cmd(const uint8_t Recv);
void process_data();
#ifdef __cplusplus
}
#endif


#endif /* INC_CMD_HANDLER_H_ */
