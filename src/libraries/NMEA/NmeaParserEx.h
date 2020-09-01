// NmeaParserEx.h
//

#ifndef __NMEAPARSEREX_H__
#define __NMEAPARSEREX_H__

#include <Arduino.h>
#include <Stream.h>
#include <time.h>

#include "DeviceDefines.h"


/////////////////////////////////////////////////////////////////////////////
// class NmeaParserEx

class NmeaParserEx
{
public:
	NmeaParserEx(Stream & stm, Stream & dgb);
	
public:
	void				update(/*float baroAlt*/);
	
	int					available();
	int					read();
	
	int					availableIGC();
	int					readIGC();

	bool				dataReady() { return mDataReady; }
	void				resetReady() { mDataReady = false; }

	void				reset();
	
	//
	#if 0
	uint32_t			getDate();  	// DDMMYY
	uint32_t			getTime();		// HHMMSS
	#endif
	time_t				getDateTime();
	float				getLatitude();
	float				getLongitude();
	float				getAltitude();
	int16_t				getSpeed();
	int16_t				getHeading();

	bool				isFixed()	{ return mFixed; }

	void				enableSimulation(bool enable);
	
private:
	int					isFull() 	{ return ((mWrite + 1) % MAX_NMEA_PARSER_BUFFER) == mTail; }
	int					isEmpty()	{ return mHead == mTail; }
	
	void				parseField(int fieldIndex, int startPos);
	
	float				strToFloat(int startPos);
	long				strToNum(int startPos);
	
	long				floatToCoordi(float value);
	
private:
	//
	Stream &			mStream;
	Stream &			mStreamDbg;

	bool				mSimulMode;

	char				mBuffer[MAX_NMEA_PARSER_BUFFER];
	volatile int		mHead;
	volatile int		mTail;
	volatile int		mWrite;
	volatile int		mFieldStart;
	volatile int		mFieldIndex;
	
	//
	volatile int		mParseStep; 		// -1 or 0 ~ 12
	volatile uint8_t	mParseState;		// 
	volatile uint8_t	mParity;
	
	//
	bool				mDataReady;

	//uint32_t			mDate;
	//uint32_t			mTime;
	
	struct tm 			mTmStruct;
	struct tm 			mTmRMC;
	struct tm 			mTmGGA;
	time_t				mDateTime;
	
	float				mLatitude;
	float				mLongitude;
	float				mAltitude;
	int16_t				mSpeed;
	int16_t				mHeading;

	bool				mFixed;
	
//	float				mBaroAlt;
	
	// IGC sentence
	char				mIGCSentence[MAX_IGC_SENTENCE+1];
	volatile int		mIGCNext;	// next = 0 ~ MAX_XXX -1 -> available
	volatile int		mIGCSize;	// size = 0 -> empty, size = MAX_xx -> valid
};


// inline functions
//

#if 0
inline uint32_t NmeaParserEx::getDate()
	{ return mDate; }
	
inline uint32_t	NmeaParserEx::getTime()
	{ return mTime; }
#endif
	
inline time_t	NmeaParserEx::getDateTime()
	{ return mDateTime; }
	
inline float NmeaParserEx::getLatitude()
	{ return mLatitude; }
	
inline float NmeaParserEx::getLongitude()
	{ return mLongitude; }
	
inline float NmeaParserEx::getAltitude()
	{ return mAltitude; }
	
inline int16_t NmeaParserEx::getSpeed()
	{ return mSpeed; }
	
inline int16_t NmeaParserEx::getHeading()
	{ return mHeading; }
	

#endif // __NMEAPARSEREX_H__
