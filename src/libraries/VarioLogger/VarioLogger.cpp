// VarioLogger.cpp
//

#include "DeviceContext.h"
#include "NmeaParserEx.h"
#include "VarioLogger.h"


#define CLEAR_STATE()		logState = 0
#define SET_STATE(bit)		logState |= (bit)
#define UNSET_STATE(bit)	logState &= ~(bit)

#define IS_SET(bit)			(logState & (bit)) == (bit)


#define LOGGER_INIT_FAILED		(1 << 0)
#define LOGGER_WORKING			(1 << 1)


const char * nameManufacture = "MIN";
const char * serialNumber = "XXX";
const char * tailOfFileName = "01.igc";

const char * logsFolder = "/TrackLogs";

char pathBuf[64];


// ANRCSTM Variometer & GPS Loggger
// HFDTExxxx
// HFPLTPILOT:xxxxxx
// HFCCLCOMPETITION CLASS:FAI-3 (PG)
// HFGTYGLIDERTYPE:xxxxx
// HFDTM100GPSDATUM:WGS-84
const char * igcHeader[] =
{
	"ANRC Variometer & GPS Loggger v2", 
	"\r\nHFDTE",					// ex) DDMMYY
	NULL,
	"\r\nHFPLTPILOT:",				// ex) akkdong
	NULL,
	"\r\nHFCCLCOMPETITION CLASS:",	// ex) FAI-3 (PG)
	NULL,
	"\r\nHFGTYGLIDERTYPE:", 		// ex) NIVIUK IcePeak6  {MANUFACTURE} {MODEL}
	NULL,
	"\r\nHFDTM100GPSDATUM:WGS-84",
	NULL,
	"\r\n"
};

#define IGC_HEADER_DATE			(2)
#define IGC_HEADER_PILOT		(4)
#define IGC_HEADER_CLASS		(6)
#define IGC_HEADER_GLIDER		(8)
#define IGC_HEADER_GPSDATUM		(10)


/////////////////////////////////////////////////////////////////////////////
// class VarioLogger

VarioLogger::VarioLogger()
{
	reset();
}

bool VarioLogger::begin(time_t date)
{
	if (! SD_CARD.valid())
	{
		SET_STATE(LOGGER_INIT_FAILED);
		return false;
	}

	// 
	if (! SD_CARD.exists(logsFolder))
	{
		if (! SD_CARD.mkdir(logsFolder))
		{
			SET_STATE(LOGGER_INIT_FAILED);
			return false;
		}
	}	
	
	// create new file // YYYY-MM-DD-NRC-STM-nn.igc
	char name[26];
	
	if (! makeFileName(name, date))
	{
		// no valid file room
		SET_STATE(LOGGER_INIT_FAILED);
		return false;
	}

	strcpy(pathBuf, logsFolder);
	strcat(pathBuf, "/");
	strcat(pathBuf, name);

	if ((file = SD_CARD.open(pathBuf, FILE_WRITE)))
	{
		//
		SET_STATE(LOGGER_WORKING);
		
		// write header
		writeHeader(date);
		
		return true;
	}
	
	return false;
}

void VarioLogger::end(time_t date)
{
	if (! IS_SET(LOGGER_WORKING))
		return;
	
	// if (date)
	// {
	// 	time_t localdate = date + (__DeviceContext.logger.timezone * 60 * 60); 
	// 	struct tm * _tm = localtime(&localdate);	
	//	
	// 	file.timestamp(T_ACCESS|T_WRITE,
	// 					_tm->tm_year + 1900, _tm->tm_mon+1, _tm->tm_mday,
	// 					_tm->tm_hour, _tm->tm_min, _tm->tm_sec);
	// }
	
	UNSET_STATE(LOGGER_WORKING);
	file.close();
}

int VarioLogger::write(uint8_t ch)
{
	if (! IS_SET(LOGGER_WORKING))
		return 0;
	
	#if 1	
	if (columnCount < 0 && ch != 'B')
		return 0;
	
	if (ch == 'B')
		columnCount = 0;
	
	// pressure altitude field is replaced by measured pressure altitude : NmeaParser returns null value
	if (columnCount == IGC_OFFSET_PRESS_ALT)
		digit.begin(varioAltitude, IGC_SIZE_PRESS_ALT);
	
	if (digit.available())
		ch = digit.read();
	
	file.write((const uint8_t *)&ch, 1);
	columnCount += 1;
	
	if (columnCount == MAX_IGC_SENTENCE)
		columnCount = -1; // eol
	#else
	file.write((const uint8_t *)&ch, 1);
	#endif
	
	return 1;
}

void VarioLogger::updateBaroAltitude(float varioAlt)
{
	varioAltitude = varioAlt;
	//Serial.println(varioAltitude);
}

