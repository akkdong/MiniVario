// NmeaParserEx.cpp
//

#include "NmeaParserEx.h"
#include "FixedLenDigit.h"
#include "DeviceContext.h"


/////////////////////////////////////////////////////////////////////////////
//

#define DEBUG_PARSING		0

#define NMEA_TALKER_SIZE	(2)
#define NMEA_TAG_SIZE 		(5)

#define CLEAR_STATE(x)		x = 0
#define SET_STATE(x,bit)	x |= (bit)
#define UNSET_STATE(x,bit)	x &= ~(bit)

#define IS_SET(x,bit)		(x & (bit))

#define SEARCH_RMC_TAG		(1 << 0)
#define SEARCH_GGA_TAG		(1 << 1)
#define PARSE_RMC			(1 << 2)
#define PARSE_GGA			(1 << 3)
#define RMC_VALID			(1 << 4)
#define GGA_VALID			(1 << 5)

#define IGC_SENTENCE_LOCKED	(1 << 6)


/////////////////////////////////////////////////////////////////////////////
//

const char tagRMC[] PROGMEM = {"GPRMC"};
const char tagGGA[] PROGMEM = {"GPGGA"};


float nmeaToDecimal(float nmea)
{
	int dd = (int)(nmea / 100);
	float ss = nmea - (float)(dd * 100.0);

	return (float)dd + ss / 60.0;
}

int compareTime(struct tm* t1, struct tm* t2)
{
	int time1 = t1->tm_hour * 3600 + t1->tm_min * 60 + t1->tm_sec;
	int time2 = t2->tm_hour * 3600 + t2->tm_min * 60 + t2->tm_sec;

	if (time1 == time2)
		return 0;
	if (time1 < time2)
		return -1;
	return 1;
}


/////////////////////////////////////////////////////////////////////////////
// class DataQueue

DataQueue::DataQueue()
{
	mHead = mTail = mFront;
}

void DataQueue::push(int ch)
{
	mBuffer[mFront] = ch;
	mFront = (mFront + 1) % MAX_NMEA_PARSER_BUFFER;
}

int DataQueue::pop()
{
	if (mHead == mTail)
		return -1;

	int ch = mBuffer[mTail];
	mTail = (mTail + 1) % MAX_NMEA_PARSER_BUFFER;

	return ch;
}

int DataQueue::get(int index)
{
	return mBuffer[index % MAX_NMEA_PARSER_BUFFER];
}

int DataQueue::copy(char* dst, int startPos, int count)
{
	for (int i = 0; i < count; i++)
		dst[i] = mBuffer[(startPos + i) % MAX_NMEA_PARSER_BUFFER];

	return count;
}

void DataQueue::acceptReserve()
{
	mHead = mFront;
}

void DataQueue::rejectReserve()
{
	mFront = mHead;
}

void DataQueue::dumpReserve()
{
	for(int i = mHead; i != mFront; )
	{
		int ch = mBuffer[i];
		if (ch == '\r' || ch == '\n')
			ch = '.';
		Serial.write((char)ch);

		i = (i + 1) % MAX_NMEA_PARSER_BUFFER;
	}
}


/////////////////////////////////////////////////////////////////////////////
// class NmeaParserEx

NmeaParserEx::NmeaParserEx(Stream & stm, Stream & dgb) : mStream(stm), mStreamDbg(dgb), mSimulMode(false)
{
	//
	reset();
	
	// initialize un-chagned characters
	memset(mIGCSentence, '0', sizeof(mIGCSentence));
	
	mIGCSentence[IGC_OFFSET_START] = 'B';
	mIGCSentence[IGC_OFFSET_VALIDITY] = 'A';
	mIGCSentence[IGC_OFFSET_RETURN] = '\r';
	mIGCSentence[IGC_OFFSET_NEWLINE] = '\n';
//	mIGCSentence[IGC_OFFSET_TERMINATE] = '\0';
}

