// Keyboard.cpp
//

#include "Keyboard.h"

#define DELAY_DEBOUNCE		 60
#define DELAY_LONG_KEY		640

enum _state_machine
{
	_RELEASED,
	_DEBOUNCE,
	_PRESSED,
	_IGNORE
};

///////////////////////////////////////////////////////////////////////////////////////////
// class Keyboard

Keyboard::Keyboard(PinSetting * pins, int count) : pinSettings(pins), pinCount(count)
{
}

int Keyboard::begin()
{
	//
	initPins(pinSettings, pinCount);
	
	//
	frontKey = rearKey = 0;
	inputMode = _RELEASED;
	
	return 0;
}

void Keyboard::update()
{
	// abtain key state
	uint8_t	keyState = getKeyState();
	
	// state-machine: RELEASED --> DEBOUNCE --> PRESSED ---------+-> RELEASED
	//                                 |           |             |
	//                                 +-----------+--> IGNORE --+	
	switch (inputMode)
	{
	case _RELEASED :
		if (keyState != 0) // any key was pressed
		{
			lastKeyState = keyState;
			lastTick = millis();
			inputMode = _DEBOUNCE;
		}
		break;
		
	case _DEBOUNCE :
		if (millis() - lastTick > DELAY_DEBOUNCE)
		{
			if (lastKeyState == keyState)
			{
				// key was unchanged until debounce period
				lastTick = millis();
				inputMode = _PRESSED;
			}
			else
			{
				// key was changed...
				inputMode = _IGNORE;
			}
		}
		else if (lastKeyState != keyState)
		{
			// key was changed...
			inputMode = _IGNORE;
		}
		break;
		
	case _PRESSED :
		if (keyState != 0)
		{
			if (keyState != lastKeyState)
			{
				// key was changed...
				inputMode = _IGNORE;
			}
			else if (millis() - lastTick > DELAY_LONG_KEY)
			{
				int key = findKey(lastKeyState);

				if (key >= 0)
					pushKey(key + pinCount);
				// else multiple key were pressed

				// wait until released : 
				inputMode = _IGNORE;
			}
		}
		else // if (keyState == 0)
		{
			int key = findKey(lastKeyState);
			
			if (key >= 0)
				pushKey(key);
			// else multiple key were pressed
			
			inputMode = _RELEASED;
		}
		break;
		
	case _IGNORE :
		if (keyState == 0)
		{
			// wait next key-input
			inputMode = _RELEASED;
		}
		break;
	}
}

// return key value which is being pressed
// 	 < 0 					: no key
// 	[0 ~ pinCount) 			: short key
// 	[pinCount, pinCount * 2): long key
int Keyboard::getch()
{
	if (rearKey == frontKey)
		return -1;
	
	int ch = keyPressed[rearKey];
	rearKey = (rearKey + 1) % (sizeof(keyPressed) / sizeof(keyPressed[0]));
	
	return ch;
}

uint8_t Keyboard::getKeyState()
{
	uint8_t	keyState = 0;
	
	for (int i = 0; i < pinCount; i++)
	{
		if (getPinState(&pinSettings[i]) == PIN_STATE_ACTIVE)
			keyState |= (1 << i);
	}

	return keyState;
}

void Keyboard::pushKey(uint8_t key)
{
	int nextKey = (frontKey + 1) % (sizeof(keyPressed) / sizeof(keyPressed[0]));
	
	if (nextKey == rearKey)
		return; // buffer is fulled
	
	keyPressed[frontKey] = key;
	frontKey = nextKey;
}

int Keyboard::findKey(uint8_t state)
{
	int key = -1;
	
	for (int i = 0; i < pinCount; i++)
	{
		if (state & (1 << i))
		{
			if (key != -1)
				return -1;
			
			key = i;
		}
	}
	
	return key;
}

#if 0

	// keyboard usage
	Keyboard keybd;


	int ch = keybd.getch();

	switch (ch)
	{
	case KEY_UP :
	case KEY_DOWN :	
	case KEY_SEL :	
	case KEY_UP_LONG :
	case KEY_DOWN_LONG :	
	case KEY_SEL_LONG :
		break;
	}
	
#endif
