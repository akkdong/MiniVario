// VarioDisplay.cpp
//

#include "VarioDisplay.h"
#include "Bitmap_StatusBar.h"
#include "Bitmap_Paragliding.h"
#include "TaskWatchdog.h"
#include <math.h>

#ifndef ARDUINO_RUNNING_CORE
#if CONFIG_FREERTOS_UNICORE
#define ARDUINO_RUNNING_CORE 0
#else
#define ARDUINO_RUNNING_CORE 1
#endif
#endif

#define TO_RADIAN(x)			((x)*(PI/180))


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
	, displayMode(_VARIO)
	, updateCount(0)
{
}

void VarioDisplay::init()
{
	EPaperDisplay::init();

#if USE_GOOD_DISPLAY
	setRotation(2);
	setMirrorMode(true);
#endif

	statusBar.setStyle(Widget_StatusBar, 0, WidgetContent_Status_Bar);
	statusBar.setPosition(0, 0, _width, 24);
}

int VarioDisplay::begin(bool confirmWakeup)
{
	// ...
	displayMode = confirmWakeup ? _CONFIRM : _VARIO;
	
	//
	return Task::createPinnedToCore(1) ? 0 : -1;
}

int VarioDisplay::beginFirmwareUpdate()
{
	// ...
	displayMode = _UPDATE;
	
	//
	return Task::createPinnedToCore(1) ? 0 : -1;
}

void VarioDisplay::TaskProc()
{
	//
	//TaskWatchdog::add(NULL);

	//
	init();
	//TaskWatchdog::reset();

	//
	switch (displayMode)
	{
	case _CONFIRM :
		//
		drawLogoScreen();
		drawConfirmMessage();

		//
		#if USE_GOOD_DISPLAY
		refresh((uint32_t)0);
		#else
		refresh(false);
		#endif
		break;
	case _UPDATE :
		break;
	}
	
	//
	while (1)
	{
		//
		//TaskWatchdog::reset();

		//
		switch (displayMode)
		{
		case _CONFIRM :
			// nop
			break;

		case _UPDATE :
			updateFirmwareUpdate();
			#if USE_GOOD_DISPLAY
			refresh(updateCount++);
			#else
			refresh(updateCount++ > 0 ? true : false);
			#endif
			break;

		case _VARIO :
			// update & refresh
			update();
			#if USE_GOOD_DISPLAY
			{
				uint32_t s = millis();
				refresh(updateCount++);
				Serial.printf("refresh: %u\n", millis() - s);
			}
			#else
			refresh(updateCount++ > 0 ? true : false);
			#endif

			// periodic full refresh: update rate : 2.5 times per second ==> 5 min = 300 x 2.5 = 750
			#if USE_GOOD_DISPLAY
			if (updateCount >= 50)
			#else
			if (updateCount > 750)
			#endif
				updateCount = 0;
			break;

		case _DEEPSLEEP :
			// draw logo screen: default sleep screen
			drawLogoScreen();
			// full refresh
			refresh((uint32_t)0);

			// sleep display
			sleep(); 
			// now sleep device
			sleepDevice();
		}
		
		//
		const TickType_t xDelay = /*50*/200 / portTICK_PERIOD_MS;
		vTaskDelay(xDelay);
	}
}

void VarioDisplay::end()
{
	Task::destroy();
}

void VarioDisplay::attachScreen(VarioScreen * screen)
{
	activeScreen = screen;
	updateCount = 0;
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
    esp_sleep_enable_ext1_wakeup(ext_wakeup_pin_1_mask /*| ext_wakeup_pin_2_mask*/, ESP_EXT1_WAKEUP_ANY_HIGH  );
	// Entering deep sleep
	esp_deep_sleep_start();
	
	// never comes here
	// ...
}

void VarioDisplay::updateFirmwareUpdate()
{
	fillScreen(COLOR_WHITE);
	setFont(__FontStack[WS_FONT_NORMAL_2]);
	setTextColor(COLOR_BLACK, COLOR_WHITE);
	setCursor(10, 10);
	print("Update in progressing...");
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
		Widget * widget = screen->getWidget(i);
		if (! widget)
			break;

		if (widget->style == Widget_Empty)
			break;
		
		draw(widget);
	}
}

