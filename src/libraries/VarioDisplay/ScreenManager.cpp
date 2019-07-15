// ScreenMaanger.cpp
//

#include <Arduino.h>
#include "ScreenManager.h"

#define SID_DEFAULT_1		(0x8001)
#define SID_DEFAULT_2		(0x8002)
#define SID_CIRCLING		(0x8003)
#define SID_STATISTIC		(0x8004)

#define STATUS_TIME_HEIGHT	24
#define STATUS_TIME_WIDTH	40
#define TEXTBOX_S_HEIGHT	56
#define TEXTBOX_S_WIDTH		88

#define SCREEN_WIDTH		176
#define SCREEN_HEIGHT		264

#define W_W					(SCREEN_WIDTH/2)
#define W_H					(60)

#define MIN_W				(SCREEN_WIDTH/2)
#define MIN_H				(48)

#define TRACK_W 			(SCREEN_WIDTH) // 176
#define TRACK_H 			(SCREEN_HEIGHT - STATUS_TIME_HEIGHT - MIN_H - MIN_H)


#define NORMAL_STATUS		(WS_FONT_NORMAL_2 | WS_TA_CENTER | WS_TA_MIDDLE)
#define NORMAL_TEXT			(WS_FONT_BOLD_3 | WS_TA_RIGHT | WS_TA_BOTTOM)
#define NORMAL_BOX			(WS_BORDER_LEFT | WS_BORDER_TOP)


////////////////////////////////////////////////////////////////////////////////////////////////
//

static WidgetData _default_1[] =
{
	{
		Widget_VarioBar, NORMAL_BOX, WidgetContent_Vario_Lazy,
		0, STATUS_TIME_HEIGHT, 48, W_H * 2
	},
	{
		Widget_Compass, NORMAL_BOX, WidgetContent_Heading,
		48, STATUS_TIME_HEIGHT, 52, W_H
	},
	{
		Widget_TextBox, NORMAL_TEXT | NORMAL_BOX | WS_BORDER_RIGHT, WidgetContent_Speed_Ground,
		48 + 52, STATUS_TIME_HEIGHT, SCREEN_WIDTH - (48 + 52), W_H
	},
	{
		Widget_TextBox, NORMAL_TEXT | NORMAL_BOX | WS_BORDER_RIGHT, WidgetContent_Altitude_GPS,
		48, STATUS_TIME_HEIGHT + W_H, SCREEN_WIDTH - 48, W_H
	},
	{
		Widget_TextBox, NORMAL_TEXT | NORMAL_BOX, WidgetContent_Vario_Lazy,
		0, STATUS_TIME_HEIGHT + W_H * 2, W_W, W_H
	},
	{
		Widget_TextBox, NORMAL_TEXT | NORMAL_BOX | WS_BORDER_RIGHT, WidgetContent_Time_Flight,
		W_W, STATUS_TIME_HEIGHT + W_H * 2, W_W, W_H
	},
	{
		Widget_TextBox, NORMAL_TEXT | NORMAL_BOX | WS_BORDER_BOTTOM, WidgetContent_Glide_Ratio,
		0, STATUS_TIME_HEIGHT + W_H * 3, W_W, W_H
	},
	{
		Widget_TextBox, NORMAL_TEXT | NORMAL_BOX | WS_BORDER_RIGHT | WS_BORDER_BOTTOM, WidgetContent_Distance_Takeoff,
		W_W, STATUS_TIME_HEIGHT + W_H * 3, W_W, W_H
	},
	{
		Widget_Empty
	}
};

