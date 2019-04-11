// digitalIo.cpp
//

#include "digitalIo.h"


///////////////////////////////////////////////////////////////////////////////////////////
//

void initPins(PinSetting * pins, int count)
{
	for (int i = 0; i < count; i++)
	{
		if (pins[i].mode == PIN_MODE_OUTPUT)
		{
			setPinState(&pins[i], pins[i].init);
			pinMode(pins[i].no, OUTPUT);
		}
		else
		{
			pinMode(pins[i].no, INPUT);
		}
	}
}

int getPinState(PinSetting * pin)
{
	int value = digitalRead(pin->no);
	
	if (pin->active == PIN_ACTIVE_LOW)
		return value == LOW ? PIN_STATE_ACTIVE : PIN_STATE_INACTIVE;
	
	return value == LOW ? PIN_STATE_INACTIVE : PIN_STATE_ACTIVE;
}

void setPinState(PinSetting * pin, int8_t state)
{
	// ACTIVE_HIGH: ACTIVE(HIGH), INACTIVE(LOW)
	// ACTIVE_LOW:  ACTIVE(LOW), INACTIVE(HIGH)
	digitalWrite(pin->no, (pin->active == PIN_ACTIVE_LOW) ? ((state == PIN_STATE_INACTIVE) ? HIGH : LOW) : ((state == PIN_STATE_INACTIVE) ? LOW : HIGH));
}
