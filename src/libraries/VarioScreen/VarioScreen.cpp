// VarioScreen.cpp
//

#include "VarioScreen.h"
#include "Bitmap_StatusBar.h"
#include "Bitmap_Paragliding.h"

#if CONFIG_FREERTOS_UNICORE
#define ARDUINO_RUNNING_CORE 0
#else
#define ARDUINO_RUNNING_CORE 1
#endif


////////////////////////////////////////////////////////////////////////////////////////////////
//

#include <Fonts/FreeSans6pt7b.h>
#include <Fonts/FreeSans7pt7b.h>
#include <Fonts/FreeSans8pt7b.h>
#include <Fonts/FreeSans9pt7b.h>
#include <Fonts/FreeSans16pt7b.h>
#include <Fonts/FreeSans18pt7b.h>
#include <Fonts/FreeSans24pt7b.h>

#include <Fonts/FreeSansBold6pt7b.h>
#include <Fonts/FreeSansBold7pt7b.h>
#include <Fonts/FreeSansBold8pt7b.h>
#include <Fonts/FreeSansBold9pt7b.h>
#include <Fonts/FreeSansBold16pt7b.h>
#include <Fonts/FreeSansBold18pt7b.h>
#include <Fonts/FreeSansBold24pt7b.h>


const GFXfont * VarioScreen::__FontStack[] = 
{
	&FreeSans6pt7b,
	&FreeSans7pt7b,
	&FreeSans16pt7b,
	&FreeSans24pt7b,
	&FreeSansBold6pt7b,
	&FreeSansBold8pt7b,
	&FreeSansBold16pt7b,
	&FreeSansBold24pt7b
};



////////////////////////////////////////////////////////////////////////////////////////////////
// class VarioScreen

VarioScreen::VarioScreen(EPaperDriver & _driver, DeviceContext & _context) 
	: EPaperDisplay(_driver)
	, Task("Display", 2048, 1)
	, activePage(0)
	, context(_context)
{

}

void VarioScreen::init()
{
	EPaperDisplay::init();
	
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

#define NORMAL_STATUS		(WS_FONT_NORMAL_2 | WS_TA_CENTER | WS_TA_MIDDLE)
#define NORMAL_TEXT			(WS_FONT_BOLD_3 | WS_TA_RIGHT | WS_TA_BOTTOM)
#define NORMAL_BOX			(WS_BORDER_LEFT | WS_BORDER_TOP)

	
	pages[0][widget].setStyle(Widget_StatusBar, 0, WidgetContent_Status_Bar);
	pages[0][widget].setPosition(x, y, _width, STATUS_TIME_HEIGHT);
	widget++;
	
	pages[0][widget].setStyle(Widget_SimpleText, NORMAL_STATUS, WidgetContent_Battery);
	pages[0][widget].setPosition(x + _width - STATUS_TIME_WIDTH - STATUS_TIME_WIDTH, y, STATUS_TIME_WIDTH, STATUS_TIME_HEIGHT);
	widget++;
	
	pages[0][widget].setStyle(Widget_SimpleText, NORMAL_STATUS, WidgetContent_Time_Current);
	pages[0][widget].setPosition(x + _width - STATUS_TIME_WIDTH, y, STATUS_TIME_WIDTH, STATUS_TIME_HEIGHT);
	widget++;
	y += STATUS_TIME_HEIGHT;
	
	pages[0][widget].setStyle(Widget_TextBox, NORMAL_TEXT | NORMAL_BOX, WidgetContent_Altitude_GPS);
	pages[0][widget].setPosition(x, y, TEXTBOX_S_WIDTH, TEXTBOX_S_HEIGHT);
	widget++;
	x += TEXTBOX_S_WIDTH;

	pages[0][widget].setStyle(Widget_TextBox, NORMAL_TEXT | NORMAL_BOX | WS_BORDER_RIGHT, WidgetContent_Speed_Ground);
	pages[0][widget].setPosition(x, y, TEXTBOX_S_WIDTH, TEXTBOX_S_HEIGHT);
	widget++;
	x = 0;
	y += TEXTBOX_S_HEIGHT;

	pages[0][widget].setStyle(Widget_TextBox, NORMAL_TEXT | NORMAL_BOX, WidgetContent_Heading);
	pages[0][widget].setPosition(x, y, TEXTBOX_S_WIDTH, TEXTBOX_S_HEIGHT);
	widget++;
	x += TEXTBOX_S_WIDTH;

	pages[0][widget].setStyle(Widget_TextBox, NORMAL_TEXT | NORMAL_BOX | WS_BORDER_RIGHT, WidgetContent_Vario_Lazy);
	pages[0][widget].setPosition(x, y, TEXTBOX_S_WIDTH, TEXTBOX_S_HEIGHT);
	widget++;
	x = 0;
	y += TEXTBOX_S_HEIGHT;
	
	pages[0][widget].setStyle(Widget_TextBox, NORMAL_TEXT | NORMAL_BOX | WS_BORDER_BOTTOM, WidgetContent_Altitude_Baro);
	pages[0][widget].setPosition(x, y, TEXTBOX_S_WIDTH, TEXTBOX_S_HEIGHT);
	widget++;
	x += TEXTBOX_S_WIDTH;

	pages[0][widget].setStyle(Widget_TextBox, NORMAL_TEXT | NORMAL_BOX | WS_BORDER_RIGHT | WS_BORDER_BOTTOM, WidgetContent_Temperature);
	pages[0][widget].setPosition(x, y, TEXTBOX_S_WIDTH, TEXTBOX_S_HEIGHT);
	widget++;
	x = 0;
	y += TEXTBOX_S_HEIGHT;

	pages[0][widget].setStyle(Widget_TextBox, WS_FONT_BOLD_4 | WS_TA_LEFT | WS_TA_BOTTOM | NORMAL_BOX | WS_BORDER_RIGHT | WS_BORDER_BOTTOM, WidgetContent_Pressure);
	pages[0][widget].setPosition(x, y, 176, 264 - y);
}

