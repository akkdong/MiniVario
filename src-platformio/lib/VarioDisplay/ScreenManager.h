// ScreenManager.h
//

#ifndef __SCREENMANAGER_H__
#define __SCREENMANAGER_H__

#include <Arduino.h>
#include "DisplayObjects.h"
#include "VarioDisplay.h"

////////////////////////////////////////////////////////////////////////////////////////////////
// class ScreenManager

class ScreenManager
{
public:
	ScreenManager();

public:
    void                loadScreens();

    void                showActiveScreen(VarioDisplay & display);
    void                showCirclingScreen(VarioDisplay & display);
    void                showStatisticScreen(VarioDisplay & display);

    void                showNextActiveScreen(VarioDisplay & display);
    void                showPrevActiveScreen(VarioDisplay & display);

    VarioScreen *       getActiveScreen();
    
private:
    void                loadScreen(VarioScreen * screen, uint32_t id, WidgetData * widgets);
    void                showScreen(VarioDisplay & display, int screen);
    int                 findScreen(uint32_t id);    

public:
	VarioScreen			screens[MAX_SCREENS];

	int					maxScreen;
	int					curScreen;
    int                 actScreen;
};





#endif // __SCREENMANAGER_H__