void VarioDisplay::draw(VarioPreference * pref)
{
	int16_t item = pref->itemTop;
	int16_t x = WORKAREA_X, y = WORKAREA_Y, w, h;

	while (item < pref->itemCount)
	{
		PrefItem * prefItem = &pref->prefs[item];

		w = WORKAREA_W;
		h = prefItem->getHeight();

		if (y + h > WORKAREA_Y + WORKAREA_H)
			break;

		drawPrefItem(prefItem, x, y, w, h, item == pref->itemSelect);
	}
}

void VarioDisplay::draw(VarioPopup * popup)
{
	switch (popup->objType)
	{
	case DispObject_PopupTopMenu :
		draw((PopupTopMenu *)popup);
		break;
	case DispObject_PopupMessageBox :
		break;
	case DispObject_PopupListBox :
		break;
	case DispObject_PopupCheckBox :
		break;
	case DispObject_PopupRadioBox :
		break;
	case DispObject_PopupWebService :
		draw((PopupWebService *)popup);
		break;
	}
}

void VarioDisplay::draw(PopupTopMenu * menu)
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
		case TMID_SHOW_PREFERENCE : strcpy(str, "Basic settings"); break;
		case TMID_TOGGLE_WEBSERVICE : strcpy(str, "Web Service On"); break;
		case TMID_TOGGLE_SOUND : sprintf(str, "Sound %s", context.volume.vario ? "Off" : "On"); break;
		case TMID_TOGGLE_BLUETOOTH : sprintf(str, "Bluetooth %s", context.deviceState.statusBT ? "Off" : "On"); break;
		case TMID_RESET_DEVICE : strcpy(str, "Restart Device"); break;
		case TMID_POWER_OFF : strcpy(str, "Power Off"); break;
		case TMID_SIMULATION_MODE : sprintf(str, "Simulation %s", context.deviceDefault.enableSimulation ? "Off" : "On"); break;
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