int VarioScreen::begin()
{
	// ...
	assertSleep = false;
	
	return Task::begin();
}

void VarioScreen::TaskProc()
{
	bool fastUpdate = false;
	
	//
	init();
	
	//
	while (1)
	{
		//
		update();
		refresh(fastUpdate);
		fastUpdate = true;
		
		//
		if (assertSleep)
		{
			// display sleep screen & sleep e-ink
			setRotation(0);
			fillScreen(COLOR_WHITE);
			setFont(__FontStack[WS_FONT_NORMAL_3]);
			setTextColor(COLOR_BLACK, COLOR_WHITE);
			drawBitmapBM(Bitmap_Paragliding, 6, 24, 164, 166, COLOR_WHITE, bm_invert);
			setCursor(0, 240);
			print("Fly high~");
			setFont(__FontStack[WS_FONT_NORMAL_1]);
			setCursor(0, 260);
			print("Notorious Rascal 2019");
			
			refresh(false);
			sleep(); // 
			
			// now sleep device
			sleepDevice();
		}
		
		//
		const TickType_t xDelay = 50 / portTICK_PERIOD_MS;
		vTaskDelay(xDelay);
	}
}

void VarioScreen::end()
{
	Task::end();
}

void VarioScreen::sleepDevice()
{
    const int ext_wakeup_pin_1 = 34;
    const uint64_t ext_wakeup_pin_1_mask = 1ULL << ext_wakeup_pin_1;
    const int ext_wakeup_pin_2 = 35;
    const uint64_t ext_wakeup_pin_2_mask = 1ULL << ext_wakeup_pin_2;

	// Enabling EXT1 wakeup on pins GPIO34, GPIO35
    esp_sleep_enable_ext1_wakeup(ext_wakeup_pin_1_mask | ext_wakeup_pin_2_mask, ESP_EXT1_WAKEUP_ANY_HIGH  );
	// Entering deep sleep
	esp_deep_sleep_start();
	
	// never comes here
	// ...
}

void VarioScreen::update()
{
	// erase screen
	fillScreen(COLOR_WHITE);
	
	// draw active page --> draw each widget of active page
	for (int i = 0; i < MAX_WIDGETS; i++)
	{
		Widget * widget = &pages[activePage][i];
		
		if (widget->style == Widget_Empty)
			break;
		
		draw(widget);
	}
	
	// draw activated popup
	// ...
}

