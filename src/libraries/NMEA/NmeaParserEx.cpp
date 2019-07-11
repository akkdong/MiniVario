// NmeaParserEx.cpp
//

#include "NmeaParserEx.h"
#include "FixedLenDigit.h"


/////////////////////////////////////////////////////////////////////////////
//

#define NMEA_TAG_SIZE 		(5)

#define CLEAR_STATE()		mParseState = 0
#define SET_STATE(bit)		mParseState |= (bit)
#define UNSET_STATE(bit)	mParseState &= ~(bit)

#define IS_SET(bit)			(mParseState & (bit))


#define SEARCH_RMC_TAG		(1 << 0)
#define SEARCH_GGA_TAG		(1 << 1)
#define PARSE_RMC			(1 << 2)
#define PARSE_GGA			(1 << 3)
#define RMC_VALID			(1 << 4)
#define GGA_VALID			(1 << 5)

#define IGC_LOCKED			(1 << 6)


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


/////////////////////////////////////////////////////////////////////////////
// class NmeaParserEx

NmeaParserEx::NmeaParserEx(Stream & stm, Stream & dgb) : mStream(stm), mStreamDbg(dgb)
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
		//Serial.write(c);
		
		if (isFull())
			break;
		
		if (mParseStep < 0 && c != '$')
		{
			//Serial.print('<'); Serial.print((char)c); Serial.println('>'); 
			continue; // skip bad characters(find first sentence character)
		}
		
		mBuffer[mWrite] = c;
		mWrite = (mWrite + 1) % MAX_NMEA_PARSER_BUFFER;

		//
		if (c == '$')
		{
			mParseStep 	= 0;
			mParseState	= 0;
			mParity		= '*';	// '*' removed by twice xor

			UNSET_STATE(PARSE_RMC|PARSE_GGA|RMC_VALID|GGA_VALID);
			SET_STATE(SEARCH_RMC_TAG);
			SET_STATE(SEARCH_GGA_TAG);

			//Serial.println("start sentence");
		}
		else
		{
			// $XXXXX,...*CCrn
			//  01234566667891   : parse step
			
			// calculate parity : before checksum(include '*')
			if (mParseStep <= NMEA_TAG_SIZE + 1)
				mParity ^= c;

			if (mParseStep < NMEA_TAG_SIZE) // 0 ~ 4, sentence identifier
			{
				//if (c != pgm_read_byte_near(tagRMC + mParseStep))
				if (c != tagRMC[mParseStep])
					UNSET_STATE(SEARCH_RMC_TAG);
				//if (c != pgm_read_byte_near(tagGGA + mParseStep))
				if (c != tagGGA[mParseStep])
					UNSET_STATE(SEARCH_GGA_TAG);

				if (! IS_SET(SEARCH_RMC_TAG) && ! IS_SET(SEARCH_GGA_TAG))
				{
					//
					//Serial.println("[unsupport sentence");
					//for(int i = mHead; i != mWrite; )
					//{
					//	Serial.write(mBuffer[i]);
					//	i = (i + 1) % MAX_NMEA_PARSER_BUFFER;
					//}
					//Serial.println("]");

					// It's not valid(known) TAG!!!
					mParseStep = -1;
					mWrite = mHead;
				}
				else
				{
					// continue
					mParseStep += 1;
				}
				
				//Serial.print("mParseStep = "); Serial.println((int)mParseStep);
				//Serial.print("mParseState = "); Serial.println((int)mParseState);
			}
			else if (mParseStep == NMEA_TAG_SIZE) // 5, start of data
			{
				if (c != ',' || (! IS_SET(SEARCH_RMC_TAG) && ! IS_SET(SEARCH_GGA_TAG)))
				{
					// bad sentence : reset parsing state
					//Serial.println("[unknown tag");
					//for(int i = mHead; i != mWrite; )
					//{
					//	Serial.write(mBuffer[i]);
					//	i = (i + 1) % MAX_NMEA_PARSER_BUFFER;
					//}
					//Serial.println("]");
					
					mParseStep = -1;
					mWrite = mHead;
				}
				else
				{
					mFieldStart = mWrite;
					mFieldIndex = 0;
					
					mParseStep += 1;
					//Serial.println("start data field");
					
					// new entry : set to invalid 
					if (IS_SET(SEARCH_RMC_TAG))
					{
						SET_STATE(PARSE_RMC);
					}
					else //if (IS_SET(SEARCH_GGA_TAG))
					{
						SET_STATE(PARSE_GGA);
						
						// make unavailable the IGC sentence, if It's unlocked
						if (! IS_SET(IGC_LOCKED))
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
					
					mFieldStart = mWrite;
					mFieldIndex = mFieldIndex + 1;
				}
				
				if (c == '*') // start of checksum
				{
					mParseStep += 1;
				}
			}
			else if (mParseStep == NMEA_TAG_SIZE + 2) // checksum high-nibble
			{
				int n = (c >= 'A') ? (c - 'A' + 10) : (c - '0');
				
				if (n != ((mParity & 0xF0) >> 4)) // bad checksum
				{
					mParseStep = -1;
					mWrite = mHead;
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
					mWrite = mHead;
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
					mWrite = mHead;
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
					mWrite = mHead;
				}
				else
				{
					//Serial.println("[complete a setence");
					//for(int i = mHead; i != mWrite; )
					//{
					//	Serial.write(mBuffer[i]);
					//	i = (i + 1) % MAX_NMEA_PARSER_BUFFER;
					//}
					//Serial.println("]");
					
					// complete a sentence
					mParseStep = -1;
					mHead = mWrite;

					//
					if (IS_SET(PARSE_GGA))
					{
						if (IS_SET(GGA_VALID) && ! IS_SET(IGC_LOCKED))
						{
							// IGC sentence is available
							mIGCSize = MAX_IGC_SENTENCE;
							mIGCNext = 0;
						}

						mFixed = IS_SET(GGA_VALID) ? true : false;
						mDataReady = true;
					}
					
					// unset parse state
					UNSET_STATE(PARSE_GGA|PARSE_RMC);
					
					// the logger(readIGC) does not unlock state while the parser does parsing.
					// so the parser must unlocked it
					if (IS_SET(IGC_LOCKED) && mIGCSize == mIGCNext)
					{
						UNSET_STATE(IGC_LOCKED);
						
						mIGCSize = 0;
						mIGCNext = 0;
					}
				}
			}			
		}
	}
}

