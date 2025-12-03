// MenuItem.h
//

#ifndef __MENUITEM_H__
#define __MENUITEM_H__

#include <Arduino.h>


////////////////////////////////////////////////////////////////////////////////////////////////
// 

enum _TopMenuItems
{
	TMID_SHOW_PREFERENCE = 0x5001,
	TMID_TOGGLE_WEBSERVICE = 0x6001,
	TMID_TOGGLE_SOUND = 0x6002,
	TMID_TOGGLE_BLUETOOTH = 0x6003,
	TMID_RESET_DEVICE = 0x7001,
	TMID_POWER_OFF = 0x7002,
	TMID_SIMULATION_MODE = 0x8000
};

typedef struct _MenuItem
{
	uint16_t	itemId;
	uint16_t	strId;

} MenuItem;


#endif // __MENUITEM_H__