void VarioScreen::draw(Widget * widget)
{
	switch (widget->getStyle())
	{
	case Widget_Empty :
		drawEmptyArea(widget);
		break;

	case Widget_TextBox :
		drawTextBox(widget);
		break;
		
	case Widget_SimpleText :
		drawSimpleText(widget);
		break;
		
	case Widget_Icon :
		drawIcon(widget);
		break;
		
	case Widget_StatusBar :
		drawStatusBar(widget);
		break;
		
	case Widget_VarioHistory:
		drawVarioHistory(widget);
		break;
		
	case Widget_VarioBar:
		drawVarioBar(widget);
		break;
		
	case Widget_Compass :
		drawCompass(widget);
		break;
		
	default :
		// nop
		break;
	}
}

void VarioScreen::drawEmptyArea(Widget * widget)
{
}

void VarioScreen::drawTextBox(Widget * widget)
{
	drawBorder(widget);
	
	drawText(getLabel(widget->getContentType()), 
		widget->x + 2, widget->y + 2, widget->w - 6, 14, 
		WS_FONT_NORMAL_2 | WS_TA_LEFT |  WS_TA_BOTTOM, 
		COLOR_BLACK);
		
	drawText(getUnit(widget->getContentType()), 
		widget->x + 2, widget->y + 2, widget->w - 6, 14, 
		WS_FONT_NORMAL_1 | WS_TA_RIGHT |  WS_TA_BOTTOM, 
		COLOR_BLACK);
		
	drawText(getString(widget->getContentType()), 
		widget->x + 2, widget->y + 14, widget->w - 6, widget->h - 18, 
		widget->extraStyle, 
		COLOR_BLACK);
}

void VarioScreen::drawSimpleText(Widget * widget)
{
	drawText(getString(widget->getContentType()), 
		widget->x + 2, widget->y + 2, widget->w - 4, widget->h - 4, 
		widget->extraStyle, 
		COLOR_BLACK);
}

void VarioScreen::drawIcon(Widget * widget)
{
}

void VarioScreen::drawStatusBar(Widget * widget)
{
	int x = widget->x + 2;
	int y = widget->y + (widget->h - SB_BITMAP_HEIGHT) / 2;
	
	drawBitmapBM(Bitmap_StatusBar_hanglider, x, y, SB_BITMAP_WIDTH, SB_BITMAP_HEIGHT, COLOR_WHITE, bm_invert);
	x += SB_BITMAP_WIDTH;
	
	drawBitmapBM(context.device.statusGPS ? Bitmap_StatusBar_gps_valid : Bitmap_StatusBar_gps_invalid, x, y, SB_BITMAP_WIDTH, SB_BITMAP_HEIGHT, COLOR_WHITE, bm_invert);
	x += SB_BITMAP_WIDTH;
	
	if (context.device.statusBT)
	{
		drawBitmapBM(Bitmap_StatusBar_bluetooth, x, y, SB_BITMAP_WIDTH, SB_BITMAP_HEIGHT, COLOR_WHITE, bm_invert);
		x += SB_BITMAP_WIDTH;
	}
		
	if (context.device.statusSDCard)
	{
		drawBitmapBM(Bitmap_StatusBar_sdcard, x, y, SB_BITMAP_WIDTH, SB_BITMAP_HEIGHT, COLOR_WHITE, bm_invert);
		x += SB_BITMAP_WIDTH;
	}
}

void VarioScreen::drawVarioHistory(Widget * widget)
{
}

void VarioScreen::drawVarioBar(Widget * widget)
{
}

void VarioScreen::drawCompass(Widget * widget)
{
}

void VarioScreen::drawBorder(Widget * widget)
{
	if (widget->extraStyle & WS_BORDER_TOP)
		drawFastHLine(widget->x, widget->y, widget->w, COLOR_BLACK);
	if (widget->extraStyle & WS_BORDER_BOTTOM)
		drawFastHLine(widget->x, widget->y + widget->h - 1, widget->w, COLOR_BLACK);
	if (widget->extraStyle & WS_BORDER_LEFT)
		drawFastVLine(widget->x, widget->y, widget->h, COLOR_BLACK);
	if (widget->extraStyle & WS_BORDER_RIGHT)
		drawFastVLine(widget->x + widget->w - 1, widget->y, widget->h, COLOR_BLACK);
}

