// Screen.ino
//

#include "VarioScreen.h"

//
PinSetting ePaperPins[] = 
{
	// CS
	{ SS, PIN_MODE_OUTPUT, PIN_ACTIVE_LOW },
	// DC
	{ 33, PIN_MODE_OUTPUT, PIN_ACTIVE_LOW },
	// RST
	{ 32, PIN_MODE_OUTPUT, PIN_ACTIVE_LOW },
	// BUSY
	{ 39, PIN_MODE_INPUT, PIN_ACTIVE_LOW },
};


DeviceContext context;

EPaper_Waveshare270 driver(ePaperPins);
VarioScreen display(driver, context);


void setup()
{
	//
	digitalWrite(19, HIGH);
	pinMode(19, OUTPUT);
	delay(10);
	
	//
	Serial.begin(115200);
	Serial.println("Screen Test!!");
	
	//
	display.init();
	
//	display.update();
	display.refresh(false);
	display.sleep();
}

void loop()
{
	// nop...
}