void NmeaParserEx::update(/*float baroAlt*/)
{
	//
	//mBaroAlt = baroAlt;
	Stream & strm = mSimulMode ? mStreamDbg : mStream;
	
	//
	while (strm.available())
	{
		//
		int c = strm.read();
		#if DEBUG_PARSING
		Serial.print((char)c);
		#endif
			

		//if (mSimulMode)
		//	mStreamDbg.write(c);
		
		if (mDataQueue.isFull())
		{
			#if DEBUG_PARSING
			Serial.println("Full DataQueue!!");
			#endif
			break;
		}
		
		if (mParseStep < 0 && c != '$')
			continue; // skip bad characters(find first sentence character)
		
		mDataQueue.push(c);

		//
		if (c == '$')
		{
			mParseStep 	= 0;
			//mParseState	= 0;
			mParity		= 0;

			SET_STATE(mParseState, SEARCH_RMC_TAG|SEARCH_GGA_TAG);
			UNSET_STATE(mParseState, PARSE_RMC|PARSE_GGA);

			#if DEBUG_PARSING
			Serial.println("start sentence");
			#endif
		}
		else
		{
			// $XXXXX,...*CCrn
			//  01234566667891   : parse step
			
			// calculate parity : before checksum(include '*')
			if (mParseStep <= NMEA_TAG_SIZE + 1)
				mParity ^= c;

			if (mParseStep < NMEA_TALKER_SIZE) // 0 ~ 1, Talker ID
			{
				// continue: skip all Talker ID
				mParseStep += 1;
			}
			else if (mParseStep < NMEA_TAG_SIZE) // 2 ~ 4, Sentence identifier
			{
				//if (c != pgm_read_byte_near(tagRMC + mParseStep))
				if (c != tagRMC[mParseStep])
					UNSET_STATE(mParseState, SEARCH_RMC_TAG);
				//if (c != pgm_read_byte_near(tagGGA + mParseStep))
				if (c != tagGGA[mParseStep])
					UNSET_STATE(mParseState, SEARCH_GGA_TAG);

				if (! IS_SET(mParseState, SEARCH_RMC_TAG) && ! IS_SET(mParseState, SEARCH_GGA_TAG))
				{
					#if DEBUG_PARSING
					Serial.print("[unsupport sentence: ");
					mDataQueue.dumpReserve();
					Serial.println("]");
					#endif

					// It's not valid(known) TAG!!!
					mParseStep = -1;
					mDataQueue.rejectReserve();
				}
				else
				{
					// continue
					mParseStep += 1;
				}
			}
			else if (mParseStep == NMEA_TAG_SIZE) // 5, start of data
			{
				#if DEBUG_PARSING
				Serial.print("** mParseState = "); Serial.print((int)mParseState, 16); Serial.println("h");
				#endif

				if (c != ',' || (! IS_SET(mParseState, SEARCH_RMC_TAG) && ! IS_SET(mParseState, SEARCH_GGA_TAG)))
				{
					// bad sentence : reset parsing state
					#if DEBUG_PARSING
					Serial.print("[unknown tag: ");
					mDataQueue.dumpReserve();
					Serial.println("]");
					#endif
					
					mParseStep = -1;
					mDataQueue.rejectReserve();
				}
				else
				{
					mFieldStart = mDataQueue.front();
					mFieldIndex = 0;
					
					mParseStep += 1;
					#if DEBUG_PARSING
					Serial.println("start data field");
					#endif
					
					// new entry : set to invalid 
					if (IS_SET(mParseState, SEARCH_RMC_TAG))
					{
						SET_STATE(mParseState, PARSE_RMC);
					}
					else //if (IS_SET(mParseState, SEARCH_GGA_TAG))
					{
						SET_STATE(mParseState, PARSE_GGA);
						
						// make unavailable the IGC sentence, if It's unlocked
						if (! IS_SET(mParseState, IGC_SENTENCE_LOCKED))
						{
							mIGCSize = 0;
							mIGCNext = 0;
						}
					}
				}
			}
			else if (mParseStep == NMEA_TAG_SIZE + 1) // 6, data
			{
				if (c == ',' || c == '*') // field delimiter
				{
					// parse field
					parseField(mFieldIndex, mFieldStart);
					
					mFieldStart = mDataQueue.front();
					mFieldIndex = mFieldIndex + 1;
				}
				
				if (c == '*') // start of checksum
				{
					mParseStep += 1;
					mParity ^= '*';	// '*' removed by twice xor

					#if DEBUG_PARSING
					Serial.print("Checksum: "); Serial.print(mParity, 16); Serial.println("h");
					#endif
				}

				#if DEBUG_PARSING && 0
				if (IS_SET(mParseState, PARSE_GGA))
				{
					Serial.print((char)c); Serial.print(" : "); Serial.print(mParity, 16); Serial.println("h");
				}
				#endif
			}
			else if (mParseStep == NMEA_TAG_SIZE + 2) // checksum high-nibble
			{
				int n = (c >= 'A') ? (c - 'A' + 10) : (c - '0');
				
				if (n != ((mParity & 0xF0) >> 4)) // bad checksum
				{
					mParseStep = -1;
					mDataQueue.rejectReserve();

					#if DEBUG_PARSING
					Serial.print("Bad checksum #1: "); Serial.print(mParity, 16);
					Serial.print("h compared with "); Serial.println((char)c);
					#endif
				}
				else
				{
					mParseStep += 1;
				}
			}
			else if (mParseStep == NMEA_TAG_SIZE + 3) // 7, checksum low-nibble
			{
				int n = (c >= 'A') ? (c - 'A' + 10) : (c - '0');
				
				if (n != (mParity & 0x0F)) // bad checksum
				{
					mParseStep = -1;
					mDataQueue.rejectReserve();

					#if DEBUG_PARSING
					Serial.print("Bad checksum #2: "); Serial.print(mParity, 16);
					Serial.print("h compared with "); Serial.println((char)c);
					#endif
				}
				else
				{
					mParseStep += 1;
				}
			}
			else if (mParseStep == NMEA_TAG_SIZE + 4) // 8, carrage-return
			{
				if (c != '\r')
				{
					mParseStep = -1;
					mDataQueue.rejectReserve();

					#if DEBUG_PARSING
					Serial.println("Bad carrage return");
					#endif
				}
				else
				{
					mParseStep += 1;
				}
			}
			else if (mParseStep == NMEA_TAG_SIZE + 5) // 9, newline
			{
				if (c != '\n')
				{
					mParseStep = -1;
					mDataQueue.rejectReserve();

					#if DEBUG_PARSING
					Serial.println("Bad newline");
					#endif
				}
				else
				{
					#if DEBUG_PARSING
					Serial.print("[complete a setence: ");
					mDataQueue.dumpReserve();
					Serial.println("]");
					Serial.print("== mParseState = "); Serial.print(mParseState, 16); Serial.println("h");
					#endif
					
					// complete a sentence
					mParseStep = -1;
					mDataQueue.acceptReserve();

					//
					if (IS_SET(mParseState, PARSE_RMC))
					{
						// compare current(rmc) time with last one --> reset valid flag, if it is not same time
						if (IS_SET(mParseState, GGA_VALID) && mTimeCurr != mTimeLast)
						{
							#if DEBUG_PARSING
							Serial.println("Unmatched RMC: throw previous GGA");
							#endif
							UNSET_STATE(mParseState, GGA_VALID);
						}
					}
					if (IS_SET(mParseState, PARSE_GGA))
					{
						// compare current(gga) time with last one --> reset valid flag, if it is not same time
						if (IS_SET(mParseState, RMC_VALID) && mTimeCurr != mTimeLast)
						{
							#if DEBUG_PARSING
							Serial.println("Unmatched GGA: throw previous RMC");
							#endif

							UNSET_STATE(mParseState, RMC_VALID);
						}
					}

					// save to last time
					mTimeLast = mTimeCurr;

					// check IGC sentence ready condition
					/*
					if (IS_SET(mParseState, GGA_VALID) && !IS_SET(mParseState, IGC_SENTENCE_LOCKED))
					{
						// IGC sentence is available
						mIGCSize = MAX_IGC_SENTENCE;
						mIGCNext = 0;

						#if DEBUG_PARSING
						for (int i = 0; i < mIGCSize; i++)
							Serial.print((char)mIGCSentence[i]);
						#endif
					}
					*/

					// check GPS data ready condition
					if (IS_SET(mParseState, GGA_VALID) && IS_SET(mParseState, RMC_VALID))
					{
						mDateTime = mktime(&mTmStruct); // mDateTime is UTC: mktime convert GMTx to UTC
						mFixed = true;
						mDataReady = true;

						if (!IS_SET(mParseState, IGC_SENTENCE_LOCKED))
						{
							// IGC sentence is available
							mIGCSize = MAX_IGC_SENTENCE;
							mIGCNext = 0;

							#if DEBUG_PARSING
							for (int i = 0; i < mIGCSize; i++)
								Serial.print((char)mIGCSentence[i]);
							#endif
						}

						// unset valid state
						UNSET_STATE(mParseState, GGA_VALID|RMC_VALID);						
					}
					else
					{
						// check unfixed
						if (!IS_SET(mParseState, GGA_VALID) && !IS_SET(mParseState, RMC_VALID))
							mFixed = false;
					}
					
					// unset parse state
					UNSET_STATE(mParseState, PARSE_GGA|PARSE_RMC);
					
					// the logger(readIGC) does not unlock state while the parser does parsing(GGA).
					// so the parser must unlocked it
					if (IS_SET(mParseState, IGC_SENTENCE_LOCKED) && mIGCSize == mIGCNext)
					{
						UNSET_STATE(mParseState, IGC_SENTENCE_LOCKED);
						
						mIGCSize = 0;
						mIGCNext = 0;
					}

					#if DEBUG_PARSING
					Serial.print("<< mParseState = "); Serial.print(mParseState, 16); Serial.println("h");
					#endif
				}
			}			
		}
	}
}

