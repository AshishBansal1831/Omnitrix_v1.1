/*
 * cmd_handler.cpp
 *
 *  Created on: 09-Jul-2026
 *      Author: ashish
 */

#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "spi.h"

#include "cmd_handler.h"

#if STATE_MACHINE_DECODE == 1
#include "InterfaceIO.h"

extern UartIO UartPort;

#endif


typedef uint8_t crc_t;

enum class cmd_decode_state_t
{
    HEADER, // 2Bytes
	PKT_NO, // current packet no
	T_PKT_NO, // total packets in this epoch
    PACKET_ID, // 1 Byte
    PACKET_SUB_ID,
    PAYLOAD,
    CRC_CHECK,
};

void Packet::Create(const packet_type_t Type, const uint8_t pkt_no, const uint8_t total_no, const uint8_t* src_payload)
{
	PacketID = Type;
	Pkt_No  = pkt_no;
	Totalno = total_no;
	memcpy(Payload, src_payload, PAYLOAD_MAX_LENGTH);
}

Packet G_Packets;
uint8_t cmd_payload_buffer[sizeof(Packet)];
uint8_t cmd_out_buffer[sizeof(Packet)];
size_t cmd_index;

crc_t calc_crc(const uint8_t *buffer, size_t Len)
{
    crc_t Crc = 0;
    for(size_t Iter = 0; Iter < Len; ++Iter)
    {
        Crc ^= buffer[Iter];
    }

    return Crc;
}

void send_payload(const packet_type_t type, uint8_t *Payload, const uint8_t TotalLength)
{
	Packet SendPacket;
	const uint8_t TotalPackets = TotalLength / PAYLOAD_MAX_LENGTH;
	uint8_t PacketCount = 0;
	UartPort.printf("total packet %d\r\n", TotalPackets);
	do
	{
		SendPacket.Create(type, PacketCount, TotalPackets, Payload);
		HAL_SPI_Transmit(&hspi2, (uint8_t *)(&SendPacket) , sizeof(SendPacket), 100);
		UartPort.printf("Sent packet %d\r\n", PacketCount);
	}while(PacketCount < TotalPackets);
}

check_cmd_status_t check_cmd(Packet *SpiPayload)
{
	if(	SpiPayload->Header[0] != 0xA5 ||
		SpiPayload->Header[1] != 0x5A)
	{
		return check_cmd_status_t::NO_CMD;
	}

	if(SpiPayload->Pkt_No > SpiPayload->Totalno)
	{
		return check_cmd_status_t::ERROR;
	}

	return check_cmd_status_t::RECV_PKT;
}