static WidgetData _default_2[] =
{
	{
		Widget_TextBox, NORMAL_TEXT | NORMAL_BOX, WidgetContent_Altitude_GPS,
		TEXTBOX_S_WIDTH * 0, TEXTBOX_S_HEIGHT * 0 + STATUS_TIME_HEIGHT, TEXTBOX_S_WIDTH, TEXTBOX_S_HEIGHT
	},
	{
		Widget_TextBox, NORMAL_TEXT | NORMAL_BOX | WS_BORDER_RIGHT, WidgetContent_Altitude_AGL,
		TEXTBOX_S_WIDTH * 1, TEXTBOX_S_HEIGHT * 0 + STATUS_TIME_HEIGHT, TEXTBOX_S_WIDTH, TEXTBOX_S_HEIGHT
	},
	{
		Widget_TextBox, NORMAL_TEXT | NORMAL_BOX, WidgetContent_Speed_Ground,
		TEXTBOX_S_WIDTH * 0, TEXTBOX_S_HEIGHT * 1 + STATUS_TIME_HEIGHT, TEXTBOX_S_WIDTH, TEXTBOX_S_HEIGHT
	},
	{
		Widget_TextBox, NORMAL_TEXT | NORMAL_BOX | WS_BORDER_RIGHT, WidgetContent_Heading,
		TEXTBOX_S_WIDTH * 1, TEXTBOX_S_HEIGHT * 1 + STATUS_TIME_HEIGHT, TEXTBOX_S_WIDTH, TEXTBOX_S_HEIGHT
	},
	{
		Widget_TextBox, NORMAL_TEXT | NORMAL_BOX, WidgetContent_Vario_Lazy,
		TEXTBOX_S_WIDTH * 0, TEXTBOX_S_HEIGHT * 2 + STATUS_TIME_HEIGHT, TEXTBOX_S_WIDTH, TEXTBOX_S_HEIGHT
	},
	{
		Widget_TextBox, NORMAL_TEXT | NORMAL_BOX | WS_BORDER_RIGHT, WidgetContent_Time_Flight,
		TEXTBOX_S_WIDTH * 1, TEXTBOX_S_HEIGHT * 2 + STATUS_TIME_HEIGHT, TEXTBOX_S_WIDTH, TEXTBOX_S_HEIGHT
	},
	{
		Widget_Compass, NORMAL_BOX | WS_BORDER_BOTTOM, WidgetContent_Heading,
		TEXTBOX_S_WIDTH * 0, TEXTBOX_S_HEIGHT * 3 + STATUS_TIME_HEIGHT, SCREEN_HEIGHT - (TEXTBOX_S_HEIGHT * 3 + STATUS_TIME_HEIGHT), SCREEN_HEIGHT - (TEXTBOX_S_HEIGHT * 3 + STATUS_TIME_HEIGHT)
	},
	{
		Widget_VarioHistory, NORMAL_BOX | WS_BORDER_RIGHT | WS_BORDER_BOTTOM, WidgetContent_Vario_History,
		SCREEN_HEIGHT - (TEXTBOX_S_HEIGHT * 3 + STATUS_TIME_HEIGHT), TEXTBOX_S_HEIGHT * 3 + STATUS_TIME_HEIGHT, SCREEN_WIDTH - (SCREEN_HEIGHT - (TEXTBOX_S_HEIGHT * 3 + STATUS_TIME_HEIGHT)), SCREEN_HEIGHT - (TEXTBOX_S_HEIGHT * 3 + STATUS_TIME_HEIGHT)
	},
	{
		Widget_Empty
	}
};

static WidgetData _default_Circling[] =
{
	{
		Widget_TrackHistory, NORMAL_BOX | WS_BORDER_RIGHT, WidgetContent_Track_History,
		0, STATUS_TIME_HEIGHT, TRACK_W, TRACK_H
	},
	{
		Widget_Compass, 0, WidgetContent_Heading,
		0, STATUS_TIME_HEIGHT, MIN_H, MIN_H
	},
	{
		Widget_TextBox, NORMAL_TEXT | NORMAL_BOX, WidgetContent_Altitude_GPS,
		0, STATUS_TIME_HEIGHT + TRACK_H, TEXTBOX_S_WIDTH, MIN_H,
	},
	{
		Widget_TextBox, NORMAL_TEXT | NORMAL_BOX | WS_BORDER_RIGHT, WidgetContent_Speed_Ground,
		TEXTBOX_S_WIDTH, STATUS_TIME_HEIGHT + TRACK_H, TEXTBOX_S_WIDTH, MIN_H,
	},
	{
		Widget_TextBox, NORMAL_TEXT | NORMAL_BOX | WS_BORDER_BOTTOM, WidgetContent_Thermaling_Gain,
		0, STATUS_TIME_HEIGHT + TRACK_H + MIN_H, TEXTBOX_S_WIDTH, MIN_H
	},
	{
		Widget_TextBox, NORMAL_TEXT | NORMAL_BOX | WS_BORDER_RIGHT | WS_BORDER_BOTTOM, WidgetContent_Vario_Active,
		TEXTBOX_S_WIDTH, STATUS_TIME_HEIGHT + TRACK_H + MIN_H, TEXTBOX_S_WIDTH, MIN_H
	},
	{
		Widget_Empty
	}
};

