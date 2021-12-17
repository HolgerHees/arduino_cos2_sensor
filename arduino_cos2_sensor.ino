#include <SoftwareSerial.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <PubSubClient.h>
#include <EEPROM.h>

ESP8266WebServer server;
IPAddress apIP(192, 168, 4, 1);

String cfg[9];
#define CFG_WIFI_SSID 0
#define CFG_WIFI_PASSWORD 1
#define CFG_MQTT_HOST 2
#define CFG_MQTT_PORT 3
#define CFG_MQTT_USER 4
#define CFG_MQTT_PASSWORD 5
#define CFG_MQTT_CLIENT_NAME 6
#define CFG_MQTT_CLIENT_TOPIC 7
#define CFG_INTERVAL 8

#define MAX_FIELD_LENGTH 60

#include "src/Eeprom.h"
#include "src/Setup.html.h"
#include "src/Setup.h"

#define LEDpin LED_BUILTIN

WiFiClient wclient;
PubSubClient client(wclient);

SoftwareSerial sensorSerial(D2, D1); // RX, TX

int readSensor()
{
  byte cmd[9] = {0xFF, 0x01, 0x86, 0x00, 0x00, 0x00, 0x00, 0x00, 0x79};
  char result[9];
  sensorSerial.write(cmd, 9);
  sensorSerial.readBytes(result, 9);
  if(result[0] != 0xFF || result[1] != 0x86) return -1;
  int high = (int) result[2];
  int low = (int) result[3];
  int ppm = (256 * high) + low;
  return ppm;
}

void setup()
{
  initEepromConfig();

  // check for setup mode
  Serial.begin(115200);
  while(Serial.available()) Serial.read();
  
  Serial.println("Check");
  delay(10);
  
  String inData = "";
  while(Serial.available())
  {
    char recieved = Serial.read();
    inData += recieved;
  }
  if( inData.startsWith("Check") )
  {
    pinMode(LEDpin, OUTPUT);
    digitalWrite(LEDpin, 0);
    mainSetup();
  }

  if(cfg[CFG_WIFI_SSID][0] > 127) 
  {
    pinMode(LEDpin, OUTPUT);
    digitalWrite(LEDpin, 0);
    mainSetup();
  }
  
  WiFi.mode(WIFI_STA);
  WiFi.begin(cfg[CFG_WIFI_SSID], cfg[CFG_WIFI_PASSWORD]);
  
  int timout = 0;
  pinMode(LEDpin, OUTPUT);
  while(WiFi.status() != WL_CONNECTED)
  {
    timout++;
    if(timout > 60)
    {
      digitalWrite(LEDpin, 0);
      delay(5000);
      digitalWrite(LEDpin, 1);
      ESP.deepSleep(1e6,WAKE_RFCAL);
      delay(100);
    }        // End timeout
    delay(300);
    digitalWrite(LEDpin, 0);
    delay(300);
    digitalWrite(LEDpin, 1);
  }

  Serial.println("");
  Serial.println("Start");
  
  pinMode(2, OUTPUT);
  digitalWrite(2, 0);
  
  sensorSerial.begin(9600);
}

void loop()
{
  if(WiFi.status() == WL_CONNECTED)
  {
    if(!client.connected())
    {
      client.setServer(cfg[CFG_MQTT_HOST].c_str(), cfg[CFG_MQTT_PORT].toInt());
      if(client.connect(cfg[CFG_MQTT_CLIENT_NAME].c_str(),cfg[CFG_MQTT_USER].c_str(),cfg[CFG_MQTT_PASSWORD].c_str()))
      {
        int value = readSensor();
        client.publish(cfg[CFG_MQTT_CLIENT_TOPIC].c_str(), String(value).c_str());
        client.loop();
        client.disconnect();
        int interval = cfg[CFG_INTERVAL].toInt();
        ESP.deepSleep(interval * 60e6, WAKE_RFCAL);
      }
    }
    else
    {
      client.loop();
    }
  }
  yield();
}
