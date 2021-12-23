byte getCheckSum(byte* packet) 
{
  byte i;
  byte checksum = 0;
  for( i = 1; i < 8; i++ ) 
  {
    checksum += packet[i];
  }
  checksum = 0xff - checksum;
  checksum += 1;
  return checksum;
}

int readCO2UART()
{
  byte cmd[9] = {0xFF, 0x01, 0x86, 0x00, 0x00, 0x00, 0x00, 0x00, 0x79};
  byte result[9];
  sensorSerial.write(cmd, 9);

  // wait until data are available
  int waited = 0;
  while( sensorSerial.available() == 0) 
  {
    DEBUG_PRINT(".");
    delay(100);
    if(waited++ > 5) 
    {
      DEBUG_PRINTLN(F(" no response after 10 seconds"));
      sensorSerial.flush();
      return -1;
    }
  }
  
  // Validate that serial stream is in sync.
  boolean skip = false;
  while( sensorSerial.available() > 0 && (byte)sensorSerial.peek() != 0xFF) 
  {
    if (!skip) {
      DEBUG_PRINT(F("skipping unexpected readings:"));
      skip = true;
    }
    //DEBUG_PRINT(" ");
    //DEBUG_PRINT(sensorSerial.peek(), HEX);
    sensorSerial.read();
  }
  
  // validate available amount of bytes
  if(sensorSerial.available() > 0) 
  {
    int count = sensorSerial.readBytes(result, 9);
    if(count < 9) 
    {
      sensorSerial.flush();
      return -1;
    }
  }
  else 
  {
    sensorSerial.flush();
    return -1;
  }

  // validate checksum
  byte check = getCheckSum(result);
  if(result[8] != check) return -1;
    
  //#if(result[0] != 0xFF || result[1] != 0x86) return -1;
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

  byte result;
  while (Serial.available()) result = Serial.read();
}

void calibrateZero()
{
  byte cmd[9] = {0xFF, 0x01, 0x87, 0x00, 0x00, 0x00, 0x00, 0x00, 0x78};
  sensorSerial.write(cmd,9);

  byte result;
  while (Serial.available()) result = Serial.read();
}
