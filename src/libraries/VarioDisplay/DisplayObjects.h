// DisplayObjects.h
//

#include <Arduino.h>
#include "Widget.h"
#include "PrefItem.h"

#define MAX_WIDGETS		(16)
#define MAX_PREFITEMS	(16)
#define MAX_MEUITEMS	(16)


////////////////////////////////////////////////////////////////////////////////////////////////
// class VarioScreen

class VarioScreen
{
public:
	VarioScreen();
	
public:
	void				reset();
	void				set();
	
	Widget				widget[MAX_WIDGETS];
};


////////////////////////////////////////////////////////////////////////////////////////////////
// class VarioPreference

class VarioPreference
{
public:
	VarioPreference();
	
public:
	PrefItem			pref[MAX_PREFITEMS];
	
	volatile int16_t	topItem;
	volatile int16_t	itemCount;
	volatile int16_t	selItem;
};



////////////////////////////////////////////////////////////////////////////////////////////////
// class VarioPopup

class VarioPopup
{
public:
	VarioPopup();
};


////////////////////////////////////////////////////////////////////////////////////////////////
// class PopupMenu

class PopupMenu : public VarioPopup
{
public:
	PopupMenu();
	
public:
	MenuItem			items[MAX_MEUITEMS];
	
	volatile int16_t	topItem;
	volatile int16_t	itemCount;
	volatile int16_t	selItem;
};


////////////////////////////////////////////////////////////////////////////////////////////////
// class PopupMessageBox

class PopupMessageBox : public VarioPopup
{
public:
	PopupMessageBox();
};


////////////////////////////////////////////////////////////////////////////////////////////////
// class PopupListBox

class PopupListBox : public VarioPopup
{
public:
	PopupListBox();
};


////////////////////////////////////////////////////////////////////////////////////////////////
// class PopupCheckBox

class PopupCheckBox : public VarioPopup
{
public:
	PopupCheckBox();
};


////////////////////////////////////////////////////////////////////////////////////////////////
// class PopupRadioBox

class PopupRadioBox : public VarioPopup
{
public:
	PopupRadioBox();
};

