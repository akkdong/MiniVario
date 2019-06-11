// Beeper.cpp
//

#include "Beeper.h"
#include "DeviceContext.h"

////////////////////////////////////////////////////////////////////////////////////
//

#define BEEP_TYPE_SILENT		(0)
#define BEEP_TYPE_SINKING		(1)
#define BEEP_TYPE_CLIMBING		(2)

#define PLAY_NONE				(0)
#define PLAY_MUTE				(1)
#define PLAY_BEEP				(2)
#define PLAY_MELODY				(3)

#define GAP_BETWEEN_TONE		(1)		// 1ms


////////////////////////////////////////////////////////////////////////////////////
//

static Tone muteTone[] = 
{ 
	{ 0, 0 }
};

static Tone beepTone[] = 
{
	{ 100, 100 },
	{   0, 100 }
};

static Tone beepToneNext[] = 
{
	{ 100, 100 },
	{   0, 100 }
};


////////////////////////////////////////////////////////////////////////////////////
// class Beeper

Beeper::Beeper() : Task("Beeper", 2 * 1024, 2)
{ 
    init();
}


int Beeper::begin()
{
    //
    init();

    //
    toneGen.begin(SineGenerator::USE_DIFFERENTIAL, SineGenerator::SCALE_FULL, 0);
    //
    Task::createPinnedToCore(1);
}

void Beeper::end()
{
    //
    exitTask = true;
    //
    toneGen.end();
}

void Beeper::init()
{
	tonePlay.playType	= PLAY_NONE;
	toneNext.playType	= PLAY_NONE;
	//
	toneIndex 		    = 0;
	toneStartTick 	    = 0;
	//
	playCount 		    = 0;
    //
    beepType            = BEEP_TYPE_SILENT;
    exitTask            = false;
}

void Beeper::setVelocity(float velocity)
{
	boolean typeChanged = false;
	
	switch (beepType)
	{
	case BEEP_TYPE_SINKING :
		if (__DeviceContext.varioSetting.sinkThreshold + __DeviceContext.varioSetting.sensitivity < velocity)
			typeChanged = true;
		break;
		
	case BEEP_TYPE_SILENT :
		if (velocity <= __DeviceContext.varioSetting.sinkThreshold || __DeviceContext.varioSetting.climbThreshold <= velocity)
			typeChanged = true;
		break;
		
	case BEEP_TYPE_CLIMBING :
		if (velocity < __DeviceContext.varioSetting.climbThreshold - __DeviceContext.varioSetting.sensitivity)
			typeChanged = true;
		break;
	}
	
	if (typeChanged)
	{
		if (velocity <= __DeviceContext.varioSetting.sinkThreshold)
			beepType = BEEP_TYPE_SINKING;
		else if (__DeviceContext.varioSetting.climbThreshold <= velocity)
			beepType = BEEP_TYPE_CLIMBING;
		else
			beepType = BEEP_TYPE_SILENT;
	}

	if (beepType != BEEP_TYPE_SILENT)
	{
		int freq, period, duty;
		
		findTone(velocity, freq, period, duty);		
		setBeep(freq, period, duty, 0);
	}
	else
	{
		setMute(0);
	}    
}

void Beeper::setMelody(Tone * tonePtr, int toneCount, int repeat, int volume, int preemptive)
{    
	toneNext.tonePtr 		= tonePtr;
	toneNext.toneCount		= toneCount;
	toneNext.repeatCount	= repeat;
	toneNext.playVolume		= volume;
	toneNext.playType		= PLAY_MELODY;
	
	if (preemptive)
		playNext();
}

void Beeper::setBeep(int freq, int period, int duty, int repeat, int volume)
{
	// update next beep tone
	beepToneNext[0].freq = freq;
	beepToneNext[0].length	= duty;

	beepToneNext[1].freq = 0;
	beepToneNext[1].length	= period - duty;
	
	//
	toneNext.tonePtr 		= &beepTone[0];	// beepToneNext? ??? ?? toneNext ???? beepTone?? ????.
	toneNext.toneCount		= sizeof(beepTone) / sizeof(beepTone[0]); // 2
	toneNext.repeatCount	= repeat; // infinite repeat
	toneNext.playVolume		= volume;
	toneNext.playType		= PLAY_BEEP;    
}

void Beeper::setMute(int preemptive)
{
	toneNext.tonePtr 		= &muteTone[0];
	toneNext.toneCount		= sizeof(muteTone) / sizeof(muteTone[0]); // 1
	toneNext.repeatCount	= 0; // infinite repeat
	toneNext.playVolume		= -1;
	toneNext.playType		= PLAY_MUTE;
	
	if (preemptive)
		playNext();
}

