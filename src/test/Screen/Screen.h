// Screen.h
//

#ifndef __SCREEN_H__
#define __SCREEN_H__

#include <Arduino.h>
#include <EPaperDisplay.h>


////////////////////////////////////////////////////////////////////////////////////////////////
//

#define MAX_PAGES			5
#define MAX_WIDGETS			16


////////////////////////////////////////////////////////////////////////////////////////////////
// Widget_TextBox extra data

#define WS_TA_LEFT			0x0000 // WidgetStyle_TextAlign: default LEFT, MIDDLE
#define WS_TA_RIGHT			0x0001
#define WS_TA_CENTER		0x0002
#define WS_TA_MIDDLE		0x0000
#define WS_TA_TOP			0x0004
#define WS_TA_BOTTOM		0x0008
#define WS_LA_LEFT			0x0000 // WidgetStyle_LabelAlign: default LEFT
#define WS_LA_RIGHT			0x0010
#define WS_LA_CENTER		0x0020
#define WS_HAS_LABEL		0x0100
#define WS_HAS_BOARDER		0x0200
#define WS_SMALL_FONT		0x0000
#define WS_MID_FONT			0x1000
#define WS_BIG_FONT			0x2000
#define WS_OWNERDRAW		0x8000 // WidgetStyle_OwnerDraw



////////////////////////////////////////////////////////////////////////////////////////////////
//

typedef enum _WidgetStyle
{
	Widget_Empty,
	
	Widget_TextBox,
	Widget_Icon,
	Widget_StatusBar,
	Widget_VarioBar,
	Widget_VarioHistory,
	Widget_Compass,
	
} WidgetStyle;

	
typedef enum _WidgetContentType
{
	// Empty area
    WidgetContent_Empty,
	
	// Text Box
	WidgetContent_Speed_Ground,
	WidgetContent_Speed_Air,
	
	WidgetContent_Heading,
//	WidgetContent_Heading_GPS,
//	WidgetContent_Heading_Compass,
//	WidgetContent_Bearing,
	
	WidgetContent_Longitude,
	WidgetContent_Latitude,
	
	WidgetContent_Altitude_GPS,		// QNH
	WidgetContent_Altitude_Baro,	// QNE
	WidgetContent_Altitude_AGL,		// QFE
	WidgetContent_Altitude_Ref1,	// QFE
	WidgetContent_Altitude_Ref2,	// QFE
	WidgetContent_Altitude_Ref3,	// QFE
	
	WidgetContent_Glide_Ratio,
	
	WidgetContent_Vario_Active,
	WidgetContent_Vario_Lazy,		// 1s average
	
	WidgetContent_DateTime,
	WidgetContent_Time_Current,
	WidgetContent_Time_Flight,
	
	WidgetContent_Pressure,
	WidgetContent_Temperature,
	
//	WidgetContent_Thermal_Time
//	WidgetContent_Thermal_Gain
	
	// Icon
	WidgetContent_Status_GPS,
	WidgetContent_Status_Bluetooth,
	WidgetContent_Status_Battery,

	// Status bar
	WidgetContent_Status_Bar,
	
	// Vario Bar
	WidgetContent_Vario_Bar,
	
	// Vario History
	WidgetContent_Vario_History,
	
	// Thermal Assist
	WidgetContent_Thermal_Assist
	
} WidgetContentType;


////////////////////////////////////////////////////////////////////////////////////////////////
//

class Widget;
class WidgetContentProvider;
class VarioScreen;


////////////////////////////////////////////////////////////////////////////////////////////////
// class Widget

class Widget
{
	friend class WidgetContentProvider;
	friend class VarioScreen;
	
public:
	Widget() : style(Widget_Empty) {}
	
public:
	void				setStyle(WidgetStyle _style, uint32_t _extra, WidgetContentType _type);
	void				setPosition(uint16_t _x, uint16_t _y, uint16_t _w, uint16_t _h);

	WidgetStyle			getStyle();
	WidgetContentType	getContentType();
	uint32_t			getExtraData();

	uint16_t			getX();
	uint16_t			getY();
	uint16_t			getWidth();
	uint16_t			getHeight();
	
protected:
	WidgetStyle			style;
	WidgetContentType	type;
	uint32_t			extra; 	// style specific extra data
	
	uint16_t			x, y, w, h;
};

inline void Widget::setStyle(WidgetStyle _style, uint32_t _extra, WidgetContentType _type)
	{ style = _style; extra = _extra; type = _type; }
	
inline void Widget::setPosition(uint16_t _x, uint16_t _y, uint16_t _w, uint16_t _h)
	{ x = _x; y = _y; w = _w; h = _h; }

inline WidgetStyle Widget::getStyle()
	{ return style; }

inline WidgetContentType Widget::getContentType()
	{ return type; }
	
inline uint32_t Widget::getExtraData()
	{ return extra; }

inline uint16_t Widget::getX()
	{ return x; }
	
inline uint16_t Widget::getY()
	{ return y; }
	
inline uint16_t Widget::getWidth()
	{ return w; }
	
inline uint16_t Widget::getHeight()	
	{ return h; }
	

////////////////////////////////////////////////////////////////////////////////////////////////
// class WidgetContentProvider

class WidgetContentProvider
{
public:
	WidgetContentProvider(/*VarioData * data*/);
	
public:
	const char *	getLabel(WidgetContentType type);
	const char *	getUnit(WidgetContentType type);
	const char *	getString(WidgetContentType type);	
};
	

////////////////////////////////////////////////////////////////////////////////////////////////
// class VarioScreen

class VarioScreen : public EPaperDisplay
{
public:
	VarioScreen(EPaperDriver & _driver, WidgetContentProvider & _provider);

public:
	void			init();
	
	void 			update();
	
protected:
	void 			draw(Widget * widget);
	
	void 			drawEmptyArea(Widget * widget);
	void 			drawTextBox(Widget * widget);
	void 			drawIcon(Widget * widget);
	void 			drawStatusBar(Widget * widget);
	void 			drawVarioHistory(Widget * widget);
	void 			drawVarioBar(Widget * widget);
	void 			drawCompass(Widget * widget);
	
	void			drawBorder(Widget * widget);
	void			drawLabel(Widget * widget, const char * str);
	void			drawUnit(Widget * widget, const char * str);
	void			drawText(Widget * widget, const char * str);
	
protected:
	Widget			pages[MAX_PAGES][MAX_WIDGETS];
	int				activePage;
	
	WidgetContentProvider & provider;
};


#endif // __SCREEN_H__
