#include <ESP8266HTTPUpdateServer.h>
String successMsg = "";
ESP8266HTTPUpdateServer httpUpdater;

String generateDummyPassword(String password)
{
    String dummyPassword = password;
    for (int i = 1; i < (dummyPassword.length() - 1); i++) dummyPassword[i] = '*';
    return dummyPassword;
}

void handleSetupPage()
{
  successMsg = "";
  server.sendHeader("Cache-Control", "no-cache");
  server.send(200, "text/html", SETUP_PAGE);
}

void handleJSDataFile()
{
  int i;
  String temp = "var data = [";
  String TempDaten;
  if (cfg[CFG_WIFI_SSID][0] < 128)
  {
    temp += "\"" + cfg[CFG_WIFI_SSID] + "\", ";
    String dummyPassword = generateDummyPassword(cfg[CFG_WIFI_PASSWORD]);
    temp += "\"" + dummyPassword + "\"";
    for (i = 2; i < 9; i++)
    {
      temp += ", \"" + cfg[i] + "\"";
    }
  }
  temp += " ]\r\n";
  temp += "var successMsg=\"" + successMsg + "\"\r\n";
  server.sendHeader("Cache-Control", "no-cache");
  server.send(200, "text/plain", temp);
}

void handleSubmitPage()
{
  cfg[CFG_WIFI_SSID] = server.arg(0);
  String dummyPassword = generateDummyPassword(cfg[CFG_WIFI_PASSWORD]);
  String temp1 = server.arg(1);
  if(temp1 != dummyPassword) cfg[CFG_WIFI_PASSWORD] = temp1;
  for(int i = 2; i < 9; i++)
  {
    cfg[i] = server.arg(i);
  }
  writeEepromConfig();

  successMsg = "Gespeichert!";
  server.sendHeader("Cache-Control", "no-cache");
  server.send(200, "text/html", SETUP_PAGE);
}

void handleNotFound()
{
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += ( server.method() == HTTP_GET ) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for ( uint8_t i = 0; i < server.args(); i++ ) {
    message += " " + server.argName ( i ) + ": " + server.arg ( i ) + "\n";
  }
  server.send ( 404, "text/plain", message );
}

void mainSetup()
{
  //WiFi.mode(WIFI_AP);
  WiFi.softAP("CO2 Sensor Setup", "connect1234");
  //delay(500);
  //WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
  //delay(500);
  
  server.onNotFound ( handleNotFound );
  server.on("/", handleSetupPage);
  server.on("/data.js", handleJSDataFile);
  server.on("/submit", handleSubmitPage);
  httpUpdater.setup(&server);
  server.begin();

  DEBUG_PRINT("Starte access point \"CO2 Sensor Setup\" ");
  DEBUG_PRINTLN(WiFi.softAPIP());

  while (true)
  {
    server.handleClient();
    delay(500);
  }
}
