// Keyboard.h
//

#ifndef __KEYBOARD_H__
#define __KEYBOARD_H__

#include <Arduino.h>
#include "digitalIo.h"

#define MAX_KEYS		(8)
#define MAX_KEYINPUT	(8)


///////////////////////////////////////////////////////////////////////////////////////////
// class Keyboard

class Keyboard
{
public:
	Keyboard(PinSetting * pins, int count);

public:
	int					begin();
	
	void				update(); // this should be called as often as possible
	
	int					getch(); // returns key value which is being pressed
	uint8_t				getKeyState(); // bitwised current key state
	
private:
	void				pushKey(uint8_t key);
	int					findKey(uint8_t state);
	
private:
	PinSetting *		pinSettings;
	int					pinCount;
	
	uint32_t			lastTick;
	uint8_t				lastKeyState;
	uint8_t				inputMode;
	
	uint8_t				keyPressed[MAX_KEYINPUT];
	int					rearKey, frontKey;
};


#endif // __KEYBOARD_H__
