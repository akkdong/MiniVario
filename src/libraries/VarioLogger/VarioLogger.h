// VarioLogger.h
//

#ifndef __VARIOLOGGER_H__
#define __VARIOLOGGER_H__

#include <Arduino.h>
#include <time.h>

#include "DeviceDefines.h"
#include "FixedLenDigit.h"
#include "FS.h"
#include "SdCard.h"


/////////////////////////////////////////////////////////////////////////////
// class VarioLogger

class VarioLogger
{
public:
	VarioLogger();
	
public:
	bool				begin(time_t date); // date = seconds since 1900
	void				end(time_t date);
	
	int					write(uint8_t ch);

	void				updateBaroAltitude(float varioAlt);
	
	int					isInitialized();
	int					isLogging();

private:
	void				reset();
	
	const char *		makeFileName(char * buf, time_t date);
	void				writeHeader(time_t date);

private:
	File				root;
	File				file;
	
	uint8_t				logState;
	int					columnCount;
	
	float				varioAltitude;
	FixedLenDigit		digit;
};

#endif // __VARIOLOGGER_H__
