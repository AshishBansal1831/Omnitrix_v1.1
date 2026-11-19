#include <cstdint>
#include "c_types.h"
#ifndef _COMMAND_HANDLER_H_

#ifdef __cplusplus
extern "C" {
#endif
#define CMD_BUFFER_SIZE 550

constexpr uint8_t PAYLOAD_MAX_LENGTH = 128U;

typedef uint8_t crc_t;

enum class packet_type_t : uint8_t
{
    // --- 0x00 - 0x0F: System & Protocol Management ---
    NOP                     = 0x00, // Safe default / Uninitialized / Dummy byte
    ACK                     = 0x01, // Positive Acknowledgment
    REQUEST                 = 0x02, // Generic Data or State Request
    HEARTBEAT               = 0x03, // Link Keep-Alive

    // --- 0x10 - 0x2F: Telemetry & Real-Time Data ---
    TIME_DATE_DATA          = 0x10, // Timestamp / Sync Packet
    DEVICE_STATUS           = 0x11, // Battery, Sensor Health, RAM usage
    EVENT_LOG               = 0x12, // System Logs / Alerts
    SENSORS_DATA            = 0x13, // Raw or Processed Sensor Readings

    // --- 0x30 - 0x4F: Wi-Fi Configuration ---
    ADD_WIFI_CREDENTIAL     = 0x30,
    REMOVE_WIFI_CREDENTIAL  = 0x31,
    SET_ACTIVE_WIFI         = 0x32,
    GET_WIFI_STATUS         = 0x33,

    // --- 0x50 - 0x6F: MQTT & Connectivity Configuration ---
    ADD_MQTT_BROKER         = 0x50,
    REMOVE_MQTT_BROKER      = 0x51,
    SET_ACTIVE_BROKER       = 0x52,
    GET_MQTT_STATUS         = 0x53,

    // --- 0x70 - 0x8F: System Debug & Operations ---
    SET_DEBUG_PORT          = 0x70,
    SET_DEBUG_MASK          = 0x71,
    SAVE_CONFIGURATION      = 0x72,
    LOAD_CONFIGURATION      = 0x73,
    FACTORY_RESET           = 0x74,

    // --- 0x90 - 0xAF: Firmware Updates & Bootloader ---
    BOOTLOADER_ENTER        = 0x90,
    BOOTLOADER_DATA         = 0x91, // OTA Flash Chunk
    BOOTLOADER_VERIFY       = 0x92,
    BOOTLOADER_EXIT         = 0x93,

    // --- 0xFF: Malformed / Invalid ---
    INVALID_PACKET_TYPE     = 0xFF
};

enum class cmd_decode_state_t 
{
    HEADER, // 2Bytes
    LENGTH,
    PACKET_ID, // 1 Byte
    PAYLOAD,
    CALC_CRC,
};

enum class check_cmd_status_t
{
    NO_CMD,
    SOME_PACKET,
    ERROR,
    RECV_PKT
};

struct Packet 
{
    uint16_t Length;
    uint16_t NoBytes;
    uint16_t Index;
    crc_t calculated_crc;

    cmd_decode_state_t Decode_state = cmd_decode_state_t::HEADER;

	packet_type_t PacketID;
	uint8_t Payload[PAYLOAD_MAX_LENGTH] {};

    check_cmd_status_t check_cmd(const uint8_t Recv);
    uint16_t create_packet(const packet_type_t pkt_type, uint8_t payload[], uint16_t Length, uint8_t *out_buff);
    crc_t calc_crc(const uint8_t *buffer, size_t Len);
};

check_cmd_status_t check_cmd(const uint8_t Recv);
void process_data();
#ifdef __cplusplus
}
#endif
#endif /* _COMMAND_HANDLER_H_ */