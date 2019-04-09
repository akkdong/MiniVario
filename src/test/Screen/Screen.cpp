// Screen.cpp
//

#include "Screen.h"

#include <Fonts/FreeSans6pt7b.h>
#include <Fonts/FreeSans8pt7b.h>
#include <Fonts/FreeSans9pt7b.h>
#include <Fonts/FreeSansBold6pt7b.h>
#include <Fonts/FreeSansBold8pt7b.h>
#include <Fonts/FreeSansBold9pt7b.h>

#include <Fonts/FreeSansBold12pt7b.h>
#include <Fonts/FreeSansBold16pt7b.h>
#include <Fonts/FreeSansBold18pt7b.h>
#include <Fonts/FreeSansBold24pt7b.h>
#include <Fonts/FreeSansBold26pt7b.h>

#define FONT_LABEL			&FreeSansBold8pt7b
#define FONT_UNIT			&FreeSans6pt7b
#define FONT_TEXT_BIG		&FreeSansBold24pt7b
#define FONT_TEXT_SMALL		&FreeSansBold16pt7b



////////////////////////////////////////////////////////////////////////////////////////////////
// class WidgetContentProvider

WidgetContentProvider::WidgetContentProvider(/*VarioData * data*/)
{
}

const char * WidgetContentProvider::getLabel(WidgetContentType type)
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
		return "Long";
	case WidgetContent_Latitude :
		return "Lat";
	case WidgetContent_Altitude_GPS :		// QNH
		return "Alt.G";
	case WidgetContent_Altitude_Baro :	// QNE
		return "Alt.B";
	case WidgetContent_Altitude_AGL :		// QFE
		return "AGL";
	case WidgetContent_Altitude_Ref1 :	// QFE
		return "Alt1";
	case WidgetContent_Altitude_Ref2 :	// QFE
		return "Alt2";
	case WidgetContent_Altitude_Ref3 :	// QFE
		return "Alt3";
	case WidgetContent_Glide_Ratio :
		return "LD";
	case WidgetContent_Vario_Active :
		return "Vario";
	case WidgetContent_Vario_Lazy :		// 1s average
		return "Vario";
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

const char * WidgetContentProvider::getUnit(WidgetContentType type)
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
		return "hPa";
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

const char * WidgetContentProvider::getString(WidgetContentType type)
{
	switch (type)
	{
	case WidgetContent_Speed_Ground :
		return "54";
	case WidgetContent_Speed_Air :
		return "";
	case WidgetContent_Heading :
		return "270";
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
	case WidgetContent_Altitude_GPS :		// QNH
		return "1520";
	case WidgetContent_Altitude_Baro :	// QNE
		return "";
	case WidgetContent_Altitude_AGL :		// QFE
		return "";
	case WidgetContent_Altitude_Ref1 :	// QFE
		return "";
	case WidgetContent_Altitude_Ref2 :	// QFE
		return "";
	case WidgetContent_Altitude_Ref3 :	// QFE
		return "";
	case WidgetContent_Glide_Ratio :
		return "";
	case WidgetContent_Vario_Active :
		return "4.2";
	case WidgetContent_Vario_Lazy :		// 1s average
		return "";
	case WidgetContent_DateTime :
		return "16:34";
	case WidgetContent_Time_Current :
		return "16:34";
	case WidgetContent_Time_Flight :
		return "2:54";
	case WidgetContent_Pressure :
		return "1013.34";
	case WidgetContent_Temperature :
		return "";
//	case WidgetContent_Thermal_Time :
//		return "";
//	case WidgetContent_Thermal_Gain :
//		return "";
	}
	
	return "";
}


	
	

////////////////////////////////////////////////////////////////////////////////////////////////
// class VarioScreen

VarioScreen::VarioScreen(EPaperDriver & _driver, WidgetContentProvider & _provider) 
	: EPaperDisplay(_driver)
	, activePage(0)
	, provider(_provider)
{

}

