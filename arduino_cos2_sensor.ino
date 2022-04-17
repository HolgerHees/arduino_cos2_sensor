#include <SoftwareSerial.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <PubSubClient.h>
#include <EEPROM.h>

ESP8266WebServer server;
IPAddress apIP(192, 168, 4, 1);

String cfg[10];
#define CFG_WIFI_SSID 0
#define CFG_WIFI_PASSWORD 1
#define CFG_MQTT_HOST 2
#define CFG_MQTT_PORT 3
#define CFG_MQTT_USER 4
#define CFG_MQTT_PASSWORD 5
#define CFG_MQTT_CLIENT_NAME 6
#define CFG_MQTT_CLIENT_TOPIC 7
#define CFG_SENSOR_INTERVAL 8
#define CFG_SENSOR_AUTO_CALIBRATE 9

#define MAX_FIELD_LENGTH 60

SoftwareSerial sensorSerial(D1, D2); // RX, TX

#include "src/log.h"
#include "src/eeprom.h"
#include "src/sensor.h"
#include "src/setup.html.h"
#include "src/setup.h"

#define LEDpin LED_BUILTIN

#define SETUP_MODE_PIN D5
#define CALIBRATION_MODE_PIN D6

WiFiClient wclient;
PubSubClient client(wclient);

long lastReconnectAttempt = 0;

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
    
    if( !digitalRead(SETUP_MODE_PIN) )
    //|| cfg[CFG_WIFI_SSID][0] > 127 )
    {
        for (int i = 0; i < 2; i++) 
        {
            ledSignal(500,500);
        }
        mainSetup();
    }
    
    if( !digitalRead(CALIBRATION_MODE_PIN) )
    {
        if( cfg[CFG_SENSOR_AUTO_CALIBRATE] != "1" )
        {
            cfg[CFG_SENSOR_AUTO_CALIBRATE] = "1";
            writeEepromConfig(); 
            setAutoCalibrate( true );
        }
        
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
    
    WiFi.mode(WIFI_STA);
    client.setServer(cfg[CFG_MQTT_HOST].c_str(), cfg[CFG_MQTT_PORT].toInt());
}

void loop()
{
    if( WiFi.status() != WL_CONNECTED )
    {
        DEBUG_PRINT(F("Connect wifi ..."));
        WiFi.begin(cfg[CFG_WIFI_SSID], cfg[CFG_WIFI_PASSWORD]);
        while(WiFi.status() != WL_CONNECTED)
        {
            DEBUG_PRINT(F("."));
            ledSignal(500,500);
        }
        DEBUG_PRINTLN(F(" done"));
    }
    
    while( !client.connected() )
    {
        DEBUG_PRINT(F("Connect mqtt ..."));
        if( client.connect(cfg[CFG_MQTT_CLIENT_NAME].c_str(),cfg[CFG_MQTT_USER].c_str(),cfg[CFG_MQTT_PASSWORD].c_str()) )
        {
            DEBUG_PRINTLN(F(" done"));
            break;
        }

        DEBUG_PRINT(F("."));
        delay(1000);
    }

    client.loop();

    DEBUG_PRINT(F("Start meassuring ..."));
    unsigned long start = millis();
    int value = -1;
    // wait for max 60 seconds => can happen only durinmg the initial warmup phase, but there is no way to detect this phase after a deepSleep
    while(millis() < start + 60 * 1000)
    {
        value = readCO2UART();
        // value must be lower then 10000, because max. detection range is between 400 and 5000. All other values are false values.
        if( value != -1 && value < 10000 ) 
        {
            DEBUG_PRINTLN(F(" done"));
            break;
        }

        DEBUG_PRINT(F("."));
        ledSignal(1000,1000);
    }

    DEBUG_PRINT(F("Publish values ..."));
    if( value != -1 )
    {
        client.publish(cfg[CFG_MQTT_CLIENT_TOPIC].c_str(), String(value).c_str());
        client.loop();
        client.disconnect();

        DEBUG_PRINTLN(F(" done"));
        /*if( !isSigngleMessurement )
        {
          int interval = cfg[CFG_SENSOR_INTERVAL].toInt();
          ESP.deepSleep(interval * 60e6, WAKE_RFCAL);
        }*/
    }
    else
    {
        DEBUG_PRINTLN(F(" error"));
    }

    client.loop();

    DEBUG_PRINT(F("Delay for "));
    DEBUG_PRINT(cfg[CFG_SENSOR_INTERVAL].toInt());
    DEBUG_PRINT(F(" minutes ..."));
    delay(cfg[CFG_SENSOR_INTERVAL].toInt() * 60000);
    DEBUG_PRINTLN(F(" done"));
}
