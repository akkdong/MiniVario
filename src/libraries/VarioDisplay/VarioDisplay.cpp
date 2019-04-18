// VarioDisplay.cpp
//

#include "VarioDisplay.h"
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


GFXfontPtr VarioDisplay::__FontStack[] = 
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
// class VarioDisplay

VarioDisplay::VarioDisplay(EPaperDriver & _driver, DeviceContext & _context) 
	: EPaperDisplay(_driver)
	, Task("Display", 8 * 1024, 1)
	, activeScreen(NULL)
	, activePref(NULL)
	, activePopup(NULL)
	, context(_context)
	, assertSleep(false)
{
}

void VarioDisplay::init()
{
	EPaperDisplay::init();
	
	statusBar.setStyle(Widget_StatusBar, 0, WidgetContent_Status_Bar);
	statusBar.setPosition(0, 0, _width, 24);
}

int VarioDisplay::begin()
{
	// ...
	assertSleep = false;
	
	return Task::begin();
}

void VarioDisplay::TaskProc()
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

void VarioDisplay::end()
{
	Task::end();
}

void VarioDisplay::attachScreen(VarioScreen * screen)
{
	activeScreen = screen;
}

void VarioDisplay::attachPreference(VarioPreference * pref)
{
}

void VarioDisplay::showPopup(VarioPopup * popupPtr)
{
	activePopup = popupPtr;
}

void VarioDisplay::sleepDevice()
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

void VarioDisplay::update()
{
	// erase screen
	fillScreen(COLOR_WHITE);

	//	
	draw(&statusBar);
	
	// preference has high priority
	if (activePref)
	{
		draw(activePref);
	}
	else if (activeScreen)
	{
		//
		draw(activeScreen);
	}
	
	// draw activated popup
	if (activePopup)
	{
		// draw any popup : menu, messagebox, ...
		draw(activePopup);
	}
	
}

void VarioDisplay::draw(VarioScreen * screen)
{
	// draw active page --> draw each widget of active page
	for (int i = 0; i < MAX_WIDGETS; i++)
	{
		Widget * widget = &screen->widget[i];
		
		if (widget->style == Widget_Empty)
			break;
		
		draw(widget);
	}
}

void VarioDisplay::draw(VarioPreference * pref)
{
}

void VarioDisplay::draw(VarioPopup * popup)
{
	switch (popup->objType)
	{
	case DispObject_PopupMenu :
		draw((PopupMenu *)popup);
		break;
	case DispObject_PopupMessageBox :
		break;
	case DispObject_PopupListBox :
		break;
	case DispObject_PopupCheckBox :
		break;
	case DispObject_PopupRadioBox :
		break;
	}
}

void VarioDisplay::draw(PopupMenu * menu)
{
	int mw = 120;
	int mh = (menu->itemCount + 1) * 24;
	int x = WORKAREA_X + (WORKAREA_W - mw) / 2;
	int y = WORKAREA_Y + (WORKAREA_H - mh) / 2;
	
	fillRect(x, y, mw, mh, COLOR_WHITE);
	drawRect(x, y, mw, mh, COLOR_BLACK);
	
	y += 24 / 2;
	for (int i = 0; i < menu->itemCount; i++)
	{
		char * str = tempString;
		
		switch (menu->items[i].itemId)
		{
		case 0x5001 : strcpy(str, "Basic settings"); break;
		case 0x5002 : sprintf(str, "Sound %s", context.volume.vario ? "Off" : "On"); break;
		case 0x5003 : sprintf(str, "Bluetooth %s", context.device.statusBT ? "Off" : "On"); break;
		case 0x5004 : strcpy(str, "Power Off"); break;
		case 0x5005 : strcpy(str, "Restart Device"); break;
		}
		
		if (i == menu->itemSelect)
		{
			// draw selected item background
			fillRect(x + 4, y + 2, mw - 8, 24 - 4, COLOR_BLACK);
		}
		
		drawText(str, 
			x, y, mw, 24,
			WS_FONT_NORMAL_2 | WS_TA_CENTER | WS_TA_MIDDLE, 
			(i == menu->itemSelect) ? COLOR_WHITE : COLOR_BLACK);	
			
		y += 24;
	}
}

void VarioDisplay::draw(Widget * widget)
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

void VarioDisplay::drawEmptyArea(Widget * widget)
{
}

void VarioDisplay::drawTextBox(Widget * widget)
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

