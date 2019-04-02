#include "BluetoothSerial.h"
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"

BluetoothSerial SerialBT;
 
void setup() 
{
//	WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); //disable brownout detector

	//
	digitalWrite(19, HIGH);
	pinMode(19, OUTPUT);	
	delay(10);

	//
	Serial.begin(115200);
	
	Serial2.begin(9600);

	if(!SerialBT.begin("Abrakatabra"))
	{
		Serial.println("An error occurred initializing Bluetooth");
		while(1);
	}
}
 
void loop() 
{
	while(Serial.available()) 
		SerialBT.write(Serial.read());

	while(SerialBT.available()) 
		Serial2.write(SerialBT.read());

	while(Serial2.available()) 
		SerialBT.write(Serial2.read());

	delay(1);
}