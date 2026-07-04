#include <stdint.h>
#include <stddef.h>

#include "command_handler.h"

typedef uint8_t crc_t;

typedef enum 
{
    HEADER, // 2Bytes
    PROTOCOL_VERSION,
    FLAGS, // 1 Byte
    PACKET_ID, // 1 Byte
    LENGTH,
    PAYLOAD,
    CRC_CHECK,
}CMD_DECODE_STATE;

typedef struct 
{
    uint16_t Header;
    uint8_t ProVer;
    uint8_t Flags;
    uint8_t PacketID;
    uint16_t Length;
    uint8_t *Buffer_pointer;
    uint8_t Recv_Crc;
    uint8_t Calc_Crc;
}Packet;

Packet G_Packets;
uint8_t cmd_payload_buffer[CMD_BUFFER_SIZE];
uint8_t cmd_out_buffer[CMD_BUFFER_SIZE];
size_t cmd_index;

crc_t calc_crc(const uint8_t *buffer, size_t Len)
{
    crc_t Crc = 0;
    for(int Iter = 0; Iter < Len; ++Iter)
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
        // printf("crc - %02X %d\r\n", Crc, length);
        cmd_out_buffer[Index] = Crc;
    }

    return Index + 1;
}

uint8_t check_cmd(const uint8_t Recv)
{
    static CMD_DECODE_STATE Decode_state = HEADER;

//    static CMD_DECODE_STATE prev_state = HEADER;
    static size_t Index = 0;
    static uint16_t NumOfBytes = 0;
    Packet *packet = &G_Packets;
//    if(prev_state != Decode_state)
//    {
//        printf("Decode_state - %d\r\n", Decode_state);
//        prev_state = Decode_state;
//    }


    switch(Decode_state)
    {
        case HEADER:
            if(NumOfBytes == 0 && Recv == 0xA5)
            {
                NumOfBytes++;
            }
            else if(NumOfBytes == 1 && Recv == 0x5A)
            {
                Decode_state = PROTOCOL_VERSION;
                Index = 0;
                NumOfBytes = 0;
                packet->Calc_Crc = 0;
            }
            break;

        case PROTOCOL_VERSION:
            packet->ProVer = Recv;
            Decode_state = FLAGS;
            packet->Calc_Crc ^= Recv;
            break;

        case FLAGS:
            packet->Calc_Crc ^= Recv;
            packet->Flags = Recv;
            Decode_state = PACKET_ID;
            break;

        case PACKET_ID:
            packet->PacketID = Recv;
            Decode_state = LENGTH;
            packet->Length = 0;
            NumOfBytes = 0;
            packet->Calc_Crc ^= Recv;
            break;

        case LENGTH:
            packet->Length = (packet->Length << 8)  | (uint16_t)Recv ;
//            printf("Got Length - %d\r\n", packet->Length);
            NumOfBytes++;
            packet->Calc_Crc ^= Recv;

            if(NumOfBytes == sizeof(packet->Length))
            {
                if(packet->Length == 0 || packet->Length > CMD_BUFFER_SIZE)
                {
                    // printf("Got length - 0\r\n");
                    Decode_state = HEADER;
                }
                else
                {
                    Index = 0;
//                    printf("Got Length - %d\r\n", packet->Length);
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
                // printf("Got the Packet\r\n");
                return 1;
            }
            // printf("CRC Mismatch %02X %02X\r\n", packet->Calc_Crc, packet->Recv_Crc);
            break;

        default:
            break;
    }

    return 0;
}


