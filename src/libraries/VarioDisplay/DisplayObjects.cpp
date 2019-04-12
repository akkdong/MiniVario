// DisplayObjects.cpp
//

#include "DisplayObjects.h"



////////////////////////////////////////////////////////////////////////////////////////////////
// class VarioScreen

VarioScreen::VarioScreen() : DisplayObject(DispObject_Screen)
{
}


////////////////////////////////////////////////////////////////////////////////////////////////
// class VarioPreference

VarioPreference::VarioPreference() : DisplayObject(DispObject_Screen)
{
}


////////////////////////////////////////////////////////////////////////////////////////////////
// class PopupMenu

PopupMenu::PopupMenu() 
	: VarioPopup(DispObject_PopupMenu)
	, itemCount(0)
	, itemTop(0)
	, itemSelect(0)
{
}

int16_t PopupMenu::addItem(uint16_t itemId, uint16_t strId)
{
	if (itemCount < MAX_MEUITEMS)
	{
		items[itemCount].itemId = itemId;
		items[itemCount].strId = strId;
		
		return ++itemCount;
	}
	
	return -1;
}

uint32_t PopupMenu::processKey(uint8_t key)
{
}


////////////////////////////////////////////////////////////////////////////////////////////////
// class PopupMessageBox

PopupMessageBox::PopupMessageBox() : VarioPopup(DispObject_PopupMessageBox)
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