static WidgetData _default_FlightStatistic[] = 
{
	{
		Widget_TextBox, NORMAL_TEXT | NORMAL_BOX, WidgetContent_Time_Takeoff,
		MIN_W * 0, MIN_H * 0 + STATUS_TIME_HEIGHT, MIN_W, MIN_H
	},
	{
		Widget_TextBox, NORMAL_TEXT | NORMAL_BOX | WS_BORDER_RIGHT, WidgetContent_Time_Flight,
		MIN_W * 1, MIN_H * 0 + STATUS_TIME_HEIGHT, MIN_W, MIN_H
	},
	{
		Widget_TextBox, NORMAL_TEXT | NORMAL_BOX, WidgetContent_Distance_Takeoff,
		MIN_W * 0, MIN_H * 1 + STATUS_TIME_HEIGHT, MIN_W, MIN_H
	},
	{
		Widget_TextBox, NORMAL_TEXT | NORMAL_BOX | WS_BORDER_RIGHT, WidgetContent_Distance_Flight,
		MIN_W * 1, MIN_H * 1 + STATUS_TIME_HEIGHT, MIN_W, MIN_H
	},
	{
		Widget_TextBox, NORMAL_TEXT | NORMAL_BOX, WidgetContent_Altitude_Max,
		MIN_W * 0, MIN_H * 2 + STATUS_TIME_HEIGHT, MIN_W, MIN_H
	},
	{
		Widget_TextBox, NORMAL_TEXT | NORMAL_BOX | WS_BORDER_RIGHT, WidgetContent_Altitude_Min,
		MIN_W * 1, MIN_H * 2 + STATUS_TIME_HEIGHT, MIN_W, MIN_H
	},
	{
		Widget_TextBox, NORMAL_TEXT | NORMAL_BOX, WidgetContent_ClimbRate_Max,
		MIN_W * 0, MIN_H * 3 + STATUS_TIME_HEIGHT, MIN_W, MIN_H
	},
	{
		Widget_TextBox, NORMAL_TEXT | NORMAL_BOX | WS_BORDER_RIGHT, WidgetContent_SinkRate_Max,
		MIN_W * 1, MIN_H * 3 + STATUS_TIME_HEIGHT, MIN_W, MIN_H
	},
	{
		Widget_TextBox, NORMAL_TEXT | NORMAL_BOX | WS_BORDER_BOTTOM, WidgetContent_Total_Thermaling,
		MIN_W * 0, MIN_H * 4 + STATUS_TIME_HEIGHT, MIN_W, MIN_H
	},
	{
		Widget_TextBox, NORMAL_TEXT | NORMAL_BOX | WS_BORDER_RIGHT | WS_BORDER_BOTTOM, WidgetContent_Max_ThermalingGain,
		MIN_W * 1, MIN_H * 4 + STATUS_TIME_HEIGHT, MIN_W, MIN_H
	},
	{
		Widget_Empty
	}
};


////////////////////////////////////////////////////////////////////////////////////////////////
// class ScreenManager

ScreenManager::ScreenManager() : maxScreen(0), curScreen(0), actScreen(0)
{

}

void ScreenManager::loadScreens()
{
	// reset
	maxScreen = actScreen = curScreen = 0;

	// load stock screens
	loadScreen(&screens[maxScreen++], SID_DEFAULT_1, _default_1);
	loadScreen(&screens[maxScreen++], SID_DEFAULT_2, _default_2);
	loadScreen(&screens[maxScreen++], SID_CIRCLING, _default_Circling);
	loadScreen(&screens[maxScreen++], SID_STATISTIC, _default_FlightStatistic);

	// load user defined screen from storage
	// ...
}

void ScreenManager::showScreen(VarioDisplay & display, int screen)
{
	if (0 <= screen && screen < maxScreen)
		display.attachScreen(&screens[screen]);
}

void ScreenManager::showActiveScreen(VarioDisplay & display)
{
	// change current to active & attach screen to display
	showScreen(display, (curScreen = actScreen));
}

void ScreenManager::showCirclingScreen(VarioDisplay & display)
{
	// find circling screen
	int scrn = findScreen(SID_CIRCLING);
	if (scrn < 0)
		return;
	
	// change current to circling screen
	showScreen(display, (curScreen = scrn));
}

void ScreenManager::showStatisticScreen(VarioDisplay & display)
{
	// find statistic screen
	int scrn = findScreen(SID_STATISTIC);
	if (scrn < 0)
		return;

	// change current to statistic screen
	showScreen(display, (curScreen = scrn));
}

void ScreenManager::showNextActiveScreen(VarioDisplay & display)
{
	// change acctive screen to next screen
	actScreen = (actScreen + 1) % maxScreen;

	// attach screen to display
	showScreen(display, (curScreen = actScreen));
}

void ScreenManager::showPrevActiveScreen(VarioDisplay & display)
{
	// change acctive screen to previous screen
	actScreen = (actScreen + (maxScreen - 1)) % maxScreen;

	// attach screen to display
	showScreen(display, (curScreen = actScreen));
}

void ScreenManager::loadScreen(VarioScreen * screen, uint32_t id, WidgetData * widgets)
{
	for (int i = 0; widgets[i].style != Widget_Empty; i++)
		screen->addWidget(&widgets[i]);

	screen->setID(id);
}

int ScreenManager::findScreen(uint32_t id)
{
	for (int i = 0; i < maxScreen; i++)
	{
		if (screens[i].getID() == id)
			return i;
	}

	return -1;
}