int NmeaParserEx::available()
{
	return (! mDataQueue.isEmpty());
}

int NmeaParserEx::read()
{
	return mDataQueue.pop();
}

int NmeaParserEx::availableIGC()
{
	return (mIGCSize == MAX_IGC_SENTENCE && mIGCNext < MAX_IGC_SENTENCE);
}

int NmeaParserEx::readIGC()
{
	if (mIGCSize == MAX_IGC_SENTENCE && mIGCNext < MAX_IGC_SENTENCE) // if available
	{
		// start reading... : lock state
		if (mIGCNext == 0)
			SET_STATE(mParseState, IGC_SENTENCE_LOCKED);
		
		int ch = mIGCSentence[mIGCNext++];

		// if it reaches end of sentence, state & buffer must be cleared.
		// however, if it's parsing state, let the parser clear it.
		if (mIGCNext == MAX_IGC_SENTENCE && ! IS_SET(mParseState, PARSE_GGA)) // end of sentence
		{
			// unlock
			UNSET_STATE(mParseState, IGC_SENTENCE_LOCKED);
			
			// empty
			mIGCSize = 0;
			mIGCNext = 0;
		}
		
		return ch;
	}
	
	return -1;
}

void NmeaParserEx::reset()
{
	//
	//mDataQueue.init();
	
	//
	mDataReady = false;	
	mTimeCurr = (time_t)-1;
	mTimeLast = (time_t)-1;
	mDateTime = 0;
	mLatitude = 0.0;
	mLongitude = 0.0;	
	mSpeed = 0;
	mAltitude = 0.0;
	mHeading = 0;
	mFixed = false;
	
	mParseStep = -1;
	mParseState = 0;

	//	
	mIGCSize = 0;
	mIGCNext = 0;
}

