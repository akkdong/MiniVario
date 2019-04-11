// digitalIo.h
//

#ifndef __DIGITALIO_H__
#define __DIGITALIO_H__

#include <Arduino.h>


///////////////////////////////////////////////////////////////////////////////////////////
//

#define PIN_MODE_INPUT			(0)
#define PIN_MODE_OUTPUT			(1)

#define PIN_ACTIVE_LOW			(0)
#define PIN_ACTIVE_HIGH			(1)

#define PIN_STATE_INACTIVE		(0)
#define PIN_STATE_ACTIVE		(1)


struct PinSetting
{
	uint8_t	no;
	uint8_t	mode;
	uint8_t	active;
	uint8_t	init;		// initial state of output pin
};


///////////////////////////////////////////////////////////////////////////////////////////
//

void 	initPins(PinSetting * pins, int count);

int		getPinState(PinSetting * pin);
void	setPinState(PinSetting * pin, int8_t state);


#endif // __DIGITALIO_H__
