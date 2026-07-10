// The built-in LED on most NodeMCU boards is connected to GPIO 2 (D4).
// Note: On ESP8266, LOW usually turns the LED ON, and HIGH turns it OFF.
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <time.h>
#include <SPI.h>
#include <LittleFS.h>

#include "command_handler.h"

#include "wifi_activity.hpp"

#define LED_PIN 2 

// Set POSIX Timezone string for India
const char* TZ_India = "IST-5:30";

extern uint8_t cmd_payload_buffer[CMD_BUFFER_SIZE];
extern uint8_t cmd_out_buffer[CMD_BUFFER_SIZE];
extern size_t cmd_index;

// Weather API Details
// Replace "London,UK" with your city and country code
// Replace "YOUR_API_KEY" with your actual OpenWeatherMap API key
String city = "London,UK";
String apiKey = "d0e233256e2932c48ee60ac7c469e5f8";
String serverPath = "http://api.openweathermap.org/data/2.5/weather?q=" + city + "&units=metric&appid=" + apiKey + "&units=metric";

unsigned long lastTime = 0;
unsigned long timerDelay = 10000; // Request weather every 10

#define CS_PIN 15

void spi_handler();

void send_packet ()
{
    for (size_t i = 0; i < cmd_index; i++) 
    {
        SPI.transfer(cmd_out_buffer[i]);
    }
}

void setup() 
{
  Serial.begin(115200);
  delay(10);

  pinMode(CS_PIN, OUTPUT);
  digitalWrite(CS_PIN, HIGH);
  SPI.begin();

  SPI.beginTransaction(
      SPISettings(8000000, MSBFIRST, SPI_MODE0)
  );

  LittleFS.begin();

  init_sequence();
}

void loop()
{
    spi_handler();
    wifi_handler();
}

uint8_t spi_receive_byte()
{
    return SPI.transfer(0xFF); 
}

void spi_handler()
{
    enum class State
    {
        IDLE,
        SEND_REQUEST,
        RECEIVE_HEADER,
        RECEIVE_PAYLOAD,
        PROCESS_PACKET
    };

    static State state = State::IDLE;

    // static Packet TxPacket;
    // static Packet RxPacket;

    static uint16_t txIndex = 0;
    static uint16_t rxIndex = 0;
    static uint16_t expectedLength = 0;

    switch(state)
    {
        //-----------------------------------------------------
        case State::IDLE:

            if(digitalRead(D1) == HIGH)
            {
                uint8_t payload = REQUEST;

                cmd_index = make_packet(&payload, 1);

                txIndex = 0;
                state = State::SEND_REQUEST;
            }

            break;

        //-----------------------------------------------------
        case State::SEND_REQUEST:
            send_packet();
            {
                rxIndex = 0;
                state = State::RECEIVE_HEADER;
            }

            break;

        //-----------------------------------------------------
        case State::RECEIVE_HEADER:

            if (check_cmd(spi_receive_byte()) )
            {                
                state = State::PROCESS_PACKET;
            }

            break;

        //-----------------------------------------------------
        case State::PROCESS_PACKET:

            // process here;
            process_data();
            state = State::IDLE;

            break;
    }
}
