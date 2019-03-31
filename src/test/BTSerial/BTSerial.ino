#include "BluetoothSerial.h"
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"

BluetoothSerial SerialBT;
 
void setup() 
{
//	WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); //disable brownout detector


	Serial.begin(115200);

	if(!SerialBT.begin("Abrakatabra"))
	{
		
		Serial.println("An error occurred initializing Bluetooth");
	}
}
 
void loop() 
{
	while(SerialBT.available()) 
	{
		Serial.write(SerialBT.read());
	}

	while(Serial.available()) 
	{
		SerialBT.write(Serial.read());
	}

	delay(50);
}