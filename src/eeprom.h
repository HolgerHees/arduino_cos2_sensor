unsigned int hexToDec(String hexString) 
{
  unsigned int decValue = 0;
  int nextInt;

  for(int i = 0; i < hexString.length(); i++) 
  {

    nextInt = int(hexString.charAt(i));
    if(nextInt >= 48 && nextInt <= 57) nextInt = map(nextInt, 48, 57, 0, 9);
    if(nextInt >= 65 && nextInt <= 70) nextInt = map(nextInt, 65, 70, 10, 15);
    if(nextInt >= 97 && nextInt <= 102) nextInt = map(nextInt, 97, 102, 10, 15);
    nextInt = constrain(nextInt, 0, 15);

    decValue = (decValue * 16) + nextInt;
  }

  return decValue;
}

int writeEeprom(int eepromIndex, String k)
{
  int counter = MAX_FIELD_LENGTH;
  int pos;
  int value;
  String hexNumber;
  String replacement = "f";

  k.replace("+", " ");

  while(counter--)
  {
    pos = k.indexOf('%');
    if(pos < 0) break;
    hexNumber = k.substring(pos + 1, pos + 3);
    value = hexToDec(hexNumber);
    replacement[0] = value;
    hexNumber = "%" + hexNumber;
    k.replace(hexNumber, replacement);
  }
  
  int i = 0;
  while (k[i])
  {
    EEPROM.write(eepromIndex, k[i]);
    eepromIndex++;
    i++;
  }
  EEPROM.write(eepromIndex, '\0');
  eepromIndex++;
  
  return eepromIndex;
}

void writeEepromConfig()
{
  EEPROM.begin(512);

  int eepromIndex = 0;
  for(int i = 0; i < 9; i++) 
  {
    eepromIndex = writeEeprom(eepromIndex, cfg[i]);
  }
  
  EEPROM.commit();
}

void initEepromConfig()
{
  EEPROM.begin(512);

  int eepromIndex = 0;
  for(int i = 0; i < 9; i++) 
  {
    char a;
    cfg[i] = "";
    do 
    {
      a = EEPROM.read(eepromIndex);
      if( a ) cfg[i] += String(a);
      eepromIndex++;
    }
    while(a);
  }
}

