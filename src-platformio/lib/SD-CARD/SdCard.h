// SdCard.h
//

#ifndef __SDCARD_H__
#define __SDCARD_H__

#include <Arduino.h>
#include <SD_MMC.h>


/////////////////////////////////////////////////////////////////////////////
// class SdCard

class SdCard
{
public:
    SdCard();

public:
    int             begin();
    void            end();

    bool            valid() { return mState > 0 ? true : false; }

    File            open(const char* path, const char* mode = FILE_READ);
    bool            exists(const char* path);
    bool            remove(const char* path);
    bool            rename(const char* pathFrom, const char* pathTo);
    bool            mkdir(const char *path);
    bool            rmdir(const char *path);

private:
    int             mState;
};


/////////////////////////////////////////////////////////////////////////////
//

extern SdCard  SD_CARD;


#endif // __SDCARD_H__