int VarioLogger::isInitialized()
{
	return IS_SET(LOGGER_INIT_FAILED) ? false : true;
}

int VarioLogger::isLogging()
{
	return IS_SET(LOGGER_WORKING) ? true : false;
}

void VarioLogger::reset()
{
	logState		= 0;
	columnCount 	= -1;
	varioAltitude 	= 0.0;
}

int VarioLogger::validateName(int ch)
{
	if ('A' <= ch && ch <= 'Z')
		return ch;
	if ('a' <= ch && ch <= 'z')
		return (ch - 'a') + 'A';
	if ('0' <= ch && ch <= '9')
		return ch;

	return '_';
}

const char * VarioLogger::makeFileName(char * buf, time_t date)
{
	// name format : YYYY-MM-DD-NRC-XXX-nn.igc
	// ...
	int pos = 0;
	int i, num;
	const char * ptr;
	struct tm * _tm;

	date = date + (__DeviceContext.logger.timezone * 60 * 60); 
	_tm = localtime(&date);
	
	// year
	digit.begin(_tm->tm_year + 1900, 4);
	while (digit.available())
		buf[pos++] = digit.read();
	buf[pos++] = '-';
	// month
	digit.begin(_tm->tm_mon + 1, 2);
	while (digit.available())
		buf[pos++] = digit.read();
	buf[pos++] = '-';
	// day
	digit.begin(_tm->tm_mday, 2);
	while (digit.available())
		buf[pos++] = digit.read();
	buf[pos++] = '-';

	// manufacture code
	for (i = 0, ptr = nameManufacture; i < 3; i++)
		buf[pos++] = (*ptr) ? *ptr++ : 'X';
	buf[pos++] = '-';
		
	// FR serial number
	ptr = __DeviceContext.logger.pilot[0] ? __DeviceContext.logger.pilot : serialNumber;
	for (i = 0; i < 3; i++)
		buf[pos++] = validateName(ptr[i]); // (*ptr) && (*ptr != 0x20) ? *ptr++ : 'X';
	buf[pos++] = '-';
	
	// flight number of the day & file extension
	for (ptr = tailOfFileName; *ptr; ptr++)
		buf[pos++] = *ptr;
	buf[pos] = '\0'; // null-terminate
	
	for (i = 2; i < 101; i++) // valid number : 01 ~ 99
	{
		strcpy(pathBuf, logsFolder);
		strcat(pathBuf, "/");
		strcat(pathBuf, buf);

		if (! SD_CARD.exists(pathBuf))
			return buf;
		
		buf[19] = (i / 10) + '0';
		buf[20] = (i % 10) + '0';
	}
	
	return NULL;
}

void VarioLogger::writeHeader(time_t date)
{
	if(! IS_SET(LOGGER_WORKING))
		return;

	//
	for (int i = 0; i < sizeof(igcHeader) / sizeof(igcHeader[0]); i++)
	{
		switch (i)
		{
		case IGC_HEADER_DATE	 :
			{
				struct tm * _tm;
				_tm = localtime(&date);				
				
				//  DD : tm_mday -> 1 ~ 31
				digit.begin(_tm->tm_mday, 2);
				while (digit.available())
					file.write(digit.read());
				// MM : tm_mon -> 0 ~ 11
				digit.begin(_tm->tm_mon + 1, 2);
				while (digit.available())
					file.write(digit.read());
				// YY : tm_year -> xxxx
				digit.begin(_tm->tm_year % 100, 2);
				while (digit.available())
					file.write(digit.read());
				
				//
//				time_t localdate = date + (__DeviceContext.logger.timezone * 60 * 60); 
//				_tm = localtime(&localdate);	
//				
//				file.timestamp(T_ACCESS|T_CREATE|T_WRITE,
//								_tm->tm_year + 1900, _tm->tm_mon+1, _tm->tm_mday,
//								_tm->tm_hour, _tm->tm_min, _tm->tm_sec);
			}
			break;
		case IGC_HEADER_PILOT	 :
			if (__DeviceContext.logger.pilot[0])
				file.print(__DeviceContext.logger.pilot);
			break;
		case IGC_HEADER_CLASS	 :
			break;
		case IGC_HEADER_GLIDER	 :
			if (__DeviceContext.gliderInfo.manufacture[0] || __DeviceContext.gliderInfo.model[0])
				file.printf("%s %s", __DeviceContext.gliderInfo.manufacture, __DeviceContext.gliderInfo.model);
			break;
		case IGC_HEADER_GPSDATUM :
			// leave it empty!!
			break;
		default 				 :
			file.print(igcHeader[i]);
			break;
		}
	}
}
