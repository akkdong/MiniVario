// LK8Sentence.cpp
//

#include "VarioSentence.h"

#define LK8_SENTENCE_TAG 					"$LK8EX1,,A,V,T,B,*P\r\n"
#define LK8_SENTENCE_TAG_SIZE 				21
#define LK8_SENTENCE_PRESSURE_POS 			8
#define LK8_SENTENCE_ALTI_POS 				9
#define LK8_SENTENCE_VARIO_POS 				11
#define LK8_SENTENCE_TEMP_POS 				13
#define LK8_SENTENCE_BAT_POS 				15
#define LK8_SENTENCE_PARITY_POS 			18
#define LK8_SENTENCE_PRESSURE_PRECISION 	0
#define LK8_SENTENCE_ALTI_PRECISION 		0
#define LK8_SENTENCE_VARIO_PRECISION 		0

#define LK8_BASE_SEA_PRESSURE 				1013.25


/////////////////////////////////////////////////////////////////////////////
// class LK8Sentence

const char * LK8Sentence::LK8Tag = LK8_SENTENCE_TAG;


void LK8Sentence::begin(float height, float vel, float temp, float bat)
{
	altimeter = height;
	vario = vel * 100.0; // cm/s
	temperature = temp;
	voltage = bat;
	
	float pressure = LK8_BASE_SEA_PRESSURE * 100.0 * pow(1 - (0.0065/288.15)*altimeter, 5.255); 
	valueDigit.begin(pressure, LK8_SENTENCE_PRESSURE_PRECISION);
	
	parity = '$';
	tagPos = 0;
}

void LK8Sentence::begin(float height, float vel, float temp, float prs, float bat)
{
	altimeter = height;
	vario = vel * 100.0; // cm/s
	temperature = temp;
	voltage = bat;
	
	valueDigit.begin(prs * 100.0, LK8_SENTENCE_PRESSURE_PRECISION);
	
	parity = '$';
	tagPos = 0;
}

int LK8Sentence::available()
{
	if (tagPos < LK8_SENTENCE_TAG_SIZE)
		return true;
	
	return false;
}

int LK8Sentence::read()
{
	uint8_t outc = 0;

	// check digits 
	if( valueDigit.available() && tagPos >= LK8_SENTENCE_PRESSURE_POS )
	{
		outc = valueDigit.get();
	}
	else if( parityDigit.available() )
	{
		outc =  parityDigit.get();
	}	
	else // else write tag
	{
		outc = pgm_read_byte_near(LK8Tag + tagPos);
		tagPos++;

		// check special characters
		if( tagPos == LK8_SENTENCE_ALTI_POS )
		{
			valueDigit.begin(altimeter, LK8_SENTENCE_ALTI_PRECISION);
			tagPos++;
		}
		else if( tagPos == LK8_SENTENCE_VARIO_POS )
		{
			valueDigit.begin(vario, LK8_SENTENCE_VARIO_PRECISION);
			tagPos++;
		}
		else if( tagPos == LK8_SENTENCE_TEMP_POS )
		{
			valueDigit.begin(temperature, 1);
			tagPos++;
		}
		else if( tagPos == LK8_SENTENCE_BAT_POS )
		{
			valueDigit.begin(voltage, 1);
			tagPos++;
		}
		else if( tagPos == LK8_SENTENCE_PARITY_POS )
		{
			parityDigit.begin(parity);
			tagPos++;
		}
	}

	// parity 
	parity ^= outc;
	
	return outc;
}
