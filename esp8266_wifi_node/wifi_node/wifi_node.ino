// The built-in LED on most NodeMCU boards is connected to GPIO 2 (D4).
// Note: On ESP8266, LOW usually turns the LED ON, and HIGH turns it OFF.
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <Ticker.h>
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


// --- NTP & Timezone Config ---
// UTC offset in seconds (e.g., IST = +5:30 -> 5.5 * 3600 = 19800)
const long GMT_OFFSET_SEC = 19800; 
const int DAYLIGHT_OFFSET_SEC = 0;
struct __attribute__((packed, aligned(4))) time_as_t {
    uint16_t year;     // e.g., 2026
    uint8_t  month;    // 1 - 12
    uint8_t  day;      // 1 - 31
    uint8_t  hours;    // 0 - 23
    uint8_t  minutes;  // 0 - 59
    uint8_t  seconds;  // 0 - 59
    uint32_t epoch;    // Unix Timestamp (4 bytes)
};

time_as_t cur_time;
Ticker spiTimer;
volatile bool sendPending = false;

void spi_handler();
void sendSpiTimePacket();

// Timer callback triggered every 100ms
void IRAM_ATTR onTimer() {
    sendPending = true;
}

void send_spi_packet(Packet *packet)
{
    if(packet == nullptr) return;
    uint8_t *buffer_ptr = (uint8_t *)(packet);
    for (size_t i = 0; i < sizeof(packet); i++, buffer_ptr++) 
    {
        SPI.transfer(*(buffer_ptr++));
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

    // configTime(GMT_OFFSET_SEC, DAYLIGHT_OFFSET_SEC, "pool.ntp.org", "time.nist.gov");
    // Serial.print("Waiting for NTP time sync");
    // time_t now = time(nullptr);
    // while (now < 8 * 3600 * 2) { // Wait until valid time is received
    //     delay(500);
    //     Serial.print(".");
    //     now = time(nullptr);
    // }
    // Serial.println("\nTime synced successfully!");
    delay(1000);
    Serial.println("\r\n\n\n\nInit done\r\n");

    // Start 100ms hardware ticker
    spiTimer.attach_ms(100, onTimer);
}

void loop()
{
    spi_handler();
    wifi_handler();
    if (sendPending) {
        sendPending = false;
        sendSpiTimePacket();
    }
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

    static uint16_t txIndex = 0;
    static uint16_t rxIndex = 0;
    static uint16_t expectedLength = 0;

    static const uint8_t empty_buffer[PAYLOAD_MAX_LENGTH] = {0};

    uint8_t PacketNo = 1;
    uint8_t TotalPacket = 1;

    check_cmd_status_t cmd_status; 

    switch(state)
    {
        //-----------------------------------------------------
        case State::IDLE:

            if(digitalRead(D1) == HIGH)
            {                
                Packet send_packet(packet_type_t::REQUEST, 1U, 1U, empty_buffer);
                send_spi_packet(&send_packet);
            }

            state = State::RECEIVE_HEADER;

            break;

        //-----------------------------------------------------
        case State::RECEIVE_HEADER:
            cmd_status = check_cmd(spi_receive_byte()); 
            if ( cmd_status == check_cmd_status_t::RECV_PKT)
            {                
                state = State::PROCESS_PACKET;
            }
            else if(cmd_status == check_cmd_status_t::SOME_PACKET || cmd_status == check_cmd_status_t::ERROR)
            {
                state = State::IDLE;
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

void sendSpiTimePacket() 
{
    time_t now = time(nullptr);
    struct tm timeinfo;
    localtime_r(&now, &timeinfo);

    // Populate packet structure
    cur_time.year     = (uint16_t)(timeinfo.tm_year + 1900);
    cur_time.month    = (uint8_t)(timeinfo.tm_mon + 1);
    cur_time.day      = (uint8_t)timeinfo.tm_mday;
    cur_time.hours    = (uint8_t)timeinfo.tm_hour;
    cur_time.minutes  = (uint8_t)timeinfo.tm_min;
    cur_time.seconds  = (uint8_t)timeinfo.tm_sec;
    cur_time.epoch    = (uint32_t)now;

    Packet time_pkt(packet_type_t::TIME_DATE_DATA, 1U, 1U, (uint8_t*)&cur_time);

    send_spi_packet(&time_pkt);
}
