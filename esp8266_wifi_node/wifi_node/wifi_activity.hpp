#ifndef __WIFI_ACTIVITY_H_
#define __WIFI_ACTIVITY_H_

#include <stdint.h>

constexpr uint8_t WIFI_SSID_MAX_CHAR_LEN = 32;
constexpr uint8_t WIFI_PASSWORD_MAX_CHAR_LEN = 63;

constexpr uint8_t MAX_WIFI_CREDENTIALS = 5;
constexpr uint8_t MAX_MQTT_BROKERS_LIST = 5;

constexpr uint8_t MAX_MQTT_NAME_LENGTH = 30;

enum class SequenceError
{
  NO_ERROR,
  FILE_NOT_OPENED,
  CREDENTIALS_NOT_FOUND,
  WIFI_NOT_CONNECTED,
  MQTT_NOT_CONNECTED
};

struct MQTTBroker
{
    char name[16];

    char host[64];
    uint16_t port;

    char username[32];
    char password[32];

    char client_id[32];

    bool use_tls;
    uint16_t keep_alive;

    bool clean_session;
};

struct __attribute__((packed)) Configuration
{
  struct 
  {
    char SSID[WIFI_SSID_MAX_CHAR_LEN];
    char PASSWORD[WIFI_PASSWORD_MAX_CHAR_LEN];
  }WifiCredentials[MAX_WIFI_CREDENTIALS];

  uint8_t ActiveWifiIndex;
  uint8_t StoredCreds;
  
  MQTTBroker MqttBrokers[MAX_MQTT_BROKERS_LIST];
  uint8_t ActiveBrokerIndex;
  uint8_t StoredBrokers;
  uint16_t MqttPort;

  uint8_t DebugPort;
  uint32_t DebugUartBaudrate;
  uint8_t DebugMask;
};

SequenceError init_sequence();
void wifi_handler();
void mqtt_handler();

#endif /* __WIFI_ACTIVITY_H_ */