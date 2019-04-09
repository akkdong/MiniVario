// Screen.cpp
//

#include "Screen.h"

#include <Fonts/FreeMono8pt7b.h>
#include <Fonts/FreeMonoBold8pt7b.h>

#include <Fonts/FreeMonoBold12pt7b.h>
#include <Fonts/FreeMonoBold18pt7b.h>
#include <Fonts/FreeMonoBold24pt7b.h>



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
		return "Heading";
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
		return "Fly";
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
		return "";
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
		return "";
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
		return "";
	case WidgetContent_Time_Current :
		return "";
	case WidgetContent_Time_Flight :
		return "";
	case WidgetContent_Pressure :
		return "";
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
	pages[0][0].setPosition(0, 16, 176, 64);

	pages[0][1].setStyle(Widget_TextBox, WS_MID_FONT, WidgetContent_Altitude_GPS);
	pages[0][1].setPosition(0, 80, 176, 64);

	pages[0][2].setStyle(Widget_TextBox, WS_BIG_FONT, WidgetContent_Vario_Active);
	pages[0][2].setPosition(0, 144, 176, 64);
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
	#if 0
	drawFastHLine(widget->x, widget->y, widget->w, COLOR_BLACK);
	drawFastHLine(widget->x, widget->y + widget->h, widget->w, COLOR_BLACK);
	drawFastVLine(widget->x, widget->y, widget->h, COLOR_BLACK);
	drawFastVLine(widget->x + widget->w - 1, widget->y, widget->h, COLOR_BLACK);
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
		
		setFont(&FreeMonoBold8pt7b);
		getTextBounds(str, 0, 0, &x, &y, &w, &h);
		drawRect(widget->x + 2, widget->y + 2, w, h, COLOR_BLACK);
		setCursor(widget->x + 2, widget->y + h + 2);
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
		
		setFont(&FreeMono8pt7b);
		getTextBounds(str, 0, 0, &x, &y, &w, &h);
		drawRect(widget->x + widget->w - w - 4, widget->y + 2, w, h, COLOR_BLACK);
		setCursor(widget->x + widget->w - w - 4, widget->y + h + 2);
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
			setFont(&FreeMonoBold24pt7b);
		else if (widget->extra & WS_MID_FONT)
			setFont(&FreeMonoBold18pt7b);
		else
			setFont(&FreeMonoBold12pt7b);			
		getTextBounds(str, 0, 0, &x, &y, &w, &h);
		drawRect(widget->x + widget->w - w - 8, widget->y + widget->h - h - 4, w, h, COLOR_BLACK);
		setCursor(widget->x + widget->w - w - 8, widget->y + widget->h - 4);
		setTextColor(COLOR_BLACK);
		print(str);
	}
}
