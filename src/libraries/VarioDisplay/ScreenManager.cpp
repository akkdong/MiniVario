// ScreenMaanger.cpp
//

#include <Arduino.h>
#include "ScreenManager.h"


#define STATUS_TIME_HEIGHT	24
#define STATUS_TIME_WIDTH	40
#define TEXTBOX_S_HEIGHT	56
#define TEXTBOX_S_WIDTH		88

#define SCREEN_WIDTH		176
#define SCREEN_HEIGHT		264

#define W_W		(SCREEN_WIDTH/2)
#define W_H		(60)

#define MIN_H	48

#define TRACK_W SCREEN_WIDTH // 176
#define TRACK_H (SCREEN_HEIGHT - STATUS_TIME_HEIGHT - MIN_H - MIN_H)


#define NORMAL_STATUS		(WS_FONT_NORMAL_2 | WS_TA_CENTER | WS_TA_MIDDLE)
#define NORMAL_TEXT			(WS_FONT_BOLD_3 | WS_TA_RIGHT | WS_TA_BOTTOM)
#define NORMAL_BOX			(WS_BORDER_LEFT | WS_BORDER_TOP)


////////////////////////////////////////////////////////////////////////////////////////////////
//

WidgetData _default_1[] =
{
	{
		Widget_TextBox, NORMAL_TEXT | NORMAL_BOX, WidgetContent_Altitude_GPS,
		TEXTBOX_S_WIDTH * 0, TEXTBOX_S_HEIGHT * 0 + STATUS_TIME_HEIGHT, TEXTBOX_S_WIDTH, TEXTBOX_S_HEIGHT
	},
	{
		Widget_TextBox, NORMAL_TEXT | NORMAL_BOX | WS_BORDER_RIGHT, WidgetContent_Speed_Ground,
		TEXTBOX_S_WIDTH * 1, TEXTBOX_S_HEIGHT * 0 + STATUS_TIME_HEIGHT, TEXTBOX_S_WIDTH, TEXTBOX_S_HEIGHT
	},
	{
		Widget_TextBox, NORMAL_TEXT | NORMAL_BOX, WidgetContent_Heading,
		TEXTBOX_S_WIDTH * 0, TEXTBOX_S_HEIGHT * 1 + STATUS_TIME_HEIGHT, TEXTBOX_S_WIDTH, TEXTBOX_S_HEIGHT
	},
	{
		Widget_TextBox, NORMAL_TEXT | NORMAL_BOX | WS_BORDER_RIGHT, WidgetContent_Vario_Lazy,
		TEXTBOX_S_WIDTH * 1, TEXTBOX_S_HEIGHT * 1 + STATUS_TIME_HEIGHT, TEXTBOX_S_WIDTH, TEXTBOX_S_HEIGHT
	},
	{
		Widget_TextBox, NORMAL_TEXT | NORMAL_BOX, WidgetContent_Altitude_Baro,
		TEXTBOX_S_WIDTH * 0, TEXTBOX_S_HEIGHT * 2 + STATUS_TIME_HEIGHT, TEXTBOX_S_WIDTH, TEXTBOX_S_HEIGHT
	},
	{
		Widget_TextBox, NORMAL_TEXT | NORMAL_BOX | WS_BORDER_RIGHT, WidgetContent_Temperature,
		TEXTBOX_S_WIDTH * 1, TEXTBOX_S_HEIGHT * 2 + STATUS_TIME_HEIGHT, TEXTBOX_S_WIDTH, TEXTBOX_S_HEIGHT
	},
	{
		Widget_Compass, NORMAL_BOX | WS_BORDER_BOTTOM, WidgetContent_Heading,
		TEXTBOX_S_WIDTH * 0, TEXTBOX_S_HEIGHT * 3 + STATUS_TIME_HEIGHT, SCREEN_HEIGHT - TEXTBOX_S_HEIGHT * 3 + STATUS_TIME_HEIGHT, SCREEN_HEIGHT - TEXTBOX_S_HEIGHT * 3 + STATUS_TIME_HEIGHT
	},
	{
		Widget_VarioHistory, NORMAL_BOX | WS_BORDER_RIGHT | WS_BORDER_BOTTOM, WidgetContent_Vario_History,
		(SCREEN_HEIGHT - TEXTBOX_S_HEIGHT * 3 + STATUS_TIME_HEIGHT), TEXTBOX_S_HEIGHT * 3 + STATUS_TIME_HEIGHT, SCREEN_WIDTH - (SCREEN_HEIGHT - TEXTBOX_S_HEIGHT * 3 + STATUS_TIME_HEIGHT), SCREEN_HEIGHT - TEXTBOX_S_HEIGHT * 3 + STATUS_TIME_HEIGHT
	},
	{
		Widget_Empty
	}
};