int NmeaParserEx::timeStr2TmStruct(struct tm * _tm, int startPos)
{
	//
	char str[6];

	for (int i = 0; i < 6; i++)
	{
		int ch = mDataQueue.get(startPos + i);
		if (ch < '0' || '9' < ch)
			return -1;

		str[i] = ch;
	}

	//
	_tm->tm_hour = ((str[0] - '0') * 10) + (str[1] - '0');
	_tm->tm_min  = ((str[2] - '0') * 10) + (str[3] - '0');
	_tm->tm_sec  = ((str[4] - '0') * 10) + (str[5] - '0');
	_tm->tm_isdst = 0;

	_tm->tm_min = _tm->tm_min + (int)(__DeviceContext.logger.timezone * 60);

	return 0;
}

int NmeaParserEx::dateStr2TmStruct(struct tm * _tm, int startPos)
{
	//
	char str[6];

	for (int i = 0; i < 6; i++)
	{
		int ch = mDataQueue.get(startPos + i);
		if (ch < '0' || '9' < ch)
			return -1;

		str[i] = ch;
	}
	
	// nmea date(year) -> since 2000
	// struct tm(year) -> since 1900
	//
	// tm_year <-- nmea date(year) + 100
	//
	
	_tm->tm_mday = ((str[0] - '0') * 10) + (str[1] - '0');			// tm_mday : 1 ~ 31
	_tm->tm_mon  = ((str[2] - '0') * 10) + (str[3] - '0') - 1;		// tm_mon  : 0 ~ 11
	_tm->tm_year = ((str[4] - '0') * 10) + (str[5] - '0') + 100;	// tm_year : 0 -> 1900, nm_year : 0 -> 2000

	return 0;
}

