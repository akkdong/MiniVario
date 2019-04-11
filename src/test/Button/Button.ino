// Button.ino
//

#include "digitalIo.h"
#include "Keyboard.h"


PinSetting outputPins[] =
{
	// PWR_PERIPH
	{ 19, PIN_MODE_OUTPUT, PIN_ACTIVE_HIGH, PIN_STATE_INACTIVE },
	// PWR_SOUND
	{ 27, PIN_MODE_OUTPUT, PIN_ACTIVE_HIGH, PIN_STATE_INACTIVE },
};

PinSetting inputPins[] =
{
	// KEY_UP
	{ 35, PIN_MODE_INPUT, PIN_ACTIVE_HIGH, PIN_STATE_ACTIVE },
	// KEY_DOWN
	{ 34, PIN_MODE_INPUT, PIN_ACTIVE_HIGH, PIN_STATE_ACTIVE },
	// KEY_SEL
	{  0, PIN_MODE_INPUT, PIN_ACTIVE_LOW, PIN_STATE_ACTIVE },
};


Keyboard keybd(inputPins, sizeof(inputPins) / sizeof(inputPins[0]));
uint32_t tickCheck;

void setup()
{
	//
	initPins(outputPins, sizeof(outputPins) / sizeof(outputPins[0]));
	delay(100);
	
	//
	Serial.begin(115200);
	Serial.println("Button Test...");
	delay(100);
	
	//
	keybd.begin();
	tickCheck = millis();
}

void loop()
{
	keybd.update();
	
	// check key input every 5s
	if (millis() - tickCheck > 5000)
	{
		int ch;
		
		Serial.println("Check keyboard.");
		while ((ch = keybd.getch()) >= 0)
		{
			Serial.print("Key input: "); Serial.println(ch);
		}
		
		tickCheck = millis();
	}
	
	#if 0
	if (getPinState(&ioPins[2]) == PIN_STATE_ACTIVE) {
		Serial.print("Key UP: "); Serial.println(digitalRead(ioPins[2].no));
	}
	if (getPinState(&ioPins[3]) == PIN_STATE_ACTIVE) {
		Serial.print("Key DN: "); Serial.println(digitalRead(ioPins[3].no));
	}
	if (getPinState(&ioPins[4]) == PIN_STATE_ACTIVE) {
		Serial.print("Key RE: "); Serial.println(digitalRead(ioPins[4].no));
	}
	#endif
	
	delay(10);
}
