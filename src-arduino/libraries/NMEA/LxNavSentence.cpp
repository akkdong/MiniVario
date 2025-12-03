// LxNavSentence.cpp
//

#include "VarioSentence.h"


/* no special character for the first digit (alti), just for vario "V" and parity "P" */
#define LXNAV_SENTENCE_TAG 				"$LXWP0,Y,,,V,,,,,,,,*P\r\n" 
#define LXNAV_SENTENCE_TAG_SIZE 		24
#define LXNAV_SENTENCE_ALTI_POS 		10
#define LXNAV_SENTENCE_VARIO_POS 		11
#define LXNAV_SENTENCE_PARITY_POS 		21
#define LXNAV_SENTENCE_ALTI_PRECISION 	1
#define LXNAV_SENTENCE_VARIO_PRECISION 	2


/////////////////////////////////////////////////////////////////////////////
// class LxNavSentence

const char * LxNavSentence::LxNavTag = LXNAV_SENTENCE_TAG;


void LxNavSentence::begin(float height, float vel, float temp, float bat)
{
  vario = vel; //vario is in cm/s
  valueDigit.begin(height, LXNAV_SENTENCE_ALTI_PRECISION);
  
  parity = '$';  //remove characters not in parity, patity computed before '*' 
  tagPos = 0;	
}

void LxNavSentence::begin(float height, float vel, float temp, float prs, float bat)
{
	begin(height, vel, temp, bat);
}

int	LxNavSentence::available()
{
	if( tagPos < LXNAV_SENTENCE_TAG_SIZE )
		return true;

	return false;
}

int LxNavSentence::read()
{
	uint8_t outc = 0;

	// check digits
	if( valueDigit.available() && tagPos >= LXNAV_SENTENCE_ALTI_POS )
	{
		outc = valueDigit.get();
	}
	else if( parityDigit.available() )
	{
		outc =  parityDigit.get();
	}
	else // else write tag
	{
		outc = pgm_read_byte_near(LxNavTag + tagPos);
		tagPos++;

		// check special characters
		if( tagPos == LXNAV_SENTENCE_VARIO_POS )
		{
			valueDigit.begin(vario, LXNAV_SENTENCE_VARIO_PRECISION);
			tagPos++;
		}
		else if( tagPos == LXNAV_SENTENCE_PARITY_POS )
		{
			parityDigit.begin(parity);
			tagPos++;
		}
	}

	// parity
	parity ^= outc;
	
	return outc;
}