int NmeaParserEx::available()
{
	return (! isEmpty());
}

int NmeaParserEx::read()
{
	if (isEmpty())
		return -1;
	
	int c = mBuffer[mTail];
	mTail = (mTail + 1) % MAX_NMEA_PARSER_BUFFER;
	
	return c;
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
			SET_STATE(IGC_LOCKED);
		
		int ch = mIGCSentence[mIGCNext++];

		// if it reaches end of sentence, state & buffer must be cleared.
		// however, if it's parsing state, let the parser clear it.
		if (mIGCNext == MAX_IGC_SENTENCE && ! IS_SET(PARSE_GGA)) // end of sentence
		{
			// unlock
			UNSET_STATE(IGC_LOCKED);
			
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
	mHead = mTail = mWrite = 0;
	
	//
	mDataReady = false;	
	mDateTime = 0;
	//mDate = 0;
	//mTime = 0;
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

	//
	mSimulMode = false;
}

void timeStr2TmStruct(struct tm * _tm, const char * str)
{
	// validation
	for (int i = 0; i < 6; i++)
	{
		if (str[i] < '0' || '9' < str[i])
			return;
	}

	//
	_tm->tm_hour = ((str[0] - '0') * 10) + (str[1] - '0');
	_tm->tm_min  = ((str[2] - '0') * 10) + (str[3] - '0');
	_tm->tm_sec  = ((str[4] - '0') * 10) + (str[5] - '0');
}

void dateStr2TmStruct(struct tm * _tm, const char * str)
{
	// validation
	for (int i = 0; i < 6; i++)
	{
		if (str[i] < '0' || '9' < str[i])
			return;
	}
	
	// nmea date(year) -> since 2000
	// struct tm(year) -> since 1900
	//
	// tm_year <-- nmea date(year) + 100
	//
	
	_tm->tm_mday = ((str[0] - '0') * 10) + (str[1] - '0');			// tm_mday : 1 ~ 31
	_tm->tm_mon  = ((str[2] - '0') * 10) + (str[3] - '0') - 1;		// tm_mon  : 0 ~ 11
	_tm->tm_year = ((str[4] - '0') * 10) + (str[5] - '0') + 100;	// tm_year : 0 -> 1900, nm_year : 0 -> 2000
}

void NmeaParserEx::parseField(int fieldIndex, int startPos)
{
	if (IS_SET(SEARCH_RMC_TAG))
	{
		switch(fieldIndex)
		{
		case 0 : // Time (HHMMSS.sss UTC)
			timeStr2TmStruct(&mTmStruct, &mBuffer[startPos]);
			break;
		case 1 : // Navigation receiver warning A = OK, V = warning
			if (mBuffer[startPos] == 'A')
				SET_STATE(RMC_VALID);
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
			break;
		case 7 : // Track Angle in degrees, True
			mHeading = (int16_t)(strToFloat(startPos) + 0.5);
			break;
		case 8 : // Date of fix  (DDMMYY)
			dateStr2TmStruct(&mTmStruct, &mBuffer[startPos]);
			
			if (IS_SET(RMC_VALID))
				mDateTime = mktime(&mTmStruct);
			//else
			//	mDateTime = 0;
			//mDate = strToNum(startPos);
			break;
		}
	}
	else if (IS_SET(SEARCH_GGA_TAG))
	{
		switch(fieldIndex)
		{
		case 0 : // Time (HHMMSS.sss UTC)
			// update IGC sentence if it's unlocked
			if (! IS_SET(IGC_LOCKED))
			{
				for(int i = 0; i < IGC_SIZE_TIME; i++)
				{
					if ('0' > mBuffer[startPos+i] || mBuffer[startPos+i] > '9')
						break;
					
					mIGCSentence[IGC_OFFSET_TIME+i] = mBuffer[startPos+i];
				}
			}
			
			// save current time
			//mTime = strToNum(startPos);
			break;
		case 1 : // Latitude (DDMM.mmm)
			// save latitude
			{
				float nmeaLatitude = strToFloat(startPos);
				mLatitude = nmeaToDecimal(nmeaLatitude);
				
				// update IGC sentence if it's unlocked
				if (! IS_SET(IGC_LOCKED))
				{
					#if 0
					for(int i = 0, j = 0; i < IGC_SIZE_LATITUDE; i++, j++)
					{
						if ('0' <= mBuffer[startPos+j] && mBuffer[startPos+j] <= '9')
							mIGCSentence[IGC_OFFSET_LATITUDE+i] = mBuffer[startPos+j];
						else if (mBuffer[startPos+j] == '.')
							i -= 1;
						else
							break;
					}
					#else
					FixedLenDigit digit;
				
					digit.begin(floatToCoordi(nmeaLatitude), IGC_SIZE_LATITUDE);
					for (int i = 0; i < IGC_SIZE_LATITUDE /*digit.available()*/; i++)
						mIGCSentence[IGC_OFFSET_LATITUDE+i] = digit.read();
					#endif
				}
			}
			break;
		case 2 : // Latitude (N or S)
			// save latitude
			if (mBuffer[startPos] != 'N')
				mLatitude = -mLatitude; // south latitude is negative
			
			// update IGC sentence if it's unlocked
			if (! IS_SET(IGC_LOCKED))
			{
				//if (mBuffer[startPos] != 'N' && mBuffer[startPos] != 'S')
				//		break;
				mIGCSentence[IGC_OFFSET_LATITUDE_] = mBuffer[startPos];
			}
			break;
		case 3 : // Longitude (DDDMM.mmmm)
			// save longitude
			{
				float nmeaLongitude = strToFloat(startPos);
				mLongitude = nmeaToDecimal(nmeaLongitude);
				
				// update IGC sentence if it's unlocked
				if (! IS_SET(IGC_LOCKED))
				{
					#if 0
					for(int i = 0, j = 0; i < IGC_SIZE_LONGITUDE; i++, j++)
					{
						if ('0' <= mBuffer[startPos+j] && mBuffer[startPos+j] <= '9')
							mIGCSentence[IGC_OFFSET_LONGITUDE+i] = mBuffer[startPos+j];
						else if (mBuffer[startPos+j] == '.')
							i -= 1;
						else
							break;
					}
					#else
					FixedLenDigit digit;
				
					digit.begin(floatToCoordi(nmeaLongitude), IGC_SIZE_LONGITUDE);
					for (int i = 0; i < IGC_SIZE_LONGITUDE /*digit.available()*/; i++)
						mIGCSentence[IGC_OFFSET_LONGITUDE+i] = digit.read();
					#endif
				}
			}			
			break;
		case 4 : // Longitude (E or W)
			// save longitude
			if (mBuffer[startPos] != 'E')
				mLongitude = -mLongitude; // west longitude is negative
			
			// update IGC sentence if it's unlocked
			if (! IS_SET(IGC_LOCKED))
			{
				//if (mBuffer[startPos] != 'W' && mBuffer[startPos] != 'E')
				//		break;
				mIGCSentence[IGC_OFFSET_LONGITUDE_] = mBuffer[startPos];
			}
			break;
		case 5 : // GPS Fix Quality (0 = Invalid, 1 = GPS fix, 2 = DGPS fix)
			if (mBuffer[startPos] == '1' || mBuffer[startPos] == '2')
				SET_STATE(GGA_VALID);
			break;
		case 6 : // Number of Satellites
			break;
		case 7 : // Horizontal Dilution of Precision
			break;
		case 8 : // Altitude(above measn sea level)
			// save GPS altitude
			mAltitude = strToFloat(startPos);
			
			// update IGC sentence if it's unlocked
			if (! IS_SET(IGC_LOCKED))
			{
				#if 0
				int i, j;
				
				for(i = 0; i < IGC_SIZE_GPS_ALT; i++)
				{
					if (! ('0' <= mBuffer[startPos+i] && mBuffer[startPos+i] <= '9') && mBuffer[startPos+i] != '-')
						break;
					
					mIGCSentence[IGC_OFFSET_GPS_ALT+i] = mBuffer[startPos+i];
				}
				
				for (j = IGC_SIZE_GPS_ALT - 1, i -= 1; i >= 0; i--, j--)
					mIGCSentence[IGC_OFFSET_GPS_ALT+j] = mIGCSentence[IGC_OFFSET_GPS_ALT+i];
				for ( ; j >= 0; j--)
					mIGCSentence[IGC_OFFSET_GPS_ALT+j] =  '0';
				#else
				FixedLenDigit digit;
			
				//
				//digit.begin(mBaroAlt, IGC_SIZE_PRESS_ALT);
				//for (int i = 0; i < IGC_SIZE_PRESS_ALT /*digit.available()*/; i++)
				//	mIGCSentence[IGC_OFFSET_PRESS_ALT+i] = digit.read();
				
				//
				digit.begin(mAltitude, IGC_SIZE_GPS_ALT);
				for (int i = 0; i < IGC_SIZE_GPS_ALT /*digit.available()*/; i++)
					mIGCSentence[IGC_OFFSET_GPS_ALT+i] = digit.read();
				#endif
			}			
			break;
		case 9 : // Altitude unit (M: meter)
			break;
		}
	}
}

float NmeaParserEx::strToFloat(int startPos)
{
	float value = 0.0, div = 0;
	
	for (int i = startPos; ;)
	{
		if (mBuffer[i] == '.')
		{
			div = 1;
		}
		else if ('0' <= mBuffer[i] && mBuffer[i] <= '9')
		{
			value *= 10;
			value += mBuffer[i] - '0';

			if (div)
				div *= 10;
		}
		else
		{
			// end of converting
			break;
		}
		
		i = (i + 1) % MAX_NMEA_PARSER_BUFFER;
	}
	
	if (div)
		value /= div;
	
	return value;
}

long NmeaParserEx::strToNum(int startPos)
{
	long value = 0;
	unsigned char sign = 1;
	
	for (int i = startPos; ;)
	{
		if (i == startPos && mBuffer[i] == '-')
		{
			sign = -1;
		}
		else if ('0' <= mBuffer[i] && mBuffer[i] <= '9')
		{
			value *= 10;
			value += mBuffer[i] - '0';
		}
		else
		{
			// end of converting
			break;
		}
		
		i = (i + 1) % MAX_NMEA_PARSER_BUFFER;
	}
	
	return value * sign;
}

long NmeaParserEx::floatToCoordi(float value)
{
	// DDDMM.mmmm -> DDDMMmmm (with round up)
	#if 0
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

	reset();
}