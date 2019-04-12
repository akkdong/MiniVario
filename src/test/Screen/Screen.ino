// Screen.ino
//

#include "VarioScreen.h"
#include "Keyboard.h"

//
PinSetting ePaperPins[] = 
{
	// CS
	{ SS, PIN_MODE_OUTPUT, PIN_ACTIVE_LOW, PIN_STATE_INACTIVE },
	// DC
	{ 33, PIN_MODE_OUTPUT, PIN_ACTIVE_LOW, PIN_STATE_INACTIVE },
	// RST
	{ 32, PIN_MODE_OUTPUT, PIN_ACTIVE_LOW, PIN_STATE_INACTIVE },
	// BUSY
	{ 39, PIN_MODE_INPUT, PIN_ACTIVE_LOW, PIN_STATE_INACTIVE },
};

PinSetting keybdPins[] =
{
	// KEY_UP
	{ 35, PIN_MODE_INPUT, PIN_ACTIVE_HIGH, PIN_STATE_ACTIVE },
	// KEY_DOWN
	{ 34, PIN_MODE_INPUT, PIN_ACTIVE_HIGH, PIN_STATE_ACTIVE },
	// KEY_SEL
	{  0, PIN_MODE_INPUT, PIN_ACTIVE_LOW, PIN_STATE_ACTIVE },
};

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

//
DeviceContext context;

VarioEPaper driver(ePaperPins);
VarioScreen display(driver, context);

Keyboard keybd(keybdPins, sizeof(keybdPins) / sizeof(keybdPins[0]));


class Varimeter : public Task
{
};


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
	display.begin();
//	display.setPage(page);
	
//	display.update();
//	display.refresh(false);
//	display.sleep();

	keybd.begin();
}


/*
Menu
	MenuItems --> show fixed value: number, float, string
	          --> show & change value

MenuHandler
	processKey
	
*/	


void loop()
{
	//
	keybd.update();
	
	#if 0
	int ch = keybd.getch();
	
	if (activeMenu)
	{
		int ret = activeMenu->processKey(ch);
		
		switch (ret)		
		{
		context.change_value();
		display.showMenu(newMenu, cascade or ...)
		}
	}
	else
	{
		KEY_UP/DOWN --> change Page
		KEY_SEL_LONG -> show top-menu
	}
	#endif
		
	#if 0
	switch (keybd.getch())
	{
	case KEY_UP :
		// move to previous page
		if (activeMenu)
		{
			activeMenu->processKey(KEY_UP);
		}
		else
		{
			page = getPrevPage();
			display.setPage(page);
		}
		break;
	case KEY_DOWN :
		if (activeMenu)
		{
			activeMenu->processKey(KEY_UP);
		// move to next page
		display.showNextPage();
		break;
	case KEY_SEL_LONG :
		// enter menu
		display.showMenu(topMenu);
		
		// temporary
		display.deepSleep();
		while(1);
	}
	#endif
}
