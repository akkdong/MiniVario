// ScreenManager.h
//

#ifndef __SCREENMANAGER_H__
#define __SCREENMANAGER_H__

#include <Arduino.h>
#include "DisplayObjects.h"


////////////////////////////////////////////////////////////////////////////////////////////////
// class ScreenManager

class ScreenManager
{
public:
	ScreenManager();

public:
    void                loadScreens();

    VarioScreen *       getActiveScreen();
    VarioScreen *       getNextScreen();
    VarioScreen *       getPrevScreen();
    
private:
    void                loadScreen(VarioScreen * screen, uint32_t id, WidgetData * widgets);

public:
	VarioScreen			screens[MAX_SCREENS];

	int					maxScreen;
	int					curScreen;
};





#endif // __SCREENMANAGER_H__
