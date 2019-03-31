// Test1.ino
//


#include "SineGenerator.h"


SineGenerator sGen;
uint32_t tickCount;
int freq = 260;

void __dacWrite(uint8_t pin, uint8_t value);

uint8_t dacValue = 0;

void setup()
{
	digitalWrite(27, HIGH);
	pinMode(27, OUTPUT);	
	
	Serial.begin(115200);
	delay(200);
	
	#if 0
    //setup channel 0 with frequency 312500 Hz
    sigmaDeltaSetup(0, 312500);
    //attach pin 18 to channel 0
    sigmaDeltaAttachPin(16,0);
    //initialize channel 0 to off
    sigmaDeltaWrite(0, 0);
	#endif
	
	
	sGen.begin(SineGenerator::USE_DIFFERENTIAL, SineGenerator::SCALE_FULL);
	
	//sGen.turnOn(frequency);
	//sGen.turnOff();
	//sGen.setScale(SCALE_FULL);
	//sGen.setOffset(offset);
	
	sGen.setFrequency(freq);
	tickCount = millis();
	
	//
	//__dacWrite(26, dacValue);
}

void loop()
{
	#if 1
	if ((millis() - tickCount) > 2000)
	{
		freq += 200;
		if (freq > 1400)
			freq = 200;
		
		sGen.setFrequency(freq);
		tickCount = millis();
		
		Serial.print("freqency = "); Serial.println(freq);
	}
	#endif
	
	#if 0
    //slowly ramp-up the value
    //will overflow at 256
    static uint8_t i = 0;
    sigmaDeltaWrite(0, i);
	i += 10;
    delay(100);	
	#endif
	
	#if 0
	if ((millis() - tickCount) > 100)
	{
		__dacWrite(26, ++dacValue);
		tickCount = millis();
	}
	#endif
}