void VarioDisplay::drawSimpleText(Widget * widget)
{
	drawText(getString(widget->getContentType()), 
		widget->x + 2, widget->y + 2, widget->w - 4, widget->h - 4, 
		widget->extraStyle, 
		COLOR_BLACK);
}

void VarioDisplay::drawIcon(Widget * widget)
{
}

void VarioDisplay::drawStatusBar(Widget * widget)
{
	int x = widget->x + 2;
	int y = widget->y + (widget->h - SB_BITMAP_HEIGHT) / 2;
	
	//
	drawBitmapBM(Bitmap_StatusBar_hanglider, x, y, SB_BITMAP_WIDTH, SB_BITMAP_HEIGHT, COLOR_WHITE, bm_invert);
	x += SB_BITMAP_WIDTH;
	
	drawBitmapBM(context.device.statusGPS ? Bitmap_StatusBar_gps_valid : Bitmap_StatusBar_gps_invalid, x, y, SB_BITMAP_WIDTH, SB_BITMAP_HEIGHT, COLOR_WHITE, bm_invert);
	x += SB_BITMAP_WIDTH;
	
	if (context.device.statusBT)
	{
		drawBitmapBM(context.device.statusBT == 1 ? Bitmap_StatusBar_bluetooth_started : Bitmap_StatusBar_bluetooth_connected, x, y, SB_BITMAP_WIDTH, SB_BITMAP_HEIGHT, COLOR_WHITE, bm_invert);
		x += SB_BITMAP_WIDTH;
	}
		
	if (context.device.statusSDCard)
	{
		drawBitmapBM(Bitmap_StatusBar_sdcard, x, y, SB_BITMAP_WIDTH, SB_BITMAP_HEIGHT, COLOR_WHITE, bm_invert);
		x += SB_BITMAP_WIDTH;
	}
	
	drawBitmapBM(context.volume.vario ? Bitmap_StatusBar_soundon : Bitmap_StatusBar_soundoff, x, y, SB_BITMAP_WIDTH, SB_BITMAP_HEIGHT, COLOR_WHITE, bm_invert);
	x += SB_BITMAP_WIDTH;
	
	//
	drawText(getString(WidgetContent_Time_Current), 
		widget->x + widget->w - STATUSBAR_TEXT_WIDTH, widget->y, STATUSBAR_TEXT_WIDTH, widget->h,
		WS_FONT_NORMAL_2 | WS_TA_CENTER | WS_TA_MIDDLE, 
		COLOR_BLACK);

	drawText(getString(WidgetContent_Battery), 
		widget->x + widget->w - (STATUSBAR_TEXT_WIDTH + STATUSBAR_TEXT_WIDTH), widget->y, STATUSBAR_TEXT_WIDTH, widget->h,
		WS_FONT_NORMAL_2 | WS_TA_CENTER | WS_TA_MIDDLE, 
		COLOR_BLACK);	
}

void VarioDisplay::drawVarioHistory(Widget * widget)
{
}

void VarioDisplay::drawVarioBar(Widget * widget)
{
}

void VarioDisplay::drawCompass(Widget * widget)
{
}

void VarioDisplay::drawBorder(Widget * widget)
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

void VarioDisplay::drawText(const char * str, int16_t x, int16_t y, uint16_t w, uint16_t h, uint32_t style, uint16_t color)
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

const char * VarioDisplay::getLabel(WidgetContentType type)
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

const char * VarioDisplay::getUnit(WidgetContentType type)
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
//		return ((context.varioState.timeCurrent - context.varioState.timeStart) < 3600) ? "mm/ss" : "hh/mm";
		return (context.varioState.timeFly < 3600) ? "mm/ss" : "hh/mm";
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

const char * VarioDisplay::getString(WidgetContentType type)
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
		#if 0
		if (context.varioState.timeCurrent != 0)
			sprintf(tempString, "%02d:%02d", (context.varioState.timeCurrent / 3600) % 24, (context.varioState.timeCurrent / 60) % 60);
		else
			strcpy(tempString, "--:--");
		#else
		{
			time_t now = time(NULL);
			sprintf(tempString, "%02d:%02d", (now / 3600) % 24, (now / 60) % 60);
		}
		#endif
		return tempString;
	case WidgetContent_Time_Flight :
		{
			time_t timeFly = context.varioState.timeFly; // (context.varioState.timeCurrent - context.varioState.timeStart);
			
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

DisplayObject * VarioDisplay::getActiveObject()
{
	if (activePopup)
		return activePopup;
	
	if (activePref)
		return activePref;

	if (activeScreen)
		return activeScreen;
	
	return NULL;
}
