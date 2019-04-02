// EEPROM.ino
//

#include "EEPROMDriver.h"


#define ROM_ADDR		(0x50)

EEPROMDriver rom(Wire);


void setup()
{
	digitalWrite(19, HIGH);
	pinMode(19, OUTPUT);	
	
	Serial.begin(115200);
	delay(100);
	
	Wire.begin();
	delay(100);
	
	uint8_t data = rom.readByte(ROM_ADDR, 0x00);
	Serial.print("Read at 0x00 = "); Serial.println(data);
	rom.writeByte(ROM_ADDR, 0x00, 0x47);
	Serial.println("Write 0x47 at 0x00 !");
	data = rom.readByte(ROM_ADDR, 0x00);
	Serial.print("Read at 0x00 = "); Serial.print(data);
}

void loop()
{
}