// NmeaParserEx.h
//

#ifndef __NMEAPARSEREX_H__
#define __NMEAPARSEREX_H__

#include <Arduino.h>
#include <Stream.h>
#include <time.h>

#include "DeviceDefines.h"


/////////////////////////////////////////////////////////////////////////////
// class DataQueue

class DataQueue
{
public:
	DataQueue();

public:
	void				push(int ch);
	int					pop();

	int					front() { return mFront; }

	int					isFull() 	{ return ((mFront + 1) % MAX_NMEA_PARSER_BUFFER) == mTail; }
	int					isEmpty()	{ return mHead == mTail; }	

	int					get(int index);
	int					copy(char* dst, int startPos, int count);

	void				acceptReserve();
	void				rejectReserve();


	// debugging stubs
	void				dumpReserve();


protected:
	char				mBuffer[MAX_NMEA_PARSER_BUFFER];
	volatile int		mHead;
	volatile int		mTail;
	volatile int		mFront;
};



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

	bool				isDataReady() { return mDataReady; }
	void				resetDataReady() { mDataReady = false; }

	bool				isFixed()	{ return mFixed; }

	void				reset();
	
	//
	time_t				getDateTime();
	float				getLatitude();
	float				getLongitude();
	float				getAltitude();
	int16_t				getSpeed();
	int16_t				getHeading();

	void				enableSimulation(bool enable);
	
private:
	int 				timeStr2TmStruct(struct tm * _tm, int startPos);
	int	 				dateStr2TmStruct(struct tm * _tm, int startPos);
	
	void				parseField(int fieldIndex, int startPos);
	
	float				strToFloat(int startPos, int limit = -1);
	long				strToNum(int startPos, int limit = -1);
	
	long				floatToCoordi(float value);
	
private:
	//
	Stream &			mStream;
	Stream &			mStreamDbg;

	bool				mSimulMode;

	DataQueue			mDataQueue;
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
	time_t				mTimeCurr; // time of latest parsing sentence
	time_t				mTimeLast; // time of last parsing sentence
	time_t				mDateTime; // current date & time

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