void VarioScreen::drawText(const char * str, int16_t x, int16_t y, uint16_t w, uint16_t h, uint32_t style, uint16_t color)
{
	int16_t tx, ty;
	uint16_t tw, th;
	
	setFont(__FontStack[style & WS_FONT_MASK]);
	getTextBounds(str, 0, 0, &tx, &ty, &tw, &th);
	
	if (style & WS_TA_RIGHT)
		x = x + w - tw;
	else if (style & WS_TA_CENTER)
		x = x + (w - tw) / 2;
	if (style & WS_TA_TOP)
		y = y + th;
	else if (style & WS_TA_MIDDLE)
		y = y + (h + th) / 2;
	else
		y = y + h;
	
	setCursor(x, y);
	setTextColor(color);
	print(str);
}



////////////////////////////////////////////////////////////////////////////////////////////////
//

const char * VarioScreen::getLabel(WidgetContentType type)
{
	switch (type)
	{
	case WidgetContent_Speed_Ground :
		return "G.Spd";
	case WidgetContent_Speed_Air :
		return "A.Spd";
	case WidgetContent_Heading :
		return "Track";
//	case WidgetContent_Heading_GPS :
//		return "Heading";
//	case WidgetContent_Heading_Compass :
//		return "Heading";
//	case WidgetContent_Bearing :
//		return "Heading";
	case WidgetContent_Longitude :
		return "Lon";
	case WidgetContent_Latitude :
		return "Lat";
	case WidgetContent_Altitude_GPS :	// QNH
		return "Alt.G";
	case WidgetContent_Altitude_Baro :	// QNE
		return "Alt.P";
	case WidgetContent_Altitude_AGL :	// QFE
		return "AGL";
	case WidgetContent_Altitude_Ref1 :	// QFE
		return "Alt1";
	case WidgetContent_Altitude_Ref2 :	// QFE
		return "Alt2";
	case WidgetContent_Altitude_Ref3 :	// QFE
		return "Alt3";
	case WidgetContent_Glide_Ratio :
		return "L/D";
	case WidgetContent_Vario_Active :
		return "Vario";
	case WidgetContent_Vario_Lazy :		// 1s average
		return "V.Spd";
	case WidgetContent_DateTime :
		return "Date";
	case WidgetContent_Time_Current :
		return "Time";
	case WidgetContent_Time_Flight :
		return "FTime";
	case WidgetContent_Pressure :
		return "Prs";
	case WidgetContent_Temperature :
		return "Temp";
//	case WidgetContent_Thermal_Time :
//		return "";
//	case WidgetContent_Thermal_Gain :
//		return "";
	}
	
	return "";
}

const char * VarioScreen::getUnit(WidgetContentType type)
{
	switch (type)
	{
	case WidgetContent_Speed_Ground :
		return "Km/h";
	case WidgetContent_Speed_Air :
		return "Km/h";
	case WidgetContent_Heading :
		return "Deg";
//	case WidgetContent_Heading_GPS :
//		return "Deg";
//	case WidgetContent_Heading_Compass :
//		return "Deg";
//	case WidgetContent_Bearing :
//		return "Deg";
	case WidgetContent_Longitude :
		return "";
	case WidgetContent_Latitude :
		return "";
	case WidgetContent_Altitude_GPS :		// QNH
		return "m";
	case WidgetContent_Altitude_Baro :	// QNE
		return "m";
	case WidgetContent_Altitude_AGL :		// QFE
		return "m";
	case WidgetContent_Altitude_Ref1 :	// QFE
		return "m";
	case WidgetContent_Altitude_Ref2 :	// QFE
		return "m";
	case WidgetContent_Altitude_Ref3 :	// QFE
		return "m";
	case WidgetContent_Glide_Ratio :
		return "";
	case WidgetContent_Vario_Active :
		return "m/s";
	case WidgetContent_Vario_Lazy :		// 1s average
		return "m/s";
	case WidgetContent_DateTime :
		return "";
	case WidgetContent_Time_Current :
		return "";
	case WidgetContent_Time_Flight :
		return ((context.varioState.timeCurrent - context.varioState.timeStart) < 3600) ? "mm/ss" : "hh/mm";
	case WidgetContent_Pressure :
		return "hPa";
	case WidgetContent_Temperature :
		return "C";
//	case WidgetContent_Thermal_Time :
//		return "";
//	case WidgetContent_Thermal_Gain :
//		return "";
	}
	
	return "";
}