void NmeaParserEx::parseField(int fieldIndex, int startPos)
{
	if (IS_SET(mParseState, PARSE_RMC)) // if (IS_SET(mParseState, SEARCH_RMC_TAG))
	{
		switch(fieldIndex)
		{
		case 0 : // Time (HHMMSS.sss UTC)
			// save RMC time
			if (timeStr2TmStruct(&mTmStruct, startPos) == 0)
				mTimeCurr = mTmStruct.tm_hour * 3600 + mTmStruct.tm_min * 60 + mTmStruct.tm_sec;
			else
				mTimeCurr = (time_t)-1;
			break;
			#if DEBUG_PARSING
			Serial.print("Time RMC = "); Serial.print(mTimeCurr, 16); Serial.println("h");
			#endif
		case 1 : // Navigation receiver warning A = OK, V = warning
			if (mDataQueue.get(startPos) == 'A')
				SET_STATE(mParseState, RMC_VALID);
			else
				UNSET_STATE(mParseState, RMC_VALID);
			break;
		case 2 : // Latitude (DDMM.mmm)
			break;
		case 3 : // Latitude (N or S)
			break;
		case 4 : // Longitude (DDDMM.mmm)
			break;
		case 5 : // Longitude (E or W)
			break;
		case 6 : // Speed over ground, Knots
			mSpeed = (int16_t)(strToFloat(startPos) * 1.852); // convert Knot to Km/h
			#if DEBUG_PARSING
			Serial.print("Speed = "); Serial.println(mSpeed, 10);
			#endif
			break;
		case 7 : // Track Angle in degrees, True
			mHeading = (int16_t)(strToFloat(startPos) + 0.5);
			break;
		case 8 : // Date of fix  (DDMMYY)
			dateStr2TmStruct(&mTmStruct, startPos);
			
			#if _MOVED_ // move to end of sentence
			if (IS_SET(mParseState, RMC_VALID))
				mDateTime = mktime(&mTmStruct); // mDateTime is UTC: mktime convert GMTx to UTC
			//else
			//	mDateTime = 0;
			//mDate = strToNum(startPos);
			#endif
			break;
		}
	}
	else if (IS_SET(mParseState, PARSE_GGA)) // if (IS_SET(mParseState, SEARCH_GGA_TAG))
	{
		switch(fieldIndex)
		{
		case 0 : // Time (HHMMSS.sss UTC)
			// save GGA_time
			if (timeStr2TmStruct(&mTmStruct, startPos) == 0)
				mTimeCurr = mTmStruct.tm_hour * 3600 + mTmStruct.tm_min * 60 + mTmStruct.tm_sec;
			else
				mTimeCurr = (time_t)-1;
			#if DEBUG_PARSING
			Serial.print("Time GGA = "); Serial.print(mTimeCurr, 16); Serial.println("h");
			#endif

			// update IGC sentence if it's unlocked
			if (! IS_SET(mParseState, IGC_SENTENCE_LOCKED))
				mDataQueue.copy(&mIGCSentence[IGC_OFFSET_TIME], startPos, IGC_SIZE_TIME);
			break;
		case 1 : // Latitude (DDMM.mmm)
			// save latitude
			{
				float nmeaLatitude = strToFloat(startPos);
				mLatitude = nmeaToDecimal(nmeaLatitude);
				
				// update IGC sentence if it's unlocked
				if (! IS_SET(mParseState, IGC_SENTENCE_LOCKED))
				{
					FixedLenDigit digit;
				
					digit.begin(floatToCoordi(nmeaLatitude), IGC_SIZE_LATITUDE);
					for (int i = 0; i < IGC_SIZE_LATITUDE /*digit.available()*/; i++)
						mIGCSentence[IGC_OFFSET_LATITUDE+i] = digit.read();
				}
			}
			break;
		case 2 : // Latitude (N or S)
			// save latitude
			if (mDataQueue.get(startPos) != 'N')
				mLatitude = -mLatitude; // south latitude is negative
			
			// update IGC sentence if it's unlocked
			if (! IS_SET(mParseState, IGC_SENTENCE_LOCKED))
				mIGCSentence[IGC_OFFSET_LATITUDE_] = mDataQueue.get(startPos);
			break;
		case 3 : // Longitude (DDDMM.mmmm)
			// save longitude
			{
				float nmeaLongitude = strToFloat(startPos);
				mLongitude = nmeaToDecimal(nmeaLongitude);
				
				// update IGC sentence if it's unlocked
				if (! IS_SET(mParseState, IGC_SENTENCE_LOCKED))
				{
					FixedLenDigit digit;
				
					digit.begin(floatToCoordi(nmeaLongitude), IGC_SIZE_LONGITUDE);
					for (int i = 0; i < IGC_SIZE_LONGITUDE /*digit.available()*/; i++)
						mIGCSentence[IGC_OFFSET_LONGITUDE+i] = digit.read();
				}
			}			
			break;
		case 4 : // Longitude (E or W)
			// save longitude
			if (mDataQueue.get(startPos) != 'E')
				mLongitude = -mLongitude; // west longitude is negative
			
			// update IGC sentence if it's unlocked
			if (! IS_SET(mParseState, IGC_SENTENCE_LOCKED))
				mIGCSentence[IGC_OFFSET_LONGITUDE_] = mDataQueue.get(startPos);
			break;
		case 5 : // GPS Fix Quality (0 = Invalid, 1 = GPS fix, 2 = DGPS fix)
			if (mDataQueue.get(startPos) != '0')
				SET_STATE(mParseState, GGA_VALID);
			else
				UNSET_STATE(mParseState, GGA_VALID);
			break;
		case 6 : // Number of Satellites
			break;
		case 7 : // Horizontal Dilution of Precision
			break;
		case 8 : // Altitude(above means sea level)
			// save GPS altitude
			mAltitude = strToFloat(startPos);
			#if DEBUG_PARSING
			Serial.print("Altitude = "); Serial.println(mAltitude);
			#endif
			
			// update IGC sentence if it's unlocked
			if (! IS_SET(mParseState, IGC_SENTENCE_LOCKED))
			{
				FixedLenDigit digit;
			
				//
				//digit.begin(mBaroAlt, IGC_SIZE_PRESS_ALT);
				//for (int i = 0; i < IGC_SIZE_PRESS_ALT /*digit.available()*/; i++)
				//	mIGCSentence[IGC_OFFSET_PRESS_ALT+i] = digit.read();
				
				//
				digit.begin(mAltitude, IGC_SIZE_GPS_ALT);
				for (int i = 0; i < IGC_SIZE_GPS_ALT /*digit.available()*/; i++)
					mIGCSentence[IGC_OFFSET_GPS_ALT+i] = digit.read();
			}			
			break;
		case 9 : // Altitude unit (M: meter)
			break;
		}
	}
}

