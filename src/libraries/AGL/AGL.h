// AGL.h
//

#ifndef __AGL_H__
#define __AGL_H__

#include <Arduino.h>

#include "FS.h"
#include "SD_MMC.h"


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//

class AGL
{
public:
	AGL();
	~AGL();

public:
	int             getGroundLevel(double lat, double lon);

private:
	int             openFile(const char * name);
	void            closeFile();

	int             getGroundLevel(File * file, double lat, double lon);

private:
	int16_t	        mLat, mLon;
	File	        mFile;
};

#endif // __AGL_H__
