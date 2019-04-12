// VarioScreen.h
//

#ifndef __SCREEN_H__
#define __SCREEN_H__

#include <Arduino.h>
#include "Widget.h"
#include "EPaperDisplay.h"
#include "DeviceContext.h"
#include "Task.h"

#define MAX_PAGES			5
#define MAX_WIDGETS			16


////////////////////////////////////////////////////////////////////////////////////////////////
//

class VarioEPaper : public EPaper_Waveshare270
{
public:
	VarioEPaper(PinSetting * pins) : EPaper_Waveshare270(pins) {
	}

	virtual void _delay(int msec) { 
		const TickType_t xDelay = msec / portTICK_PERIOD_MS;
		vTaskDelay(xDelay);
	}
};


////////////////////////////////////////////////////////////////////////////////////////////////
// class VarioScreen

class VarioScreen : public EPaperDisplay, public Task
{
public:
	VarioScreen(EPaperDriver & _driver, DeviceContext & _context);

public:
	int 			begin();
	void			end();
	
	void			deepSleep() { assertSleep = true; }
	
protected:
	// 
	void			init();	
	void 			update();
	
	//
	void 			draw(Widget * widget);
	
	void 			drawEmptyArea(Widget * widget);
	void 			drawTextBox(Widget * widget);
	void			drawSimpleText(Widget * widget);
	void 			drawIcon(Widget * widget);
	void 			drawStatusBar(Widget * widget);
	void 			drawVarioHistory(Widget * widget);
	void 			drawVarioBar(Widget * widget);
	void 			drawCompass(Widget * widget);
	
	void			drawBorder(Widget * widget);
	void			drawText(const char * str, int16_t x, int16_t y, uint16_t w, uint16_t h, uint32_t style, uint16_t color);
	
	//
	const char *	getLabel(WidgetContentType type);
	const char *	getUnit(WidgetContentType type);
	const char *	getString(WidgetContentType type);
	
	//
	void			sleepDevice();
	
protected:
	void 			TaskProc();

	
protected:
	Widget			pages[MAX_PAGES][MAX_WIDGETS];
	int				activePage;
	
	DeviceContext &	context;
	
	static const GFXfont * __FontStack[8];
	
private:
	char			tempString[16];
	volatile bool	assertSleep;
};


#endif // __SCREEN_H__
