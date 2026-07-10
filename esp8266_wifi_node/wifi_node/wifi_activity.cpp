#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <PubSubClient.h>
#include <LittleFS.h>

#include "wifi_activity.hpp"

#define GPIO_READ_PIN D1

constexpr char ConfigFileName[] = "/configfile.bin";

bool configReadFlag = false;

Configuration configurations;

WiFiClient wifiClient;
PubSubClient client(wifiClient);

void mqtt_handler();

void mqtt_callback(char* topic, byte* payload, unsigned int length)
{
    // Handle received messages
}

bool isNetworkAvailable(const char* ssid)
{
    int n = WiFi.scanNetworks();

    if (n <= 0)
    {
        return false;
    }

    for (int i = 0; i < n; i++)
    {
        if (WiFi.SSID(i) == ssid)
        {
            return true;
        }
    }

    WiFi.scanDelete();

    return false;
}

SequenceError init_sequence()
{
  SequenceError error = SequenceError::NO_ERROR;
  uint8_t Iter = 0;


  // Retrieve Data from LittleFS
  {
    File file = LittleFS.open(ConfigFileName, "r");
    
    if(!file)
    {
      Serial.println("file Not opened");
      return SequenceError::FILE_NOT_OPENED;
    }
    
    configReadFlag = true;
    file.read((uint8_t *)&configurations, sizeof(configurations));
    file.close();
  }


  // Working with sense Pin
  {
    pinMode(GPIO_READ_PIN, OUTPUT);
    digitalWrite(GPIO_READ_PIN, HIGH);

    delay(20);

    pinMode(GPIO_READ_PIN, INPUT);
  }

  // Wifi Connections
  {
    if(configurations.StoredCreds == 0)
    {
      Serial.print("No Wifi Credentials Found");
      return SequenceError::CREDENTIALS_NOT_FOUND;
    }

    if(configurations.StoredCreds < configurations.ActiveWifiIndex)
    {
      Serial.print("configurations.StoredCreds < configurations.ActiveWifiIndex");
      return SequenceError::CREDENTIALS_NOT_FOUND;
    }

    WiFi.mode(WIFI_STA);

    Serial.println("Scanning...");
    bool ConnectionFoundStatus = false;

    if(!isNetworkAvailable(configurations.WifiCredentials[configurations.ActiveWifiIndex].SSID) )
    {
      for(Iter = 0; Iter < configurations.StoredCreds ; ++Iter)
      {
        if (isNetworkAvailable(configurations.WifiCredentials[Iter].SSID) )
        {
          ConnectionFoundStatus = true;
          break;
        }
      }

      if(Iter == configurations.StoredCreds)
      {
        return SequenceError::WIFI_NOT_CONNECTED;
      }

      configurations.ActiveWifiIndex = Iter;
    }

    
  }
  
  return SequenceError::NO_ERROR;
}

void wifi_handler()
{
    enum class States
    {
        CONNECTING,
        CONNECTED
    };

    static States Cur_State = States::CONNECTING;
    static uint32_t LastAttempt = 0;

    switch(Cur_State)
    {
        case States::CONNECTING:

            if(WiFi.status() == WL_CONNECTED)
            {
                Serial.println("WiFi Connected");
                Cur_State = States::CONNECTED;
                break;
            }

            if(millis() - LastAttempt > 5000)
            {
                LastAttempt = millis();

                WiFi.begin(
                    configurations.WifiCredentials[
                        configurations.ActiveWifiIndex].SSID,

                    configurations.WifiCredentials[
                        configurations.ActiveWifiIndex].PASSWORD);
            }

            break;

        case States::CONNECTED:

            if(WiFi.status() != WL_CONNECTED)
            {
                Cur_State = States::CONNECTING;
                break;
            }

            mqtt_handler();

            break;
    }
}

void mqtt_handler()
{
    enum class States
    {
        SERVER_CONFIG,
        CONNECTING,
        PROCESS_MQTT
    };

    static States Cur_State = States::SERVER_CONFIG;
    static uint32_t LastReconnectAttempt = 0;

    switch(Cur_State)
    {
        //---------------------------------------------------------
        case States::SERVER_CONFIG:
        {
            client.setServer(
                configurations.MqttBrokers[configurations.ActiveBrokerIndex].host,
                configurations.MqttBrokers[configurations.ActiveBrokerIndex].port);

            client.setCallback(mqtt_callback);

            Cur_State = States::CONNECTING;
            break;
        }

        //---------------------------------------------------------
        case States::CONNECTING:
        {
            if(client.connected())
            {
                Cur_State = States::PROCESS_MQTT;
                break;
            }

            // Retry every 5 seconds
            if(millis() - LastReconnectAttempt < 5000)
                break;

            LastReconnectAttempt = millis();

            if(client.connect(
                    configurations.MqttBrokers[configurations.ActiveBrokerIndex].client_id,
                    configurations.MqttBrokers[configurations.ActiveBrokerIndex].username,
                    configurations.MqttBrokers[configurations.ActiveBrokerIndex].password))
            {
                Serial.println("MQTT Connected");

                client.subscribe("device/cmd");

                Cur_State = States::PROCESS_MQTT;
            }
            else
            {
                Serial.printf("MQTT Failed rc=%d\n", client.state());
            }

            break;
        }

        //---------------------------------------------------------
        case States::PROCESS_MQTT:
        {
            if(!client.connected())
            {
                Cur_State = States::CONNECTING;
                break;
            }

            client.loop();

            // Publish queue, receive queue etc.

            break;
        }
    }
}

