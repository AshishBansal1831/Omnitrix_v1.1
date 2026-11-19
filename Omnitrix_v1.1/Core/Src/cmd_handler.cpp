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
#include "gpio.h"

#include "cmd_handler.h"

#if STATE_MACHINE_DECODE == 1
#include "InterfaceIO.h"

extern UartIO UartPort;

#endif

typedef uint8_t crc_t;

Packet G_Packets;
uint8_t cmd_payload_buffer[sizeof(Packet)];
uint8_t cmd_out_buffer[sizeof(Packet)];
size_t cmd_index;

crc_t Packet::calc_crc(const uint8_t *buffer, size_t Len)
{
    crc_t Crc = 0;
    for(size_t Iter = 0; Iter < Len; ++Iter)
    {
        Crc ^= buffer[Iter];
    }

    return Crc;
}

/*
    returns
*/
check_cmd_status_t Packet::check_cmd(const uint8_t Recv)
{
    check_cmd_status_t ret_status = check_cmd_status_t::NO_CMD;

#ifdef DEBUG
   static cmd_decode_state_t prev_state = HEADER;
   if(prev_state != this->Decode_state)
   {
       printf("Decode_state - %d\r\n", this->Decode_state);
       prev_state = this->Decode_state;
   }
#endif

    switch(Decode_state)
    {
        case cmd_decode_state_t::HEADER:
            if(this->NoBytes == 0 && Recv == 0xA5)
            {
                this->NoBytes++;
            }
            else if(this->NoBytes == 1 && Recv == 0x5A)
            {
                Decode_state = cmd_decode_state_t::LENGTH;
                this->Length = 0;
                this->NoBytes = 0;
                this->calculated_crc = 0;
            }
            else
            {
                ret_status = check_cmd_status_t::NO_CMD;
            }
            break;

        case cmd_decode_state_t::LENGTH: // only payloads length.
            this->calculated_crc ^= Recv;
            this->Length = Recv | this->Length << (this->NoBytes * 8);
            this->NoBytes++;
            if(this->NoBytes == 2)
            {
                this->Decode_state = cmd_decode_state_t::PACKET_ID;
            }
            break;

        case cmd_decode_state_t::PACKET_ID:
            this->calculated_crc ^= Recv;
            this->PacketID = static_cast<packet_type_t>(Recv);
            this->Decode_state = cmd_decode_state_t::PAYLOAD;
            this->Index = 0;

            break;

        case cmd_decode_state_t::PAYLOAD:
            this->calculated_crc ^= Recv;
            this->Payload[Index++] = Recv;
            if(this->Index >= this->Length)
            {
                this->Decode_state = cmd_decode_state_t::CALC_CRC;
            }
            break;

        case cmd_decode_state_t::CALC_CRC:
            if(this->calculated_crc == Recv)
            {
                ret_status = check_cmd_status_t::RECV_PKT;
            }
            break;

        default:
            break;
    }

    return ret_status;
}
/*
    Return length of final packet
*/
uint16_t Packet::create_packet(const packet_type_t pkt_type, uint8_t payload[], uint16_t Length, uint8_t *out_buff)
{
    uint16_t Index = 5;
    out_buff[0] = 0xA5;
    out_buff[1] = 0x5A;
    out_buff[2] = (uint8_t)pkt_type;

    memcpy(out_buff + Index, payload, Length);
    Index += Length;
    out_buff[3] = Index & 0xFF;
    out_buff[4] = (Index >> 8) & 0xFF;

    out_buff[Index] = this->calc_crc(out_buff + 2, Index);

    return Index;
}

