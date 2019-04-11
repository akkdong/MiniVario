// Keyboard.h
//

#ifndef __KEYBOARD_H__
#define __KEYBOARD_H__

#include <Arduino.h>
#include "digitalIo.h"

#define MAX_KEYS		(8)

enum _InputKey
{
	KEY_UP,
	KEY_DOWN,
	KEY_SEL,
	KEY_UP_LONG,
	KEY_DOWN_LONG,
	KEY_SEL_LONG,
	KEY_Count
};



///////////////////////////////////////////////////////////////////////////////////////////
// class Keyboard

class Keyboard
{
public:
	Keyboard(PinSetting * pins, int count);
	
public:
	int					begin();
	
	void				update(); // it should be called as often as possible
	
	int					getch(); // return key value which is being pressed
	uint8_t				getKeyState(); // bitwised current key state
	
private:
	void				pushKey(uint8_t key);
	int					findKey(uint8_t state);
	
private:
	PinSetting *		pinSettings;
	int					pinCount;
	
	uint32_t			lastTick;
	uint8_t				lastState;
	uint8_t				mode;
	
	uint8_t				keyPressed[8];
	int					rearKey, frontKey;
};


#endif // __KEYBOARD_H__