const char * VarioScreen::getString(WidgetContentType type)
{
	switch (type)
	{
	case WidgetContent_Speed_Ground :
		return itoa(context.varioState.speedGround + 0.5, tempString, 10);
	case WidgetContent_Speed_Air :
		return itoa(context.varioState.speedAir + 0.5, tempString, 10);
	case WidgetContent_Heading :
		return itoa(context.varioState.heading + 0.5, tempString, 10);
//	case WidgetContent_Heading_GPS :
//		return "";
//	case WidgetContent_Heading_Compass :
//		return "";
//	case WidgetContent_Bearing :
//		return "";
	case WidgetContent_Longitude :
		return "";
	case WidgetContent_Latitude :
		return "";
	case WidgetContent_Altitude_GPS :	// QNH
		return itoa(context.varioState.altitudeGPS + 0.5, tempString, 10);
	case WidgetContent_Altitude_Baro :	// QNE
		return itoa(context.varioState.altitudeBaro + 0.5, tempString, 10);
	case WidgetContent_Altitude_AGL :	// QFE
		return itoa(context.varioState.altitudeAGL + 0.5, tempString, 10);
	case WidgetContent_Altitude_Ref1 :	// QFE
		return itoa(context.varioState.altitudeRef1 + 0.5, tempString, 10);
	case WidgetContent_Altitude_Ref2 :	// QFE
		return itoa(context.varioState.altitudeRef2 + 0.5, tempString, 10);
	case WidgetContent_Altitude_Ref3 :	// QFE
		return itoa(context.varioState.altitudeRef3 + 0.5, tempString, 10);
	case WidgetContent_Glide_Ratio :
		if (context.varioState.glideRatio == 0.0)
			return "--";
		sprintf(tempString, "%.1f", context.varioState.glideRatio + 0.05);
		return tempString;
	case WidgetContent_Vario_Active :
		sprintf(tempString, "%.1f", context.varioState.speedVertActive);
		return tempString;
	case WidgetContent_Vario_Lazy :		// 1s average
		sprintf(tempString, "%.1f", context.varioState.speedVertLazy);
		return tempString;
	case WidgetContent_DateTime :
		return "16:34";
	case WidgetContent_Time_Current :
		if (context.varioState.timeCurrent != 0)
			sprintf(tempString, "%02d:%02d", (context.varioState.timeCurrent / 3600) % 24, (context.varioState.timeCurrent / 60) % 60);
		else
			strcpy(tempString, "--:--");
		return tempString;
	case WidgetContent_Time_Flight :
		{
			time_t timeFly = (context.varioState.timeCurrent - context.varioState.timeStart);
			
			if (timeFly < 3600)
				sprintf(tempString, "%02d:%02d", timeFly / 60, timeFly % 60);
			else 
				sprintf(tempString, "%02d:%02d", timeFly / 3600, (timeFly / 60) % 60);
		}
		return tempString;
	case WidgetContent_Pressure :
		sprintf(tempString, "%.2f", context.varioState.pressure);
		return tempString;
	case WidgetContent_Temperature :
		sprintf(tempString, "%.1f", context.varioState.temperature);
		return tempString;
//	case WidgetContent_Thermal_Time :
//		return "";
//	case WidgetContent_Thermal_Gain :
//		return "";
	case WidgetContent_Battery :
		sprintf(tempString, "%.1fV", context.device.batteryPower);
		return tempString;
	case WidgetContent_Title :
		return "Rascal";
	}
	
	return "";
}

