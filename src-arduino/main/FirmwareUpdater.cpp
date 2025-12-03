// FirmwareUpdater.cpp
//

#include "FirmwareUpdater.h"
#include "TaskWatchdog.h"
#include "Update.h"

#define FILE_FIRMWARE       "/mini-vario.bin"


////////////////////////////////////////////////////////////////////////////////
// class FirmwareUpdater

FirmwareUpdater::FirmwareUpdater()
{

}

bool FirmwareUpdater::checkUpdate()
{
	bin = SD_CARD.open(FILE_FIRMWARE);

	// is it valid & not directory & not empty ?
	if (bin && ! bin.isDirectory() && bin.size() > 0)
    {
        Serial.printf("[FU] Firmware exist!\n");
        return true;
    }

    bin.close();

    return false;
}

bool FirmwareUpdater::performUpdate(VarioDisplay & display)
{
    //TaskWatchdog::remove(NULL);
    display.beginFirmwareUpdate();

    Serial.printf("[FU] Start firmware updating\n");
    if (Update.begin(bin.size()))
    {
        size_t written = Update.writeStream(bin);
        Serial.printf("[FU] Total %d, written %d\n", bin.size(), written);
       
        if (! Update.end())
            Serial.printf("[FU] Error occurred: #%d\n", Update.getError());
        else
            Serial.printf("[FU] Update successfully completed!\n");

       SD_CARD.remove(FILE_FIRMWARE);

       return true;
    }
    else
    {
        Serial.printf("[FU] Not enough space to begin OTA");
    }

    return false;
}