float NmeaParserEx::strToFloat(int startPos, int limit)
{
	float value = 0.0, div = 0;
	
	for (int i = startPos; ; i++)
	{
		if (limit > 0 && limit <= i - startPos)
			break;

		int ch = mDataQueue.get(i);

		if (ch == '.')
		{
			div = 1;
		}
		else if ('0' <= ch && ch <= '9')
		{
			value *= 10;
			value += ch - '0';

			if (div)
				div *= 10;
		}
		else
		{
			// end of converting
			break;
		}
	}
	
	if (div)
		value /= div;
	
	return value;
}

long NmeaParserEx::strToNum(int startPos, int limit)
{
	long value = 0;
	unsigned char sign = 1;
	
	for (int i = startPos; ; i++)
	{
		if (limit > 0 && limit <= i - startPos)
			break;

		int ch = mDataQueue.get(i);

		if (i == startPos && ch == '-')
		{
			sign = -1;
		}
		else if ('0' <= ch && ch <= '9')
		{
			value *= 10;
			value += ch - '0';
		}
		else
		{
			// end of converting
			break;
		}
	}
	
	return value * sign;
}

long NmeaParserEx::floatToCoordi(float value)
{
	// DDDMM.mmmm -> DDDMMmmm (with round up)
	#if _SIMPLIFY_
	long coordi = (long)value;
	float temp = (value - coordi) * 1000.0f;
	
	return coordi * 1000 + (long)temp;
	#else
	float temp = value * 1000.0f;
	return (long)temp;
	#endif
}

void NmeaParserEx::enableSimulation(bool enable)
{
	mSimulMode = enable;

	Stream & stm = mStreamDbg;
	stm.printf("Simulation %s\n", mSimulMode ? "On" : "Off");

	reset();
}