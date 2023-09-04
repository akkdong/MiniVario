// DisplayObjects.cpp
//

#include "DisplayObjects.h"
#include "DeviceDefines.h"


////////////////////////////////////////////////////////////////////////////////////////////////
// class VarioScreen

VarioScreen::VarioScreen() : DisplayObject(DispObject_Screen), activeWidget(0)
{
}

uint32_t VarioScreen::processKey(uint8_t key)
{
	switch (key)
	{
	case KEY_UP :
		// move to previous page
		return CMD_SHOW_PREV_PAGE;
	case KEY_DOWN :
		// move to next page
		return CMD_SHOW_NEXT_PAGE;
	
	case KEY_UP_LONG :
		return CMD_SAVE_SCREENSHOT;

	case KEY_DOWN_LONG :
		return CMD_TOOGLE_SOUND;
		
	case KEY_SEL_LONG :
		return CMD_SHOW_TOP_MENU;
	}
	
	return 0;
}

Widget * VarioScreen::getWidget(size_t index)
{
	if (index < activeWidget)
		return &widget[index];

	return NULL;
}

int VarioScreen::addWidget(WidgetData * data)
{
	if (activeWidget < MAX_WIDGETS)
	{
		Widget * _widget = &widget[activeWidget];

		_widget->setStyle(data->style, data->extraStyle, data->type);
		_widget->setPosition(data->x, data->y, data->w, data->h);

		return ++activeWidget;
	}

	return -1;
}


////////////////////////////////////////////////////////////////////////////////////////////////
// class VarioPreference

VarioPreference::VarioPreference() : DisplayObject(DispObject_Screen)
{
}


////////////////////////////////////////////////////////////////////////////////////////////////
// class PopupTopMenu

PopupTopMenu::PopupTopMenu() 
	: VarioPopup(DispObject_PopupTopMenu)
	, itemCount(0)
	, itemTop(0)
	, itemSelect(0)
{
}

int16_t PopupTopMenu::addItem(uint16_t itemId, uint16_t strId)
{
	if (itemCount < MAX_MEUITEMS)
	{
		items[itemCount].itemId = itemId;
		items[itemCount].strId = strId;
		
		return ++itemCount;
	}
	
	return -1;
}

uint32_t PopupTopMenu::processKey(uint8_t key)
{
	switch (key)
	{
	case KEY_UP :
		// previous item
		itemSelect = (itemSelect == 0) ? (itemCount - 1) : (itemSelect - 1);
		break;
	case KEY_DOWN :
		// next item
		itemSelect = itemSelect + 1;
		if (itemSelect == itemCount)
			itemSelect = 0;
		break;
		
	case KEY_SEL :
		return MAKE_LRESULT(CMD_LEAVE_TOPMENU, items[itemSelect].itemId);
		break;
	case KEY_SEL_LONG :
		return MAKE_LRESULT(CMD_LEAVE_TOPMENU, 0x0000);
	}
	
	return 0;
}


////////////////////////////////////////////////////////////////////////////////////////////////
// class PopupMessageBox

PopupMessageBox::PopupMessageBox(uint32_t type, const char * msg) : VarioPopup(DispObject_PopupMessageBox)
{
}


////////////////////////////////////////////////////////////////////////////////////////////////
// class PopupListBox

PopupListBox::PopupListBox() : VarioPopup(DispObject_PopupListBox)
{
}


////////////////////////////////////////////////////////////////////////////////////////////////
// class PopupCheckBox

PopupCheckBox::PopupCheckBox() : VarioPopup(DispObject_PopupCheckBox)
{
}


////////////////////////////////////////////////////////////////////////////////////////////////
// class PopupRadioBox

PopupRadioBox::PopupRadioBox() : VarioPopup(DispObject_PopupRadioBox)
{
}

////////////////////////////////////////////////////////////////////////////////////////////////
// class PopupWebService

PopupWebService::PopupWebService() 
	: VarioPopup(DispObject_PopupWebService)
{
}

uint32_t PopupWebService::processKey(uint8_t key)
{
	switch (key)
	{
	case KEY_SEL_LONG :
		return MAKE_LRESULT(CMD_LEAVE_WEBSERVICE, 0x0000);
	}
	
	return 0;
}