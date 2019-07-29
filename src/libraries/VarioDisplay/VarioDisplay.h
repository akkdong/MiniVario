// VarioDisplay.h
//

#ifndef __VARIODISPLAY_H__
#define __VARIODISPLAY_H__

#include <Arduino.h>
#include "EPaperDisplay.h"
#include "DeviceContext.h"
#include "Task.h"
#include "DisplayObjects.h"


#define MAX_FONTS				(8)
#define MAX_TEMP_STRLEN			(16)

#define STATUSBAR_WIDTH			(width())
#define	STATUSBAR_HEIGHT		(24)
#define STATUSBAR_TEXT_WIDTH	(40)

#define WORKAREA_X				(0)
#define WORKAREA_Y				(STATUSBAR_HEIGHT)
#define WORKAREA_W				(width())
#define WORKAREA_H				(height() - STATUSBAR_HEIGHT)


typedef const GFXfont * GFXfontPtr;



////////////////////////////////////////////////////////////////////////////////////////////////
// class VarioDisplayDriver

class VarioDisplayDriver : public EPaper_Waveshare270
{
public:
	VarioDisplayDriver(PinSetting * pins) : EPaper_Waveshare270(pins) {
	}

	virtual void _delay(int msec) { 
		const TickType_t xDelay = msec / portTICK_PERIOD_MS;
		vTaskDelay(xDelay);
	}
};


////////////////////////////////////////////////////////////////////////////////////////////////
// class VarioDisplay

class VarioDisplay : public EPaperDisplay, public Task
{
public:
	VarioDisplay(EPaperDriver & _driver, DeviceContext & _context);

public:
	int 				begin(bool confirmWakeup);
	int					beginFirmwareUpdate();
	void				end();
	
	void				attachScreen(VarioScreen * screen);
	void				attachPreference(VarioPreference * pref);
	void				showPopup(VarioPopup * popupPtr);

	void				saveScreenShot();

	void				deepSleep() { displayMode = _DEEPSLEEP; }
	void				wakeupConfirmed() { displayMode = _VARIO; }
	
	DisplayObject *		getActiveObject();

protected:	
	// 	
	void				init();	
	
	void 				update();
	void				updateFirmwareUpdate();

	//	
	void				draw(VarioScreen * screen);
	void				draw(VarioPreference * pref);
	void				draw(VarioPopup * popup);
	void				draw(PopupTopMenu * menu);
	void				draw(PopupWebService * service);
	void 				draw(Widget * widget);

	void 				drawEmptyArea(Widget * widget);
	void 				drawTextBox(Widget * widget);
	void				drawSimpleText(Widget * widget);
	void 				drawIcon(Widget * widget);
	void 				drawStatusBar(Widget * widget);
	void 				drawVarioHistory(Widget * widget);
	void				drawTrackHistory(Widget * widget);
	void 				drawVarioBar(Widget * widget);
	void 				drawCompass(Widget * widget);
	void				drawTriangle(int16_t cx, int16_t cy, int16_t r, int16_t heading);

	void				drawBorder(Widget * widget);
	void				drawText(const char * str, int16_t x, int16_t y, uint16_t w, uint16_t h, uint32_t style, uint16_t color);

	void				drawPrefItem(PrefItem * item, int16_t x, int16_t y, int16_t w, int16_t h, bool selected);

	void				drawLogoScreen();
	void				drawConfirmMessage();

	//	
	const char *		getLabel(WidgetContentType type);
	const char *		getUnit(WidgetContentType type);
	const char *		getString(WidgetContentType type);
	
	//
	void				sleepDevice();
	
protected:
	void 				TaskProc();

private:
	const char *		makeTimeString(char * str, time_t sec);
	
protected:
	static GFXfontPtr 	__FontStack[MAX_FONTS];
	
protected:
	//
	Widget				statusBar;
	
	VarioScreen *		activeScreen;
	VarioPreference *	activePref;
	VarioPopup *		activePopup;	
	
	//
	DeviceContext & 	context;
	
private:
	char				tempString[MAX_TEMP_STRLEN];

	enum _DisplayMode
	{
		_CONFIRM,
		_VARIO,
		_DEEPSLEEP,
		_UPDATE
	};

	volatile int		displayMode;
	volatile uint32_t	updateCount;
};


#endif // __VARIODISPLAY_H__
