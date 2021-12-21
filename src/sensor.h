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