void Beeper::TaskProc()
{
    const TickType_t xDelay = 1 / portTICK_PERIOD_MS;

    while (! exitTask)
    {
        if (playCheck())
            playNext();

        vTaskDelay(xDelay);
    }

    vTaskDelete(NULL);
}

void Beeper::findTone(float velocity, int & freq, int & period, int & duty)
{
	int index;
	
	for (index = 0; index < (sizeof(__DeviceContext.toneTable) / sizeof(__DeviceContext.toneTable[0])); index++)
	{
		if (velocity <= __DeviceContext.toneTable[index].velocity)
			break;
	}
	
	if (index == 0 || index == (sizeof(__DeviceContext.toneTable) / sizeof(__DeviceContext.toneTable[0])))
	{
		if (index != 0)
			index -= 1;
		
		freq = __DeviceContext.toneTable[index].freq;
		period = __DeviceContext.toneTable[index].period;
		duty = __DeviceContext.toneTable[index].duty;
	}
	else
	{
		float ratio = __DeviceContext.toneTable[index].velocity / velocity;
		
		freq = (__DeviceContext.toneTable[index].freq - __DeviceContext.toneTable[index-1].freq) / (__DeviceContext.toneTable[index].velocity - __DeviceContext.toneTable[index-1].velocity) * (velocity - __DeviceContext.toneTable[index-1].velocity) + __DeviceContext.toneTable[index-1].freq;
		period = (__DeviceContext.toneTable[index].period - __DeviceContext.toneTable[index-1].period) / (__DeviceContext.toneTable[index].velocity - __DeviceContext.toneTable[index-1].velocity) * (velocity - __DeviceContext.toneTable[index-1].velocity) + __DeviceContext.toneTable[index-1].period;
		duty = (__DeviceContext.toneTable[index].duty - __DeviceContext.toneTable[index-1].duty) / (__DeviceContext.toneTable[index].velocity - __DeviceContext.toneTable[index-1].velocity) * (velocity - __DeviceContext.toneTable[index-1].velocity) + __DeviceContext.toneTable[index-1].duty;
	}
	
	//period = (int)(period * 1.0);
	duty = (int)(period * duty / 100.0);
	
	if (period == duty)
		period = duty = 0; // infinite beepping
	
	//Serial.print(freq); Serial.print(", "); Serial.print(period); Serial.print(", "); Serial.println(duty);
}

int Beeper::playCheck()
{
	if (tonePlay.playType == PLAY_NONE)
	{
		//if (toneNext.playType != PLAY_NONE)
		//	return 1;
		//
		//return 0;
		
		return 1;
	}
	
	if (tonePlay.tonePtr[toneIndex].length > 0)
	{
		unsigned long now = millis();

		if (toneStartTick + tonePlay.tonePtr[toneIndex].length <= now)
		{
			toneIndex += 1;
			
			if (toneIndex < tonePlay.toneCount)
			{
				toneStartTick = now;
				playTone(tonePlay.tonePtr[toneIndex].freq);
			}
			else
			{
				// now all tone is played
				// move next or repeat again ?
				playCount += 1;
				
				if (tonePlay.repeatCount == 0 || playCount < tonePlay.repeatCount)
				{
					// interrupt continuous tone if next tone exist
					if (tonePlay.repeatCount == 0 && toneNext.playType != PLAY_NONE)
						return 1; // play next
					
					// else replay from first tone
					toneIndex = 0;
					toneStartTick = now;
					playTone(tonePlay.tonePtr[toneIndex].freq);
				}
				else // repeatCount > 0 && playerCount == repeatCount
				{
					tonePlay.playType = PLAY_NONE; // stop play
					playTone(0); // mute
					
					return 1; // play next
				}
			}
		}
		else
		{
			// insert mute gap between tones.
			if (toneStartTick + tonePlay.tonePtr[toneIndex].length - GAP_BETWEEN_TONE <= now)
				playTone(0); // mute
		}
	}
	else
	{
		// 
		if (toneNext.playType != PLAY_NONE)
			return 1;
	}

	return 0;
}

void Beeper::playNext()
{
	if (toneNext.playType == PLAY_NONE)
		return;
	
	// copy next context to current 
	tonePlay = toneNext;
	// copy beepToneNext to beepTone if next play type is beep
	if (tonePlay.playType == PLAY_BEEP)
	{
		beepTone[0] = beepToneNext[0];
		beepTone[1] = beepToneNext[1];
	}

	// reset next play
	toneNext.playType = PLAY_NONE;
	
	//
	toneIndex = 0;
	toneStartTick = millis();
	playCount = 0;
	
	playTone(tonePlay.tonePtr[toneIndex].freq);
}

void Beeper::playTone(int freq, int volume)
{
	if (freq > 0)
		toneGen.setTone(freq, volume);
	else
		toneGen.setTone(0);
}
