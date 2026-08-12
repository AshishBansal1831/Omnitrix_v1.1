/*
 * cmd_handler.cpp
 *
 *  Created on: 09-Jul-2026
 *      Author: ashish
 */

#include <stdint.h>
#include <stddef.h>
#include <string.h>

#include "cmd_handler.h"

#if STATE_MACHINE_DECODE == 1
#include "InterfaceIO.h"

extern UartIO UartPort;

#endif


typedef uint8_t crc_t;

enum CMD_DECODE_STATE
{
    HEADER, // 2Bytes
    PACKET_ID, // 1 Byte
    LENGTH,
    PACKET_SUB_ID,
    PAYLOAD,
    CRC_CHECK,
};

struct Packet
{
    uint16_t Header;
    uint8_t ProVer;
    uint8_t Flags;
    enum PACKET_TYPE PacketID;
    uint16_t Length;
    uint8_t Buffer[PACKET_BUFFER_SIZE];
    uint8_t Recv_Crc;
    uint8_t Calc_Crc;
};

Packet G_Packets;
uint8_t cmd_payload_buffer[CMD_BUFFER_SIZE];
uint8_t cmd_out_buffer[CMD_BUFFER_SIZE];
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

/*
 * return  length of bytes written (Index variable)
 */
uint16_t make_packet(uint8_t *payload, uint16_t length)
{
    uint16_t Index = 2;
    uint8_t Crc = 0;
    if(payload != NULL)
    {
        cmd_out_buffer[0] = 0xA5;
        cmd_out_buffer[1] = 0x5A;
        cmd_out_buffer[2] = 0;
        Index = 3;

        // 2 bytes Length
        Index += sizeof(length);

        //Payload
        memcpy(cmd_out_buffer + Index, payload, length);
        Index += length;

        cmd_out_buffer[2] = (Index >> 8) & 0xFF;
        cmd_out_buffer[3] = Index & 0xFF;

        // XOR CRC
        Crc= calc_crc(cmd_out_buffer, length);
        // UartPort.printf("crc - %02X %d\r\n", Crc, length);
        cmd_out_buffer[Index] = Crc;
    }

    return Index + 1;
}

uint8_t check_cmd(const uint8_t Recv)
{
    static CMD_DECODE_STATE Decode_state = HEADER;
#if STATE_MACHINE_DECODE == 1
    static CMD_DECODE_STATE prev_state = HEADER;
#endif
    static size_t Index = 0;
    static uint16_t NumOfBytes = 0;
    Packet *packet = &G_Packets;
#if STATE_MACHINE_DECODE == 1
    if(prev_state != Decode_state)
    {
        UartPort.printf("Decode_state - %d\r\n", Decode_state);
        prev_state = Decode_state;
    }
#endif

    switch(Decode_state)
    {
        case HEADER:
            if(NumOfBytes == 0 && Recv == 0xA5)
            {
                NumOfBytes++;
            }
            else if(NumOfBytes == 1 && Recv == 0x5A)
            {
                Decode_state = PACKET_ID;
                Index = 0;
                NumOfBytes = 0;
                packet->Calc_Crc = 0;
            }
            break;

        case PACKET_ID:
            packet->PacketID = (PACKET_TYPE)Recv;
            Decode_state = LENGTH;
            packet->Length = 0;
            NumOfBytes = 0;
            packet->Calc_Crc ^= Recv;
            break;

        case LENGTH:
            packet->Length = (packet->Length << 8)  | (uint16_t)Recv ;
#if STATE_MACHINE_DECODE == 1
            UartPort.printf("Got Length - %d\r\n", packet->Length);
#endif
            NumOfBytes++;
            packet->Calc_Crc ^= Recv;

            if(NumOfBytes == sizeof(packet->Length))
            {
                if(packet->Length == 0 || packet->Length > CMD_BUFFER_SIZE)
                {
#if STATE_MACHINE_DECODE == 1
                	UartPort.printf("Got length - 0\r\n");
#endif
                    Decode_state = HEADER;
                }
                else
                {
                    Index = 0;
#if STATE_MACHINE_DECODE == 1
                    UartPort.printf("Got Length - %d\r\n", packet->Length);
#endif
                    NumOfBytes = 0;
                    Decode_state = PAYLOAD;
                }
            }
            break;

        case PAYLOAD:
            packet->Calc_Crc ^= Recv;
            cmd_payload_buffer[Index++] = Recv;
            if(Index >= packet->Length)
            {
                Decode_state = CRC_CHECK;
                packet->Recv_Crc = 0;
            }
            break;

        case CRC_CHECK:
            packet->Recv_Crc = Recv;
            Decode_state = HEADER;
            if(packet->Recv_Crc == packet->Calc_Crc) //CRC Correct
            {
#if STATE_MACHINE_DECODE == 1
                 UartPort.print("Got the Packet\r\n");
#endif
                return 1;
            }
#if STATE_MACHINE_DECODE == 1
             UartPort.printf("CRC Mismatch %02X %02X\r\n", packet->Calc_Crc, packet->Recv_Crc);
#endif
            break;

        default:
            break;
    }

    return 0;
}





