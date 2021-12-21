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

#include "src/log.h"
#include "src/eeprom.h"
#include "src/setup.html.h"
#include "src/setup.h"

#define LEDpin LED_BUILTIN

#define SETUP_MODE_PIN D5
#define CALIBRATION_MODE_PIN D6

WiFiClient wclient;
PubSubClient client(wclient);

SoftwareSerial sensorSerial(D1, D2); // RX, TX

int readCO2UART()
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

void setAutoCalibrate(boolean b)
{
  byte cmd_enableAutoCal[9] = { 0xFF, 0x01, 0x79, 0xA0, 0x00, 0x00, 0x00, 0x00, 0xE6 };
  byte cmd_disableAutoCal[9] = { 0xFF, 0x01, 0x79, 0x00, 0x00, 0x00, 0x00, 0x00, 0x86};
  if(b) sensorSerial.write(cmd_enableAutoCal,9);
  else sensorSerial.write(cmd_disableAutoCal,9);

  char result;
  while (Serial.available()) result = Serial.read();
}

void calibrateZero()
{
  byte cmd[9] = {0xFF, 0x01, 0x87, 0x00, 0x00, 0x00, 0x00, 0x00, 0x78};
  sensorSerial.write(cmd,9);

  char result;
  while (Serial.available()) result = Serial.read();
}

void setup()
{
  initEepromConfig();

  sensorSerial.begin(9600);

  Serial.begin(115200);
  delay(10); 

  DEBUG_PRINTLN("");

  pinMode(LEDpin, OUTPUT);
  pinMode(SETUP_MODE_PIN, INPUT_PULLUP);
  pinMode(CALIBRATION_MODE_PIN, INPUT_PULLUP);

  if( !digitalRead(SETUP_MODE_PIN) || cfg[CFG_WIFI_SSID][0] > 127 )
  {
    for (int i = 0; i < 2; i++) 
    {
      ledSignal(500,500);
    }
    mainSetup();
  }

  if( !digitalRead(CALIBRATION_MODE_PIN) )
  {
    setAutoCalibrate(false);

    DEBUG_PRINT("Start calibration ...");
    unsigned long start = millis();
    // wait for 20 minutes (+1) calibration time
    while(millis() < start + 21 * 60 * 1000)
    {
      ledSignal(2000,2000);
    }
    
    calibrateZero();

    ledSignal(2000,2000);

    DEBUG_PRINTLN(" done");
  }

  DEBUG_PRINT("Connect wifi ...");
  WiFi.mode(WIFI_STA);
  WiFi.begin(cfg[CFG_WIFI_SSID], cfg[CFG_WIFI_PASSWORD]);
  while(WiFi.status() != WL_CONNECTED)
  {
    ledSignal(500,500);
  }
  DEBUG_PRINTLN(" done");

  DEBUG_PRINT("Wait for sensor to become ready ...");
  unsigned long start = millis();
  int value = -1;
  // wait for 60 seconds
  while(millis() < start + 60 * 1000)
  {
    value = readCO2UART();
    if( value != -1 ) break;

    ledSignal(1000,1000);
  }

  if( value != -1 )
  {
    DEBUG_PRINTLN(" done");
    DEBUG_PRINTLN("Start measuring");
  }
  else
  {
    DEBUG_PRINTLN(" failed");
    while( true )
    {
      ledSignal(250,250);
    }
  }
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
        int value = readCO2UART();
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
}

void ledSignal(int offTime, int onTime)
{
  digitalWrite(LEDpin, 0);
  delay(onTime);
  digitalWrite(LEDpin, 1);
  delay(offTime);
}
