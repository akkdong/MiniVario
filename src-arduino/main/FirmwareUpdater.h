// FirmwareUpdater.h
//

#ifndef __FIRMWAREUPDATER_H__
#define __FIRMWAREUPDATER_H__

#include <Arduino.h>
#include "SdCard.h"
#include "VarioDisplay.h"


////////////////////////////////////////////////////////////////////////////////
// class FirmwareUpdater

class FirmwareUpdater
{
public:
    FirmwareUpdater();

public:
    bool checkUpdate();
    bool performUpdate(VarioDisplay & display);

private:
    File bin;
};


#endif // __FIRMWAREUPDATE_H__