uint8_t littlefs_handler(const uint8_t *data, uint16_t length)
{
    if(data == nullptr || length == 0)
    {
        return false;
    }

    ConfigCommand cmd = static_cast<ConfigCommand>(data[0]);

    switch(cmd)
    {
        //------------------------------------------------------
        case ConfigCommand::ADD_WIFI_CREDENTIAL:
        {
            if(configurations.StoredCreds >= MAX_WIFI_CREDENTIALS)
            {
                return false;
            }

            if(length != 1 + sizeof(configurations.WifiCredentials[0]))
            {
                return false;
            }

            memcpy(
                &configurations.WifiCredentials[configurations.StoredCreds],
                &data[1],
                sizeof(configurations.WifiCredentials[0]));

            configurations.StoredCreds++;

            break;
        }

        //------------------------------------------------------
        case ConfigCommand::REMOVE_WIFI_CREDENTIAL:
        {
            if(length != 2)
                return false;

            uint8_t index = data[1];

            if(index >= configurations.StoredCreds)
                return false;

            for(uint8_t i = index; i < configurations.StoredCreds - 1; i++)
            {
                configurations.WifiCredentials[i] =
                    configurations.WifiCredentials[i + 1];
            }

            configurations.StoredCreds--;

            if(configurations.ActiveWifiIndex >= configurations.StoredCreds)
                configurations.ActiveWifiIndex = 0;

            break;
        }

        //------------------------------------------------------
        case ConfigCommand::SET_ACTIVE_WIFI:
        {
            if(length != 2)
                return false;

            if(data[1] >= configurations.StoredCreds)
                return false;

            configurations.ActiveWifiIndex = data[1];

            break;
        }

        //------------------------------------------------------
        case ConfigCommand::ADD_MQTT_BROKER:
        {
            if(configurations.StoredBrokers >= MAX_MQTT_BROKERS_LIST)
                return false;

            if(length != 1 + sizeof(MQTTBroker))
                return false;

            memcpy(
                &configurations.MqttBrokers[configurations.StoredBrokers],
                &data[1],
                sizeof(MQTTBroker));

            configurations.StoredBrokers++;

            break;
        }

        //------------------------------------------------------
        case ConfigCommand::REMOVE_MQTT_BROKER:
        {
            if(length != 2)
                return false;

            uint8_t index = data[1];

            if(index >= configurations.StoredBrokers)
                return false;

            for(uint8_t i = index; i < configurations.StoredBrokers - 1; i++)
            {
                configurations.MqttBrokers[i] =
                    configurations.MqttBrokers[i + 1];
            }

            configurations.StoredBrokers--;

            if(configurations.ActiveBrokerIndex >= configurations.StoredBrokers)
                configurations.ActiveBrokerIndex = 0;

            break;
        }

        //------------------------------------------------------
        case ConfigCommand::SET_ACTIVE_BROKER:
        {
            if(length != 2)
                return false;

            if(data[1] >= configurations.StoredBrokers)
                return false;

            configurations.ActiveBrokerIndex = data[1];

            break;
        }

        //------------------------------------------------------
        case ConfigCommand::SET_DEBUG_PORT:
        {
            if(length != 2)
                return false;

            configurations.DebugPort = data[1];

            break;
        }

        //------------------------------------------------------
        case ConfigCommand::SET_DEBUG_MASK:
        {
            if(length != 2)
                return false;

            configurations.DebugMask = data[1];

            break;
        }

        //------------------------------------------------------
        case ConfigCommand::SAVE_CONFIGURATION:
        {
            File file = LittleFS.open(ConfigFileName, "w");

            if(!file)
                return false;

            file.write(
                (uint8_t *)&configurations,
                sizeof(configurations));

            file.close();

            break;
        }

        //------------------------------------------------------
        case ConfigCommand::LOAD_CONFIGURATION:
        {
            File file = LittleFS.open(ConfigFileName, "r");

            if(!file)
                return false;

            file.read(
                (uint8_t *)&configurations,
                sizeof(configurations));

            file.close();

            break;
        }

        //------------------------------------------------------
        case ConfigCommand::FACTORY_RESET:
        {
            memset(&configurations, 0, sizeof(configurations));
            break;
        }

        //------------------------------------------------------
        default:
            return false;
    }

    return true;
}