void VarioDisplay::draw(PopupWebService * service)
{
	int mw = 140;
	int mh = 160;
	int x = WORKAREA_X + (WORKAREA_W - mw) / 2;
	int y = WORKAREA_Y + (WORKAREA_H - mh) / 2;
	char sz[32];

	fillRect(x, y, mw, mh, COLOR_WHITE);
	drawRect(x, y, mw, mh, COLOR_BLACK);

	x += 10;
	y += 10;
	drawText("Web Service", x, y, mw - 10, 24, WS_FONT_NORMAL_2 | WS_TA_LEFT | WS_TA_TOP, COLOR_BLACK);

	y += 24;
	drawText("WiFi:", x, y, mw - 10, 18, WS_FONT_NORMAL_2 | WS_TA_LEFT | WS_TA_MIDDLE, COLOR_BLACK);
	y += 18;
	drawText(context.deviceDefault.wifiSSID, x + 8, y, mw - 18, 18, WS_FONT_NORMAL_2 | WS_TA_LEFT | WS_TA_MIDDLE, COLOR_BLACK);
	y += 18;
	drawText(context.deviceDefault.wifiPassword, x + 8, y, mw - 18, 18, WS_FONT_NORMAL_2 | WS_TA_LEFT | WS_TA_MIDDLE, COLOR_BLACK);
	y += 36;
	drawText("IP", x, y, mw - 10, 18, WS_FONT_NORMAL_2 | WS_TA_LEFT | WS_TA_MIDDLE, COLOR_BLACK);
	y += 18;
	drawText("192.168.1.1", x + 8, y, mw - 18, 18, WS_FONT_NORMAL_2 | WS_TA_LEFT | WS_TA_MIDDLE, COLOR_BLACK);
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

	case Widget_TrackHistory:
		drawTrackHistory(widget);
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
	
	drawBitmapBM(context.deviceState.statusGPS ? Bitmap_StatusBar_gps_valid : Bitmap_StatusBar_gps_invalid, x, y, SB_BITMAP_WIDTH, SB_BITMAP_HEIGHT, COLOR_WHITE, bm_invert);
	x += SB_BITMAP_WIDTH;
	
	if (context.deviceState.statusBT)
	{
		drawBitmapBM(context.deviceState.statusBT == 1 ? Bitmap_StatusBar_bluetooth_started : Bitmap_StatusBar_bluetooth_connected, x, y, SB_BITMAP_WIDTH, SB_BITMAP_HEIGHT, COLOR_WHITE, bm_invert);
		x += SB_BITMAP_WIDTH;
	}
		
	if (context.deviceState.statusSDCard)
	{
		drawBitmapBM(context.deviceState.statusSDCard == 1 ? Bitmap_StatusBar_sdcard : Bitmap_StatusBar_sdcard_logging, x, y, SB_BITMAP_WIDTH, SB_BITMAP_HEIGHT, COLOR_WHITE, bm_invert);
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
	drawBorder(widget);

	int y0 = widget->y + widget->h / 2;
//	int yt = widget->x + 6;
//	int yb = widget->x + widget->h - 6;
	int xs = widget->x + 8;
//	int xe = widget->x + widget->w - 8;
	int bw = 3;	
	int mh = (widget->h - 8) / 2;
	int j = context.varioState.speedVertNext;
	int v;

	for (int i = 0; i < MAX_VARIO_HISTORY; i++)
	{
		v = (int)(context.varioState.speedVertHistory[j] * 6);
		j = (j + 1) % MAX_VARIO_HISTORY;

		if (v > 0)
		{
			if (v > mh)
				v = mh;
			fillRect(xs, y0 - v, bw, v, COLOR_BLACK);
		}
		else if (v < 0)
		{
			v = -v;
			if (v > mh)
				v = mh;
			fillRect(xs, y0, bw, v, COLOR_BLACK);
		}
		else
		{
			drawFastHLine(xs, y0, bw, COLOR_BLACK);
		}

		xs += bw;
	}
}

void VarioDisplay::drawTrackHistory(Widget * widget)
{
#define ZOOM_FACTOR			(0.6)

	//
	drawBorder(widget);

	//
	float theta = TO_RADIAN(180 - context.varioState.heading);

	for (int i = context.flightState.rearPoint; i != context.flightState.frontPoint; )
	{
#if 1 // 0 clock is heading
		float x0 = context.flightState.trackDistance[i].dx * ZOOM_FACTOR;
		float y0 = context.flightState.trackDistance[i].dy * ZOOM_FACTOR;

		int16_t x = (widget->x + widget->w / 2) - (int16_t)(x0 * cos(theta) - y0 * sin(theta));
		int16_t y = (widget->y + widget->h / 2) - (int16_t)(x0 * sin(theta) + y0 * cos(theta));
#else // 0 clock is north
		int16_t x = widget->x + widget->w / 2;
		int16_t y = widget->y + widget->h / 2;

		x -= context.flightState.trackDistance[i].dx * ZOOM_FACTOR;
		y += context.flightState.trackDistance[i].dy * ZOOM_FACTOR;
#endif

		if (widget->x + 2 < x && widget->y + 2 < y && x < widget->x + widget->w - 2 && y < widget->y + widget->h - 2)
		{
			if (context.flightState.trackPoints[i].vario < 0)
			{
				drawRect(x - 2, y - 2, 4, 4, COLOR_BLACK);
			}
			else
			{
				int16_t r = 2;

				if (context.flightState.trackPoints[i].vario > 1)
					r = (context.flightState.trackPoints[i].vario > 2) ? 4 : 3;

				drawCircle(x, y, r, COLOR_BLACK);
			}
		}

		i = (i + 1) % MAX_TRACK_HISTORY;
	}

	// draw glider
	{
		int16_t cx = widget->x + widget->w / 2;
		int16_t cy = widget->y + widget->h / 2;

		drawLine(cx - 10, cy, cx, cy - 18, COLOR_BLACK);
		drawLine(cx, cy - 18, cx + 10, cy, COLOR_BLACK);
	}
}

void VarioDisplay::drawVarioBar(Widget * widget)
{
	#define BAR_H	(4)
	#define BAR_G	(1)
	int16_t h = (widget->h - 6) / 2;
	int16_t w = widget->w - 6;
	int16_t x = widget->x + 3;
	int16_t y = widget->y + widget->h / 2;
	int16_t sign = 1;

	float vario = context.varioState.speedVertLazy;
	if (vario > 0) 
	{
		sign = 1;
		vario = (vario > 5.0) ? 5.0 : vario;
	}
	else
	{
		sign = -1;
		vario = (vario < -5.0) ? 5.0 : -vario;
	}

	// vario --> height
	// vario : 5.0 = y : h, y = vario / 5.0 * h;
	int16_t v_offset = (int16_t)(vario / 5.0 * h);

	drawBorder(widget);
	drawFastHLine(x, y, w, COLOR_BLACK);	

	for (int16_t yy = 0; yy <= h; yy += (BAR_H + BAR_G))
	{
		if (v_offset <= yy)
			break;

		if (sign > 0)
		{
			fillRect(x, y - (yy + BAR_H), w, BAR_H, COLOR_BLACK);
		}
		else
		{
			drawRect(x, y + yy, w, BAR_H, COLOR_BLACK);
		}
		
		w -= 2;
	}
}

void VarioDisplay::drawCompass(Widget * widget)
{
	int16_t r = (widget->w < widget->h ? widget->w : widget->h) / 2;
	int16_t cx = widget->x + widget->w / 2;
	int16_t cy = widget->y + widget->h / 2;

	drawBorder(widget);
	drawCircle(cx, cy, r - 4, COLOR_BLACK);
	drawTriangle(cx, cy, r - 10, -context.varioState.heading);

	if  (context.flightState.bearingTakeoff >= 0)
	{
		double theta = context.flightState.bearingTakeoff - context.varioState.heading;

		int16_t x = (r - 10) * sin(theta * PI / 180.0) + cx;
		int16_t y = -(r - 10) * cos(theta * PI / 180.0) + cy;

		drawCircle(x, y, 4, COLOR_BLACK);
	}
}

void VarioDisplay::drawTriangle(int16_t cx, int16_t cy, int16_t r, int16_t heading)
{
	double theta;
	int16_t x1, y1, x2, y2, x3, y3;

	//
	theta = heading;
	x1 = r * sin(theta * PI / 180.0) + cx;
	y1 = -r * cos(theta * PI / 180.0) + cy;

	//
	theta = heading + 140.0;
	x2 = r * sin(theta * PI / 180.0) + cx;
	y2 = -r * cos(theta * PI / 180.0) + cy;
	//

	theta = heading + 220.0;
	x3 = r * sin(theta * PI / 180.0) + cx;
	y3 = -r * cos(theta * PI / 180.0) + cy;

	//
	drawLine(x1, y1, x2, y2, COLOR_BLACK);
	drawLine(x2, y2, cx, cy, COLOR_BLACK);
	drawLine(cx, cy, x3, y3, COLOR_BLACK);
	drawLine(x1, y1, x3, y3, COLOR_BLACK);
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

void VarioDisplay::drawPrefItem(PrefItem * item, int16_t x, int16_t y, int16_t w, int16_t h, bool selected)
{	
}

void VarioDisplay::drawLogoScreen()
{
	fillScreen(COLOR_WHITE);
	setFont(__FontStack[WS_FONT_NORMAL_3]);
	setTextColor(COLOR_BLACK, COLOR_WHITE);
	drawBitmapBM(Bitmap_Paragliding, 6, 24, 164, 166, COLOR_WHITE, bm_invert);
	setCursor(0, 240);
	print("Fly High~");
	setFont(__FontStack[WS_FONT_NORMAL_1]);
	setCursor(0, 260);
	print("Notorious Rascal 2019");
}

void VarioDisplay::drawConfirmMessage()
{
	int mw = 160;
	int mh = 50;
	int x = (width() - mw) / 2;
	int y = (height() - mh) / 2;
	
	fillRect(x, y, mw, mh, COLOR_WHITE);
	drawRect(x, y, mw, mh, COLOR_BLACK);

	drawText("Confirm wakeup !!", 
		x, y, mw, 25,
		WS_FONT_NORMAL_2 | WS_TA_CENTER | WS_TA_MIDDLE, 
		COLOR_BLACK);
	y += 25;
	drawText("Press OK in 10 seconds.", 
		x, y, mw, 25,
		WS_FONT_NORMAL_1 | WS_TA_CENTER | WS_TA_MIDDLE, 
		COLOR_BLACK);	
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
	case WidgetContent_Bearing_Takeoff :
		return "Track";
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
		return "Time.Fl";
	case WidgetContent_Time_Takeoff :
		return "Time.Ta";
	case WidgetContent_Pressure :
		return "Prs";
	case WidgetContent_Temperature :
		return "Temp";
	case WidgetContent_Distance_Takeoff :
		return "Dist.Ta";
	case WidgetContent_Distance_Flight :
		return "Dist.Fl";
	case WidgetContent_Thermaling_Gain :
		return "Th.Gain";
	case WidgetContent_Thermaling_Time :
		return "Th.Time";
	case WidgetContent_Thermaling_Slope :
		return "";

	case WidgetContent_Altitude_Max :
		return "Alt.Max";
	case WidgetContent_Altitude_Min :
		return "Alt.Min";
	case WidgetContent_ClimbRate_Max :
		return "Clm.Max";
	case WidgetContent_SinkRate_Max :
		return "Snk.Max";
	case WidgetContent_Total_Thermaling :
		return "Th.Cnt";
	case WidgetContent_Max_ThermalingGain :
		return "Th.Max";

	case WidgetContent_Ground_Level :
		return "G.Lvl";

	case WidgetContenxt_FlightState_deltaHeading_AVG :
		return "HdgAVG";
	case WidgetContenxt_FlightState_deltaHeading_SUM :
		return "HdgSUM";
	case WidgetContenxt_FlightState_glidingCount :
		return "GldCnt";
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
		return "Hdg";
//	case WidgetContent_Heading_GPS :
//		return "Deg";
//	case WidgetContent_Heading_Compass :
//		return "Deg";
	case WidgetContent_Bearing_Takeoff :
		return "Brg";
	case WidgetContent_Longitude :
		return "";
	case WidgetContent_Latitude :
		return "";
	case WidgetContent_Altitude_GPS :	// QNH
		return "m";
	case WidgetContent_Altitude_Baro :	// QNE
		return "m";
	case WidgetContent_Altitude_AGL :	// QFE
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
	case WidgetContent_Vario_Lazy :		// x seconds average
		return "m/s";
	case WidgetContent_DateTime :
		return "";
	case WidgetContent_Time_Current :
		return "";
	case WidgetContent_Time_Flight :
		return (context.flightState.flightTime < 3600) ? "m:s" : "h:m";
	case WidgetContent_Time_Takeoff :
		return "h:m";
	case WidgetContent_Pressure :
		return "hPa";
	case WidgetContent_Temperature :
		return "C";
	case WidgetContent_Distance_Takeoff :
	case WidgetContent_Distance_Flight :
		return "Km";
	case WidgetContent_Thermaling_Gain :
		return "m";
	case WidgetContent_Thermaling_Time :
		return "m:s";

	case WidgetContent_Altitude_Max :
		return "m";
	case WidgetContent_Altitude_Min :
		return "m";
	case WidgetContent_ClimbRate_Max :
		return "m/s";
	case WidgetContent_SinkRate_Max :
		return "m/s";
	case WidgetContent_Total_Thermaling :
		return "";
	case WidgetContent_Max_ThermalingGain :		
		return "m";

	case WidgetContent_Ground_Level :
		return "m";
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
		return itoa(context.varioState.heading, tempString, 10);
//	case WidgetContent_Heading_GPS :
//		return "";
//	case WidgetContent_Heading_Compass :
//		return "";
	case WidgetContent_Bearing_Takeoff :
		if (context.flightState.bearingTakeoff < 0)
			return "--";
		return itoa(context.flightState.bearingTakeoff, tempString, 10);
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
		if (context.flightState.glideRatio < 0.0)
			return "-/-";
		else if (context.flightState.glideRatio == 0)
			return "inf";
		sprintf(tempString, "%.1f", context.flightState.glideRatio);
		return tempString;
	case WidgetContent_Vario_Active :
		sprintf(tempString, "%.1f", context.varioState.speedVertActive);
		return tempString;
	case WidgetContent_Vario_Lazy :		// 1s average
		sprintf(tempString, "%.1f", context.varioState.speedVertLazy);
		return tempString;
	case WidgetContent_DateTime :
		return "";
	case WidgetContent_Time_Current :
		if (context.varioState.timeCurrent == 0)
			strcpy(tempString, "--:--");
		else
			getTimeString(tempString, time(NULL));
		return tempString;
	case WidgetContent_Time_Flight :
		return getElapsedTimeString(tempString, context.flightState.flightTime);
	case WidgetContent_Time_Takeoff :
		return getTimeString(tempString, context.flightState.takeOffTime);

	case WidgetContent_Pressure :
		sprintf(tempString, "%.2f", context.varioState.pressure);
		return tempString;
	case WidgetContent_Temperature :
		sprintf(tempString, "%.1f", context.varioState.temperature);
		return tempString;
	case WidgetContent_Distance_Takeoff :
		sprintf(tempString, "%.1f", context.flightState.distTakeoff / 1000.0);
		return tempString;
	case WidgetContent_Distance_Flight :
		sprintf(tempString, "%.1f", context.flightState.distFlight / 1000.0);
		return tempString;
	case WidgetContent_Thermaling_Gain :
		return itoa(context.flightState.circlingGain, tempString, 10);
	case WidgetContent_Thermaling_Time :
		return getElapsedTimeString(tempString, context.flightState.circlingTime);

	case WidgetContent_Altitude_Max :
		return itoa(context.flightStats.altitudeMax + 0.5, tempString, 10);
	case WidgetContent_Altitude_Min :
		return itoa(context.flightStats.altitudeMin + 0.5, tempString, 10);
	case WidgetContent_ClimbRate_Max :
		sprintf(tempString, "%.1f", context.flightStats.varioMax);
		return tempString;
	case WidgetContent_SinkRate_Max :
		sprintf(tempString, "%.1f", context.flightStats.varioMin);
		return tempString;
	case WidgetContent_Total_Thermaling :
		return itoa(context.flightStats.totalThermaling, tempString, 10);
	case WidgetContent_Max_ThermalingGain :
		return itoa(context.flightStats.thermalingMaxGain, tempString, 10);

	case WidgetContent_Ground_Level :
		return itoa(context.varioState.altitudeGround + 0.5, tempString, 10);

	case WidgetContent_Battery :
		sprintf(tempString, "%.1fV", context.deviceState.batteryPower);
		return tempString;
	case WidgetContent_Title :
		return "Rascal";

	case WidgetContenxt_FlightState_deltaHeading_AVG :
		return itoa(context.flightState.deltaHeading_AVG, tempString, 10);
	case WidgetContenxt_FlightState_deltaHeading_SUM :
		return itoa(context.flightState.deltaHeading_SUM, tempString, 10);
	case WidgetContenxt_FlightState_glidingCount :
		return itoa(context.flightState.glidingCount, tempString, 10);
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

const char * VarioDisplay::getElapsedTimeString(char * str, time_t sec)
{
	sec = sec % (24 * 60 * 60); // trim date

	if (sec < 3600)
		sprintf(str, "%02d:%02d", sec / 60, sec % 60);
	else 
		sprintf(str, "%02d:%02d", sec / 3600, (sec / 60) % 60);

	return str;
}

const char * VarioDisplay::getTimeString(char * str, time_t t)
{
	struct tm * _tm = localtime(&t);
	sprintf(str, "%02d:%02d", _tm->tm_hour, _tm->tm_min);

	return str;
}


#include "SdCard.h"

static unsigned char BMP_Header[] = 
{
	// BMP Header
	0x42, 0x4D,				// ID field
	0xFE, 0x18, 0x00, 0x00, // Size of BMP file : (176+16)*264/8 + 54 + 8 = 6398 = 0x18FE
	0x00, 0x00, 0x00, 0x00,	// unused
	0x36, 0x00, 0x00, 0x00,	// Offset where bitmap data
	// DIB Header
	0x28, 0x00, 0x00, 0x00, // IDB Header size
	0xB0, 0x00, 0x00, 0x00,	// Width in pixels : 176
	0x08, 0x01, 0x00, 0x00, // Height in pixels : 264
	0x01, 0x00,				// Number of color planes
	0x01, 0x00,				// Number of bits per pixel
	0x00, 0x00, 0x00, 0x00,	// BI_RGB
	0xC0, 0x18, 0x00, 0x00, // Size of bitmap data : (176+16)*264/8 = 6336 = 0x18C0
	0x00, 0x00, 0x00, 0x00,	// Print resolution : horizontal
	0x00, 0x00, 0x00, 0x00,	// Print resolution : vertical
	0x00, 0x00, 0x00, 0x00,	// Number of colors in the palette
	0x00, 0x00, 0x00, 0x00,	// Number of important colors

	0x00, 0x00, 0x00, 0x00,	// BLACK
	0xFF, 0xFF, 0xFF, 0x00,	// WHITE
};

void VarioDisplay::saveScreenShot()
{
	//
	char name[32];

	for (int i = 1; i < 50; i++)
	{
		//
		sprintf(name, "/screen%02d.bmp", i);
		if (! SD_CARD.exists(name))
			break;
	}

	//
	File file = SD_CARD.open(name, FILE_WRITE);
	if (file)
	{
		//
		file.write(BMP_Header, sizeof(BMP_Header));

		//
		int x, y, row = (WIDTH / 8 + 3) / 4 * 4; // (176/8+3)/4*4
		unsigned char * ptr = _buffer;

		for (y = 0; y < HEIGHT; y++)
		{
			for (x = 0; x < WIDTH / 8; x++)
				file.write(*ptr++);
			for (; x < row; x++)
				file.write(0x00);
		}

		//
		file.close();

		Serial.printf("Save screenshot: %s\n", name);
	}
}
