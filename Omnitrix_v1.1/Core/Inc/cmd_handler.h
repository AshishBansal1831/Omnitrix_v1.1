/*
 * cmd_handler.h
 *
 *  Created on: 09-Jul-2026
 *      Author: ashish
 */

#ifndef INC_CMD_HANDLER_H_
#define INC_CMD_HANDLER_H_

#define STATE_MACHINE_DECODE	1

#ifdef __cplusplus
extern "C" {
#endif
#define CMD_BUFFER_SIZE 550

#define PACKET_BUFFER_SIZE	50
#define PAYLOAD_SIZE	18U

constexpr uint8_t TOTAL_SPI_PACKT_SIZE = 24U; // 0xA5 0x5A PacketNo, TOtalPackets, PacketID, 18Bytes Payload

enum PACKET_TYPE : uint8_t
{
  TIME_DATE_DATA = 1,
  REQUEST,
  WIFI_CONFIG,
  ACK,
  DEVICE_STATUS,
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
