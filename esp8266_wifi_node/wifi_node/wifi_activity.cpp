#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <LittleFS.h>

#include "wifi_activity.hpp"
#include "wifi_manager.hpp"

#define GPIO_READ_PIN D1

constexpr char ConfigFileName[] = "/configfile.bin";

bool configReadFlag = false;

Configuration configurations;

void mqtt_handler();

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

  // MQTT
  {
    if(configurations.StoredBrokers == 0)
    {
      return SequenceError::MQTT_NOT_CONNECTED;
    }

    client.setServer(configuration.MqttBrokers[configurations.] ,configurations.MqttPort);
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

  static States Cur_State;

  switch(Cur_State)
  {
    case States::CONNECTING :
      if(WiFi.status() == WL_CONNECTED)
      {
          state = CONNECTED;          
      }
      break;

    case States::CONNECTED :
      if(WiFi.status() == WL_CONNECTED)
      {
        mqtt_handler();
      }
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



  static States Cur_State;
}