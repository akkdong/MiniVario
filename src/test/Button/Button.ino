// Button.ino
//

#include "digitalIo.h"

PinSetting ioPins[] =
{
	// PWR_PERIPH
	{ 19, PIN_MODE_OUTPUT, PIN_ACTIVE_HIGH, PIN_STATE_INACTIVE },
	// PWR_SOUND
	{ 27, PIN_MODE_OUTPUT, PIN_ACTIVE_HIGH, PIN_STATE_INACTIVE },
	// KEY_UP
	{ 35, PIN_MODE_INPUT, PIN_ACTIVE_HIGH, PIN_STATE_ACTIVE },
	// KEY_DOWN
	{ 34, PIN_MODE_INPUT, PIN_ACTIVE_HIGH, PIN_STATE_ACTIVE },
	// KEY_SEL
	{  0, PIN_MODE_INPUT, PIN_ACTIVE_LOW, PIN_STATE_ACTIVE },
};

void setup()
{
	//
	initPins(ioPins, sizeof(ioPins) / sizeof(ioPins[0]));
	delay(100);
	
	Serial.begin(115200);
	Serial.println("Button Test...");
	delay(100);
}

void loop()
{
	if (getPinState(&ioPins[2]) == PIN_STATE_ACTIVE) {
		Serial.print("Key UP: "); Serial.println(digitalRead(ioPins[2].no));
	}
	if (getPinState(&ioPins[3]) == PIN_STATE_ACTIVE) {
		Serial.print("Key DN: "); Serial.println(digitalRead(ioPins[3].no));
	}
	if (getPinState(&ioPins[4]) == PIN_STATE_ACTIVE) {
		Serial.print("Key RE: "); Serial.println(digitalRead(ioPins[4].no));
	}
	
	delay(50);
}