WidgetData _default_2[] =
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
		Widget_TextBox, NORMAL_TEXT | NORMAL_BOX | WS_BORDER_BOTTOM, WidgetContent_Altitude_Ref1,
		0, STATUS_TIME_HEIGHT + W_H * 3, W_W, W_H
	},
	{
		Widget_TextBox, NORMAL_TEXT | NORMAL_BOX | WS_BORDER_RIGHT | WS_BORDER_BOTTOM, WidgetContent_Temperature,
		W_W, STATUS_TIME_HEIGHT + W_H * 3, W_W, W_H
	},
	{
		Widget_Empty
	}
};

WidgetData _default_Circling[] =
{
	{
		Widget_TrackHistory, NORMAL_BOX | WS_BORDER_RIGHT, WidgetContent_Track_History,
		0, STATUS_TIME_HEIGHT, TRACK_W, TRACK_H
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
		Widget_TextBox, NORMAL_TEXT | NORMAL_BOX | WS_BORDER_BOTTOM, WidgetContent_Altitude_AGL,
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


////////////////////////////////////////////////////////////////////////////////////////////////
// class ScreenManager

ScreenManager::ScreenManager() : maxScreen(0), curScreen(0)
{

}

void ScreenManager::loadScreens()
{
	// reset
	maxScreen = curScreen = 0;

	// load stock screens
	loadScreen(&screens[maxScreen++], 0x8001, _default_1);
	loadScreen(&screens[maxScreen++], 0x8001, _default_2);
	loadScreen(&screens[maxScreen++], 0x8002, _default_Circling);

	// load user defined screen from storage
	// ...
}

VarioScreen * ScreenManager::getNextScreen() 
{
	curScreen = (curScreen + 1) % maxScreen;

	return &screens[curScreen];
}

VarioScreen * ScreenManager::getPrevScreen() 
{ 
	curScreen = ((curScreen == 0) ? maxScreen - 1 : (curScreen - 1));

	return &screens[curScreen];
}

VarioScreen * ScreenManager::getActiveScreen()
{
	if (curScreen < maxScreen)
		return &screens[curScreen];

	return NULL;
}

void ScreenManager::loadScreen(VarioScreen * screen, uint32_t id, WidgetData * widgets)
{
	for (int i = 0; widgets[i].style != Widget_Empty; i++)
		screen->addWidget(&widgets[i]);

	screen->setID(id);
}

#if 0
void loadPages(VarioScreen * pages)
{
	// config pages <-- load page information from somewhere....
	//
	// below is just a test
	int widget = 0;
	
	// Screen: 176 x 264
	//
	// StatusBar: 176 x 24
	// Text-Box small: 88 x 52
	// Text-Box normal: 176 x 64
	//
	// 240 / 4 = 60, 240 / 5 = 48
	//
	int x = 0, y = 0;
	
#define STATUS_TIME_HEIGHT	24
#define STATUS_TIME_WIDTH	40
#define TEXTBOX_S_HEIGHT	56
#define TEXTBOX_S_WIDTH		88

#define SCREEN_WIDTH		176
#define SCREEN_HEIGHT		264


#define NORMAL_STATUS		(WS_FONT_NORMAL_2 | WS_TA_CENTER | WS_TA_MIDDLE)
#define NORMAL_TEXT			(WS_FONT_BOLD_3 | WS_TA_RIGHT | WS_TA_BOTTOM)
#define NORMAL_BOX			(WS_BORDER_LEFT | WS_BORDER_TOP)

#if 0	
	pages[0].widget[widget].setStyle(Widget_StatusBar, 0, WidgetContent_Status_Bar);
	pages[0].widget[widget].setPosition(x, y, SCREEN_WIDTH, STATUS_TIME_HEIGHT);
	widget++;
	
	pages[0].widget[widget].setStyle(Widget_SimpleText, NORMAL_STATUS, WidgetContent_Battery);
	pages[0].widget[widget].setPosition(x + SCREEN_WIDTH - STATUS_TIME_WIDTH - STATUS_TIME_WIDTH, y, STATUS_TIME_WIDTH, STATUS_TIME_HEIGHT);
	widget++;
	
	pages[0].widget[widget].setStyle(Widget_SimpleText, NORMAL_STATUS, WidgetContent_Time_Current);
	pages[0].widget[widget].setPosition(x + SCREEN_WIDTH - STATUS_TIME_WIDTH, y, STATUS_TIME_WIDTH, STATUS_TIME_HEIGHT);
	widget++;
#endif	
	y += STATUS_TIME_HEIGHT;
	
	pages[0].getWidget(widget)->setStyle(Widget_TextBox, NORMAL_TEXT | NORMAL_BOX, WidgetContent_Altitude_GPS);
	pages[0].getWidget(widget)->setPosition(x, y, TEXTBOX_S_WIDTH, TEXTBOX_S_HEIGHT);
	widget++;
	x += TEXTBOX_S_WIDTH;

	pages[0].getWidget(widget)->setStyle(Widget_TextBox, NORMAL_TEXT | NORMAL_BOX | WS_BORDER_RIGHT, WidgetContent_Speed_Ground);
	pages[0].getWidget(widget)->setPosition(x, y, TEXTBOX_S_WIDTH, TEXTBOX_S_HEIGHT);
	widget++;
	x = 0;
	y += TEXTBOX_S_HEIGHT;

	pages[0].getWidget(widget)->setStyle(Widget_TextBox, NORMAL_TEXT | NORMAL_BOX, WidgetContent_Heading);
	pages[0].getWidget(widget)->setPosition(x, y, TEXTBOX_S_WIDTH, TEXTBOX_S_HEIGHT);
	widget++;
	x += TEXTBOX_S_WIDTH;
 
	pages[0].getWidget(widget)->setStyle(Widget_TextBox, NORMAL_TEXT | NORMAL_BOX | WS_BORDER_RIGHT, WidgetContent_Vario_Lazy);
	pages[0].getWidget(widget)->setPosition(x, y, TEXTBOX_S_WIDTH, TEXTBOX_S_HEIGHT);
	widget++;
	x = 0;
	y += TEXTBOX_S_HEIGHT;
	
//	pages[0].getWidget(widget)->setStyle(Widget_TextBox, NORMAL_TEXT | NORMAL_BOX | WS_BORDER_BOTTOM, WidgetContent_Altitude_Baro);
	pages[0].getWidget(widget)->setStyle(Widget_TextBox, NORMAL_TEXT | NORMAL_BOX, WidgetContent_Altitude_Baro);
	pages[0].getWidget(widget)->setPosition(x, y, TEXTBOX_S_WIDTH, TEXTBOX_S_HEIGHT);
	widget++;
	x += TEXTBOX_S_WIDTH;

//	pages[0].getWidget(widget)->setStyle(Widget_TextBox, NORMAL_TEXT | NORMAL_BOX | WS_BORDER_RIGHT | WS_BORDER_BOTTOM, WidgetContent_Temperature);
	pages[0].getWidget(widget)->setStyle(Widget_TextBox, NORMAL_TEXT | NORMAL_BOX | WS_BORDER_RIGHT, WidgetContent_Temperature);
	pages[0].getWidget(widget)->setPosition(x, y, TEXTBOX_S_WIDTH, TEXTBOX_S_HEIGHT);
	widget++;
	x = 0;
	y += TEXTBOX_S_HEIGHT;

	pages[0].getWidget(widget)->setStyle(Widget_Compass, NORMAL_BOX | WS_BORDER_BOTTOM, WidgetContent_Heading);
	pages[0].getWidget(widget)->setPosition(x, y, SCREEN_HEIGHT - y, SCREEN_HEIGHT - y);
	widget++;
	x += SCREEN_HEIGHT - y;

	pages[0].getWidget(widget)->setStyle(Widget_VarioHistory, NORMAL_BOX | WS_BORDER_RIGHT | WS_BORDER_BOTTOM, WidgetContent_Vario_History);
	pages[0].getWidget(widget)->setPosition(x, y, SCREEN_WIDTH - x, SCREEN_HEIGHT - y);

	//
#define W_W		(SCREEN_WIDTH/2)
#define W_H		(60)

	widget = 0;
	x = 0, y = STATUS_TIME_HEIGHT;

	pages[1].getWidget(widget)->setStyle(Widget_VarioBar, NORMAL_BOX, WidgetContent_Vario_Lazy);
	pages[1].getWidget(widget)->setPosition(x, y, 48, W_H + W_H);
	widget++;
	x += 48;

	pages[1].getWidget(widget)->setStyle(Widget_Compass, NORMAL_BOX, WidgetContent_Heading);
	pages[1].getWidget(widget)->setPosition(x, y, 52, W_H);
	widget++;
	x += 52;

	pages[1].getWidget(widget)->setStyle(Widget_TextBox, NORMAL_TEXT | NORMAL_BOX | WS_BORDER_RIGHT, WidgetContent_Speed_Ground);
	pages[1].getWidget(widget)->setPosition(x, y, SCREEN_WIDTH - x, W_H);
	widget++;
	x = 48; y += W_H;

	pages[1].getWidget(widget)->setStyle(Widget_TextBox, NORMAL_TEXT | NORMAL_BOX | WS_BORDER_RIGHT, WidgetContent_Altitude_GPS);
	pages[1].getWidget(widget)->setPosition(x, y, SCREEN_WIDTH - x, W_H);
	widget++;
	x = 0; y += W_H;

	pages[1].getWidget(widget)->setStyle(Widget_TextBox, NORMAL_TEXT | NORMAL_BOX, WidgetContent_Vario_Lazy);
	pages[1].getWidget(widget)->setPosition(x, y, W_W, W_H);
	widget++;
	x += W_W;

	pages[1].getWidget(widget)->setStyle(Widget_TextBox, NORMAL_TEXT | NORMAL_BOX | WS_BORDER_RIGHT, WidgetContent_Time_Flight);
	pages[1].getWidget(widget)->setPosition(x, y, W_W, W_H);
	widget++;
	x = 0; y += W_H;

	pages[1].getWidget(widget)->setStyle(Widget_TextBox, NORMAL_TEXT | NORMAL_BOX | WS_BORDER_BOTTOM, WidgetContent_Altitude_Ref1);
	pages[1].getWidget(widget)->setPosition(x, y, W_W, W_H);
	widget++;
	x += W_W;

	pages[1].getWidget(widget)->setStyle(Widget_TextBox, NORMAL_TEXT | NORMAL_BOX | WS_BORDER_RIGHT | WS_BORDER_BOTTOM, WidgetContent_Temperature);
	pages[1].getWidget(widget)->setPosition(x, y, W_W, W_H);


	//
#define MIN_H	48
	widget = 0;
	x = 0, y = STATUS_TIME_HEIGHT;

	pages[2].getWidget(widget)->setStyle(Widget_TextBox, NORMAL_TEXT | NORMAL_BOX, WidgetContent_Altitude_GPS);
	pages[2].getWidget(widget)->setPosition(x, y, TEXTBOX_S_WIDTH, MIN_H);
	widget++;
	x += TEXTBOX_S_WIDTH;

	pages[2].getWidget(widget)->setStyle(Widget_TextBox, NORMAL_TEXT | NORMAL_BOX | WS_BORDER_RIGHT, WidgetContent_Speed_Ground);
	pages[2].getWidget(widget)->setPosition(x, y, TEXTBOX_S_WIDTH, MIN_H);
	widget++;
	x = 0;
	y += MIN_H;

	pages[2].getWidget(widget)->setStyle(Widget_TextBox, NORMAL_TEXT | NORMAL_BOX, WidgetContent_Ground_Level);
	pages[2].getWidget(widget)->setPosition(x, y, TEXTBOX_S_WIDTH, MIN_H);
	widget++;
	x += TEXTBOX_S_WIDTH;

	pages[2].getWidget(widget)->setStyle(Widget_TextBox, NORMAL_TEXT | NORMAL_BOX | WS_BORDER_RIGHT, WidgetContent_Vario_Lazy);
	pages[2].getWidget(widget)->setPosition(x, y, TEXTBOX_S_WIDTH, MIN_H);
	widget++;
	x = 0;
	y += MIN_H;

	pages[2].getWidget(widget)->setStyle(Widget_TextBox, NORMAL_TEXT | NORMAL_BOX, WidgetContent_Heading);
	pages[2].getWidget(widget)->setPosition(x, y, TEXTBOX_S_WIDTH, MIN_H);
	widget++;
	x += TEXTBOX_S_WIDTH;

	pages[2].getWidget(widget)->setStyle(Widget_TextBox, NORMAL_TEXT | NORMAL_BOX | WS_BORDER_RIGHT, WidgetContent_Bearing_Takeoff);
	pages[2].getWidget(widget)->setPosition(x, y, TEXTBOX_S_WIDTH, MIN_H);
	widget++;
	x = 0;
	y += MIN_H;

	pages[2].getWidget(widget)->setStyle(Widget_TextBox, NORMAL_TEXT | NORMAL_BOX, WidgetContent_Thermaling_Gain);
	pages[2].getWidget(widget)->setPosition(x, y, TEXTBOX_S_WIDTH, MIN_H);
	widget++;
	x += TEXTBOX_S_WIDTH;

	pages[2].getWidget(widget)->setStyle(Widget_TextBox, NORMAL_TEXT | NORMAL_BOX | WS_BORDER_RIGHT, WidgetContent_Thermaling_Time);
	pages[2].getWidget(widget)->setPosition(x, y, TEXTBOX_S_WIDTH, MIN_H);
	widget++;
	x = 0;
	y += MIN_H;

	pages[2].getWidget(widget)->setStyle(Widget_TextBox, NORMAL_TEXT | NORMAL_BOX | WS_BORDER_BOTTOM, WidgetContent_Distance_Takeoff);
	pages[2].getWidget(widget)->setPosition(x, y, TEXTBOX_S_WIDTH, MIN_H);
	widget++;
	x += TEXTBOX_S_WIDTH;

	pages[2].getWidget(widget)->setStyle(Widget_TextBox, NORMAL_TEXT | NORMAL_BOX | WS_BORDER_RIGHT | WS_BORDER_BOTTOM, WidgetContent_Distance_Flight);
	pages[2].getWidget(widget)->setPosition(x, y, TEXTBOX_S_WIDTH, MIN_H);
	widget++;
	x = 0;
	y += MIN_H;

	//
	widget = 0;
	x = 0, y = STATUS_TIME_HEIGHT;

	pages[3].getWidget(widget)->setStyle(Widget_TextBox, NORMAL_TEXT | NORMAL_BOX, WidgetContent_Altitude_GPS);
	pages[3].getWidget(widget)->setPosition(x, y, TEXTBOX_S_WIDTH, MIN_H);
	widget++;
	x += TEXTBOX_S_WIDTH;

	pages[3].getWidget(widget)->setStyle(Widget_TextBox, NORMAL_TEXT | NORMAL_BOX | WS_BORDER_RIGHT, WidgetContent_Speed_Ground);
	pages[3].getWidget(widget)->setPosition(x, y, TEXTBOX_S_WIDTH, MIN_H);
	widget++;
	x = 0;
	y += MIN_H;

	pages[3].getWidget(widget)->setStyle(Widget_TextBox, NORMAL_TEXT | NORMAL_BOX, WidgetContent_Ground_Level);
	pages[3].getWidget(widget)->setPosition(x, y, TEXTBOX_S_WIDTH, MIN_H);
	widget++;
	x += TEXTBOX_S_WIDTH;

	pages[3].getWidget(widget)->setStyle(Widget_TextBox, NORMAL_TEXT | NORMAL_BOX | WS_BORDER_RIGHT, WidgetContent_Altitude_AGL);
	pages[3].getWidget(widget)->setPosition(x, y, TEXTBOX_S_WIDTH, MIN_H);
	widget++;
	x = 0;
	y += MIN_H;

	pages[3].getWidget(widget)->setStyle(Widget_TextBox, NORMAL_TEXT | NORMAL_BOX, WidgetContent_Glide_Ratio);
	pages[3].getWidget(widget)->setPosition(x, y, TEXTBOX_S_WIDTH, MIN_H);
	widget++;
	x += TEXTBOX_S_WIDTH;

	pages[3].getWidget(widget)->setStyle(Widget_TextBox, NORMAL_TEXT | NORMAL_BOX | WS_BORDER_RIGHT, WidgetContenxt_FlightState_glidingCount);
	pages[3].getWidget(widget)->setPosition(x, y, TEXTBOX_S_WIDTH, MIN_H);
	widget++;
	x = 0;
	y += MIN_H;

	pages[3].getWidget(widget)->setStyle(Widget_TextBox, NORMAL_TEXT | NORMAL_BOX, WidgetContenxt_FlightState_deltaHeading_AVG);
	pages[3].getWidget(widget)->setPosition(x, y, TEXTBOX_S_WIDTH, MIN_H);
	widget++;
	x += TEXTBOX_S_WIDTH;

	pages[3].getWidget(widget)->setStyle(Widget_TextBox, NORMAL_TEXT | NORMAL_BOX | WS_BORDER_RIGHT, WidgetContenxt_FlightState_deltaHeading_SUM);
	pages[3].getWidget(widget)->setPosition(x, y, TEXTBOX_S_WIDTH, MIN_H);
	widget++;
	x = 0;
	y += MIN_H;

	pages[3].getWidget(widget)->setStyle(Widget_TextBox, NORMAL_TEXT | NORMAL_BOX | WS_BORDER_BOTTOM, WidgetContent_Thermaling_Gain);
	pages[3].getWidget(widget)->setPosition(x, y, TEXTBOX_S_WIDTH, MIN_H);
	widget++;
	x += TEXTBOX_S_WIDTH;

	pages[3].getWidget(widget)->setStyle(Widget_TextBox, NORMAL_TEXT | NORMAL_BOX | WS_BORDER_RIGHT | WS_BORDER_BOTTOM, WidgetContent_Thermaling_Time);
	pages[3].getWidget(widget)->setPosition(x, y, TEXTBOX_S_WIDTH, MIN_H);
	widget++;
	x = 0;
	y += MIN_H;	



	//
#define TRACK_W SCREEN_WIDTH // 176
#define TRACK_H (SCREEN_HEIGHT - STATUS_TIME_HEIGHT - MIN_H - MIN_H)
	widget = 0;
	x = 0, y = STATUS_TIME_HEIGHT;

	pages[4].getWidget(widget)->setStyle(Widget_TrackHistory, NORMAL_BOX | WS_BORDER_RIGHT, WidgetContent_Track_History);
	pages[4].getWidget(widget)->setPosition(x, y, TRACK_W, TRACK_H);
	widget++;
	x = 0;
	y += TRACK_H;

	pages[4].getWidget(widget)->setStyle(Widget_TextBox, NORMAL_TEXT | NORMAL_BOX, WidgetContent_Altitude_GPS);
	pages[4].getWidget(widget)->setPosition(x, y, TEXTBOX_S_WIDTH, MIN_H);
	widget++;
	x += TEXTBOX_S_WIDTH;

	pages[4].getWidget(widget)->setStyle(Widget_TextBox, NORMAL_TEXT | NORMAL_BOX | WS_BORDER_RIGHT, WidgetContent_Speed_Ground);
	pages[4].getWidget(widget)->setPosition(x, y, TEXTBOX_S_WIDTH, MIN_H);
	widget++;
	x = 0;
	y += MIN_H;

	pages[4].getWidget(widget)->setStyle(Widget_TextBox, NORMAL_TEXT | NORMAL_BOX | WS_BORDER_BOTTOM, WidgetContent_Altitude_AGL);
	pages[4].getWidget(widget)->setPosition(x, y, TEXTBOX_S_WIDTH, MIN_H);
	widget++;
	x += TEXTBOX_S_WIDTH;

	pages[4].getWidget(widget)->setStyle(Widget_TextBox, NORMAL_TEXT | NORMAL_BOX | WS_BORDER_RIGHT | WS_BORDER_BOTTOM, WidgetContent_Vario_Active);
	pages[4].getWidget(widget)->setPosition(x, y, TEXTBOX_S_WIDTH, MIN_H);
	widget++;
	x = 0;
	y += MIN_H;
}
#endif
