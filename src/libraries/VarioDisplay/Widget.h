// Widget.h
//

#ifndef __WIDGET_H__
#define __WIDGET_H__

#include <Arduino.h>


////////////////////////////////////////////////////////////////////////////////////////////////
// Widget_TextBox extraStyle
//
// extraStyle -> unsigned 32 bits : 0x00000000
//  box-style : 0xF0000000
//		0x80000000 -> draw left line
//		0x40000000 -> draw top line
//		0x20000000 -> draw right line
//		0x10000000 -> draw bottom line
//	font : 0x0000000F

#define WS_BOX_MASK			0xF0000000
#define WS_BORDER_LEFT		0x80000000	// box-style: has left line
#define WS_BORDER_TOP		0x40000000	// box-style: has top line
#define WS_BORDER_RIGHT		0x20000000	// box-style: has right line
#define WS_BORDER_BOTTOM	0x10000000	// box-style: has bottom line

#define WS_HAS_LABEL		0x01000000
#define WS_HAS_UNIT			0x02000000

#define WS_TA_LEFT			0x00000000 // WidgetStyle_TextAlign: default LEFT, BOTTOM
#define WS_TA_RIGHT			0x00010000
#define WS_TA_CENTER		0x00020000
#define WS_TA_TOP			0x00040000
#define WS_TA_MIDDLE		0x00080000
#define WS_TA_BOTTOM		0x00000000

#define WS_FONT_MASK		0x0000000F
#define WS_FONT_DEFAULT		0x00000001
#define WS_FONT_NORMAL_1	0x00000000	// MonoSerif 6pt
#define WS_FONT_NORMAL_2	0x00000001	// MonoSerif 8pt
#define WS_FONT_NORMAL_3	0x00000002	// MonoSerif 16pt
#define WS_FONT_NORMAL_4	0x00000003	// MonoSerif 24pt
#define WS_FONT_BOLD_1		0x00000004	// MonoSerifBold 6pt
#define WS_FONT_BOLD_2		0x00000005	// MonoSerifBold 8pt
#define WS_FONT_BOLD_3		0x00000006	// MonoSerifBold 16pt
#define WS_FONT_BOLD_4		0x00000007	// MonoSerifBold 24pt



////////////////////////////////////////////////////////////////////////////////////////////////
//

typedef enum _WidgetStyle
{
	Widget_Empty,
	
	Widget_TextBox,
	Widget_SimpleText,
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
	WidgetContent_Bearing_Takeoff,
	
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

	//
	WidgetContent_Distance_Takeoff,
	WidgetContent_Distance_Flight,

	WidgetContent_Thermaling_Gain,
	WidgetContent_Thermaling_Time,
	WidgetContent_Thermaling_Slope, // thermal incline (degree)

	//
	WidgetContent_Altitude_Max,
	WidgetContent_Altitude_Min,
	WidgetContent_ClimbRate_Max,
	WidgetContent_SinkRate_Max,

	//
	WidgetContent_Ground_Level,

	// Simple Text
	WidgetContent_Title,
	WidgetContent_Battery,
	
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
	WidgetContent_Thermal_Assist,

	// for debugging
	WidgetContenxt_FlightState_deltaHeading_AVG,
	WidgetContenxt_FlightState_deltaHeading_SUM,
	WidgetContenxt_FlightState_glidingCount,

	
} WidgetContentType;



////////////////////////////////////////////////////////////////////////////////////////////////
//

class Widget;
class VarioDisplay;



////////////////////////////////////////////////////////////////////////////////////////////////
// class Widget

class Widget
{
	friend class VarioDisplay;
	
public:
	Widget() : style(Widget_Empty) {}
	
public:
	void				setStyle(WidgetStyle _style, uint32_t _extraStyle, WidgetContentType _type);
	void				setPosition(uint16_t _x, uint16_t _y, uint16_t _w, uint16_t _h);

	WidgetStyle			getStyle();
	uint32_t			getExtraStyle();
	WidgetContentType	getContentType();

	uint16_t			getX();
	uint16_t			getY();
	uint16_t			getWidth();
	uint16_t			getHeight();
	
protected:
	WidgetStyle			style;
	uint32_t			extraStyle;
	WidgetContentType	type;
	
	uint16_t			x, y, w, h;
};

inline void Widget::setStyle(WidgetStyle _style, uint32_t _extraStyle, WidgetContentType _type)
	{ style = _style; extraStyle = _extraStyle; type = _type; }
	
inline void Widget::setPosition(uint16_t _x, uint16_t _y, uint16_t _w, uint16_t _h)
	{ x = _x; y = _y; w = _w; h = _h; }

inline WidgetStyle Widget::getStyle()
	{ return style; }

inline uint32_t Widget::getExtraStyle()
	{ return extraStyle; }

inline WidgetContentType Widget::getContentType()
	{ return type; }
	
inline uint16_t Widget::getX()
	{ return x; }
	
inline uint16_t Widget::getY()
	{ return y; }
	
inline uint16_t Widget::getWidth()
	{ return w; }
	
inline uint16_t Widget::getHeight()	
	{ return h; }
	

#endif // __WIDGET_H__
