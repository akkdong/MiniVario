// SdCard.cpp
//

#include "SdCard.h"


/////////////////////////////////////////////////////////////////////////////
// class SdCard

SdCard::SdCard() : mState(-1)
{
}

int SdCard::begin()
{
    if (mState < 0)
    {
        mState = 1;

        if (! SD_MMC.begin() || SD_MMC.cardType() == CARD_NONE)
            mState = 0;
    }

    return mState;
}

File SdCard::open(const char* path, const char* mode)
{
    if (valid())
        return SD_MMC.open(path, mode);

    return File();
}

bool SdCard::exists(const char* path)
{
    if (valid())
        return SD_MMC.exists(path);

    return false;
}

bool SdCard::remove(const char* path)
{
    if (valid())
        return SD_MMC.remove(path);

    return false;
}

bool SdCard::rename(const char* pathFrom, const char* pathTo)
{
    if (valid())
        return SD_MMC.rename(pathFrom, pathTo);

    return false;
}

bool SdCard::mkdir(const char *path)
{
    if (valid())
        return SD_MMC.mkdir(path);

    return false;
}

bool SdCard::rmdir(const char *path)
{
    if (valid())
        return SD_MMC.rmdir(path);

    return false;
}


/////////////////////////////////////////////////////////////////////////////
//

SdCard  SD_CARD;