void VarioScreen::init()
{
	EPaperDisplay::init();
	
	// config pages <-- load page information from somewhere....
	//
	// below is just a test
	pages[0][0].setStyle(Widget_TextBox, WS_SMALL_FONT, WidgetContent_Speed_Ground);
	pages[0][0].setPosition(0, 24, 88, 52);

	pages[0][1].setStyle(Widget_TextBox, WS_SMALL_FONT, WidgetContent_Altitude_GPS);
	pages[0][1].setPosition(88, 24, 88, 52);

	pages[0][2].setStyle(Widget_TextBox, WS_BIG_FONT, WidgetContent_Vario_Active);
	pages[0][2].setPosition(0, 76, 88, 64);

	pages[0][5].setStyle(Widget_TextBox, WS_BIG_FONT, WidgetContent_Heading);
	pages[0][5].setPosition(88, 76, 88, 64);
	
	pages[0][3].setStyle(Widget_TextBox, WS_BIG_FONT, WidgetContent_Time_Flight);
	pages[0][3].setPosition(0, 140, 176, 64);

	pages[0][4].setStyle(Widget_TextBox, WS_BIG_FONT, WidgetContent_Pressure);
	pages[0][4].setPosition(0, 204, 176, 64);

	pages[0][6].setStyle(Widget_SimpleText, 0, WidgetContent_Time_Current);
	pages[0][6].setPosition(176-40, 0, 40, 24);
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
		
		Serial.print("draw widget: "); Serial.println(widget->style);
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
	const char * label = provider.getLabel(widget->getContentType());
	const char * unit = provider.getUnit(widget->getContentType());
	const char * string = provider.getString(widget->getContentType());
	
	Serial.println("drawBorder");
	drawBorder(widget);
	Serial.print("drawLabel: "); Serial.println(label);
	drawLabel(widget, label);
	Serial.print("drawUnit: "); Serial.println(unit);
	drawUnit(widget, unit);
	Serial.print("drawText: "); Serial.println(string);
	drawText(widget, string);
}

void VarioScreen::drawSimpleText(Widget * widget)
{
	const char * str = provider.getString(widget->getContentType());
	if (str && str[0])
	{
		int16_t x, y;
		uint16_t w, h;
		
		setFont(FONT_UNIT);
		getTextBounds(str, 0, 0, &x, &y, &w, &h);
		setCursor(widget->x + (widget->w - w) / 2, widget->y + (widget->h + h) / 2);
		setTextColor(COLOR_BLACK);
		print(str);
	}	
}

void VarioScreen::drawIcon(Widget * widget)
{
}

void VarioScreen::drawStatusBar(Widget * widget)
{
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
	#if 1
	drawFastHLine(widget->x, widget->y, widget->w, COLOR_BLACK);
//	drawFastHLine(widget->x, widget->y + widget->h, widget->w, COLOR_BLACK);
	drawFastVLine(widget->x, widget->y, widget->h, COLOR_BLACK);
//	drawFastVLine(widget->x + widget->w - 1, widget->y, widget->h, COLOR_BLACK);
	#else
	drawRect(widget->x, widget->y, widget->w, widget->h, COLOR_BLACK);
	#endif
}

void VarioScreen::drawLabel(Widget * widget, const char * str)
{
	if (str && str[0])
	{
		int16_t x, y;
		uint16_t w, h;
		
		setFont(FONT_LABEL);
		getTextBounds(str, 0, 0, &x, &y, &w, &h);
		setCursor(widget->x + 2, widget->y + 14);
		setTextColor(COLOR_BLACK);
		print(str);
	}
}

void VarioScreen::drawUnit(Widget * widget, const char * str)
{
	if (str && str[0])
	{
		int16_t x, y;
		uint16_t w, h;
		
		setFont(FONT_UNIT);
		getTextBounds(str, 0, 0, &x, &y, &w, &h);
		setCursor(widget->x + widget->w - w - 4, widget->y + 14);
		setTextColor(COLOR_BLACK);
		print(str);
	}
}

void VarioScreen::drawText(Widget * widget, const char * str)
{
	if (str && str[0])
	{
		int16_t x, y;
		uint16_t w, h;
		
		if (widget->extra & WS_BIG_FONT)
			setFont(FONT_TEXT_BIG);
		else
			setFont(FONT_TEXT_SMALL);
		getTextBounds(str, 0, 0, &x, &y, &w, &h);
		setCursor(widget->x + widget->w - w - 8, widget->y + widget->h - 6);
		setTextColor(COLOR_BLACK);
		print(str);
	}
}
