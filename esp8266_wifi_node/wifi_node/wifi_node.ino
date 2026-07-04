// The built-in LED on most NodeMCU boards is connected to GPIO 2 (D4).
// Note: On ESP8266, LOW usually turns the LED ON, and HIGH turns it OFF.
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <time.h>
#include <SPI.h>
#include <LittleFS.h>

#include "command_handler.h"

#define LED_PIN 2 

const char* ssid = "Swami Krupa 202_5G";
const char* password = "9545884567";
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

  Serial.println();
  // Serial.print("Connecting to ");
  // Serial.println(ssid);

  // // Set the ESP8266 to Station mode (connects to a Wi-Fi network)
  // WiFi.mode(WIFI_STA);

  // Serial.println("Scanning...");

  // int n = WiFi.scanNetworks();

  // if (n == 0)
  // {
  //     Serial.println("No networks found");
  // }
  // else
  // {
  //     Serial.printf("%d networks found\n", n);

  //     for (int i = 0; i < n; i++)
  //     {
  //         Serial.printf("%2d : %-32s RSSI=%4d Ch=%2d %s\n",
  //                       i + 1,
  //                       WiFi.SSID(i).c_str(),
  //                       WiFi.RSSI(i),
  //                       WiFi.channel(i),
  //                       (WiFi.encryptionType(i) == ENC_TYPE_NONE) ? "Open" : "Secured");
  //     }
  // }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  // Configure time with the NTP server and timezone
  configTime(TZ_India, "pool.ntp.org");

  // Wait until time is synced
  time_t now = time(nullptr);
  while (now < 8 * 3600 * 2) {
    delay(500);
    Serial.print(".");
    now = time(nullptr);
  }

  /// Ashish@Weather
  Serial.println("\nTime Synced!");

  SPI.beginTransaction(
      SPISettings(8000000, MSBFIRST, SPI_MODE0)
  );

   if (!LittleFS.begin())
  {
      Serial.println("LittleFS Mount Failed");

      // Optional: format on first boot
      LittleFS.format();
      LittleFS.begin();
  }
#if 0
  File file = LittleFS.open("/hello.txt", "w");
  Serial.print("something was done to file but nothing happened\r\n");
  if(file)
  {
      file.println("Hello Ashish Bansal");
      file.close();
  }
#endif

#if 0
  File file = LittleFS.open("/hello.txt", "r");
  Serial.print("something was done to file but nothing happened\r\n");
  if(file)
  {
      while(file.available())
      {
          Serial.write(file.read());
      }

      file.close();
  }
#endif
}

void loop() {
  time_t now = time(nullptr);
  struct tm* timeinfo = localtime(&now);

  // Pack the timestamp (HHMMSS) into a uint32_t
  const uint32_t Time = timeinfo->tm_sec + timeinfo->tm_min * 100 + timeinfo->tm_hour * 10000;
  
  // Generate your packet and update the global cmd_index
  cmd_index = make_packet((uint8_t*)&Time, 4);
  // printf("cmd Index - %d\r\n", cmd_index);
  
  // Print time
  Serial.print(asctime(timeinfo));
  Serial.print("\r");

  // --- STRICT SPI TRANSACTION FRAMING ---
  // 1. Drop CS LOW to tell the STM32 hardware slave to start listening
  digitalWrite(CS_PIN, LOW);

  // 2. Shift out the entire dynamic buffer over SPI in one go
  send_packet(); 

  // 3. Pull CS HIGH to tell the STM32 hardware slave the burst is complete
  digitalWrite(CS_PIN, HIGH);
  // --------------------------------------
  // Send an HTTP GET request every 10 seconds
  if ((millis() - lastTime) > timerDelay) {
    if (WiFi.status() == WL_CONNECTED) {
      WiFiClient client;
      HTTPClient http;
      
      // Specify destination URL
      http.begin(client, serverPath);
      
      // Send the HTTP GET request
      int httpCode = http.GET();
      
      if (httpCode > 0) {
        // HTTP header has been sent and Server response header has been handled
        Serial.printf("[HTTP] GET... code: %d\n", httpCode);

        // File found at server
        if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
          String payload = http.getString();
          Serial.println("\r\n--- Weather JSON Payload ---");
          Serial.println(payload);
          Serial.println("----------------------------\r\n");
        }
      } else {
        Serial.printf("[HTTP] GET failed, error: %s\n", http.errorToString(httpCode).c_str());
      }
      
      http.end();
    } else {
      Serial.println("WiFi Disconnected");
    }
    lastTime = millis();
  }
  delay(1000);
}
