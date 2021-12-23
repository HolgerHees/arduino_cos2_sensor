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

SoftwareSerial sensorSerial(D1, D2); // RX, TX

#include "src/log.h"
#include "src/eeprom.h"
#include "src/setup.html.h"
#include "src/setup.h"
#include "src/sensor.h"

#define LEDpin LED_BUILTIN

#define SETUP_MODE_PIN D5
#define CALIBRATION_MODE_PIN D6

WiFiClient wclient;
PubSubClient client(wclient);

void ledSignal(int offTime, int onTime)
{
  digitalWrite(LEDpin, 0);
  delay(onTime);
  digitalWrite(LEDpin, 1);
  delay(offTime);
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

    DEBUG_PRINT(F("Start calibration ..."));
    unsigned long start = millis();
    // wait for 20 minutes (+1) calibration time
    while(millis() < start + 21 * 60 * 1000)
    {
      ledSignal(2000,2000);
    }
    
    calibrateZero();

    ledSignal(2000,2000);

    DEBUG_PRINTLN(F(" done"));
  }

  DEBUG_PRINT(F("Connect wifi ..."));
  WiFi.mode(WIFI_STA);
  WiFi.begin(cfg[CFG_WIFI_SSID], cfg[CFG_WIFI_PASSWORD]);
  while(WiFi.status() != WL_CONNECTED)
  {
    ledSignal(500,500);
  }
  DEBUG_PRINTLN(F(" done"));

  DEBUG_PRINTLN(F("Start measuring"));
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
        unsigned long start = millis();
        int value = -1;
        // wait for max 60 seconds => can happen only durinmg the initial warmup phase, but there is no way to detect this phase after a deepSleep
        while(millis() < start + 60 * 1000)
        {
          value = readCO2UART();
          if( value != -1 ) break;
      
          ledSignal(1000,1000);
        }

        if( value != -1 )
        {
          client.publish(cfg[CFG_MQTT_CLIENT_TOPIC].c_str(), String(value).c_str());
          client.loop();
          client.disconnect();
  
          int interval = cfg[CFG_INTERVAL].toInt();
          ESP.deepSleep(interval * 60e6, WAKE_RFCAL);
        }
        else
        {
          while( true )
          {
            ledSignal(250,250);
          }
        }
      }
    }
    else
    {
      client.loop();
    }
  }
}
