// Beeper.cpp
//

#include "Beeper.h"


////////////////////////////////////////////////////////////////////////////////////
//

#define BEEP_TYPE_SILENT		(0)
#define BEEP_TYPE_SINKING		(1)
#define BEEP_TYPE_CLIMBING		(2)

#define PLAY_NONE				(0)
#define PLAY_MUTE				(1)
#define PLAY_BEEP				(2)
#define PLAY_MELODY				(3)

#define GAP_BETWEEN_TONE		(20)		// 1ms


////////////////////////////////////////////////////////////////////////////////////
// class Beeper

Beeper::Beeper() : Task("Beeper", 2 * 1024, 2), context(__DeviceContext)
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
	//
	memset(playTones, 0, sizeof(playTones));
	memset(muteTone, 0, sizeof(muteTone));
	memset(beepTone, 0, sizeof(beepTone));
	memset(beepToneNext, 0, sizeof(beepToneNext));

	activeTones = 0;

    //
    beepType            = BEEP_TYPE_SILENT;
    exitTask            = false;
}

void Beeper::setVelocity(float velocity, int volume)
{
	boolean typeChanged = false;
	
	switch (beepType)
	{
	case BEEP_TYPE_SINKING :
		if (context.varioSetting.sinkThreshold + context.varioSetting.sensitivity < velocity)
			typeChanged = true;
		break;
		
	case BEEP_TYPE_SILENT :
		if (velocity <= context.varioSetting.sinkThreshold || context.varioSetting.climbThreshold <= velocity)
			typeChanged = true;
		break;
		
	case BEEP_TYPE_CLIMBING :
		if (velocity < context.varioSetting.climbThreshold - context.varioSetting.sensitivity)
			typeChanged = true;
		break;
	}
	
	if (typeChanged)
	{
		if (velocity <= context.varioSetting.sinkThreshold)
			beepType = BEEP_TYPE_SINKING;
		else if (context.varioSetting.climbThreshold <= velocity)
			beepType = BEEP_TYPE_CLIMBING;
		else
			beepType = BEEP_TYPE_SILENT;
	}

	if (beepType != BEEP_TYPE_SILENT)
	{
		int freq, period, duty;
		
		findTone(velocity, freq, period, duty);		
		setBeep(freq, period, duty, 0, volume);
	}
	else
	{
		setMute(0);
	}    
}

void Beeper::setMelody(Tone * tonePtr, int toneCount, int repeat, int volume, int preemptive)
{
	cs.Enter();
	{
		PlayTones * next = &playTones[1 - activeTones];

		next->tonePtr = tonePtr;
		next->toneCount = toneCount;
		next->volume = volume;
		next->repeatCount = repeat;
		next->activeTone = 0;
		next->playCount = 0;
		
		if (preemptive)
			activeTones = 1 - activeTones;
	}
	cs.Leave();
}

void Beeper::setBeep(int freq, int period, int duty, int repeat, int volume)
{
	cs.Enter();
	{
		//
		if (period == duty || period == 0)
		{
			beepToneNext[0].freq = freq;
			beepToneNext[0].length = 100;

			beepToneNext[1].freq = freq;
			beepToneNext[1].length = 100;
		}
		else
		{
			beepToneNext[0].freq = freq;
			beepToneNext[0].length = duty;

			beepToneNext[1].freq = 0;
			beepToneNext[1].length = period - duty;
		}

		//
		PlayTones * next = &playTones[1 - activeTones];

		next->tonePtr = &beepTone[0];
		next->toneCount = 2;
		next->volume = volume;
		next->repeatCount = repeat;
		next->activeTone = 0;
		next->playCount = 0;
	}
	cs.Leave();
}

void Beeper::setMute(int preemptive)
{
	cs.Enter();
	{
		PlayTones * next = &playTones[1 - activeTones];

		next->tonePtr = &muteTone[0];
		next->toneCount = 1;
		next->volume = 0;
		next->repeatCount = 0; // infinite repeat
		next->activeTone = 0;
		next->playCount = 0;
		
		if (preemptive)
			activeTones = 1 - activeTones;
	}
	cs.Leave();
}

void Beeper::TaskProc()
{
    const TickType_t xDelay10 = 30 / portTICK_PERIOD_MS;
	const TickType_t xDelay100 = 100 / portTICK_PERIOD_MS;

    while (! exitTask)
    {
		TickType_t xDelay = 0;

		cs.Enter();
		{
			PlayTones * tones = &playTones[activeTones];
			PlayTones * nextTones = &playTones[1 - activeTones];

			if (tones->tonePtr)
			{
				Tone * tone = &tones->tonePtr[tones->activeTone];

				// 
				if (tones->tonePtr == &beepTone[0] && tones->activeTone == 0)
					memcpy(&beepTone[0], &beepToneNext[0], sizeof(beepTone));
				if (tones->activeTone > 0 && tone->freq < 0)
				{
					playTone(0);
					vTaskDelay(xDelay10); // negative freqency means play with a stacato
				}

				// check if has playable tone
				if (tones->activeTone < tones->toneCount)
				{
					playTone(abs(tone->freq), tones->volume);
					tones->activeTone += 1; // next tone

					xDelay = tone->length > 0 ? tone->length / portTICK_PERIOD_MS : xDelay100;
				}
				else 
				{
					// after play all tones: no more playable tone
					// mute & increase play count
					playTone(0);
					tones->playCount += 1;

					if ((tones->repeatCount == 0 && nextTones->tonePtr == NULL) || (tones->playCount < tones->repeatCount))
					{
						// repeat tones
						tones->activeTone = 0;
					}
					else
					{
						// infinite tones has next tones or finish playing all
						// --> reset current tones
						tones->tonePtr = NULL;
						// --> change active tones
						activeTones = 1 - activeTones;						
					}
				}
			}
			else if (nextTones->tonePtr)
			{
				// change active tones
				activeTones = 1 - activeTones;	
			}
			else
			{
				// no play tones : sleep 100ms
				xDelay = xDelay100;
			}
		}
		cs.Leave();

		if (xDelay)
			vTaskDelay(xDelay);
    }

    vTaskDelete(NULL);
}

void Beeper::findTone(float velocity, int & freq, int & period, int & duty)
{
	int index;
	
	for (index = 0; index < (sizeof(context.toneTable) / sizeof(context.toneTable[0])); index++)
	{
		if (velocity <= context.toneTable[index].velocity)
			break;
	}
	
	if (index == 0 || index == (sizeof(context.toneTable) / sizeof(context.toneTable[0])))
	{
		if (index != 0)
			index -= 1;
		
		freq = context.toneTable[index].freq;
		period = context.toneTable[index].period;
		duty = context.toneTable[index].duty;
	}
	else
	{
		float ratio = context.toneTable[index].velocity / velocity;
		
		freq = (context.toneTable[index].freq - context.toneTable[index-1].freq) / (context.toneTable[index].velocity - context.toneTable[index-1].velocity) * (velocity - context.toneTable[index-1].velocity) + context.toneTable[index-1].freq;
		period = (context.toneTable[index].period - context.toneTable[index-1].period) / (context.toneTable[index].velocity - context.toneTable[index-1].velocity) * (velocity - context.toneTable[index-1].velocity) + context.toneTable[index-1].period;
		duty = (context.toneTable[index].duty - context.toneTable[index-1].duty) / (context.toneTable[index].velocity - context.toneTable[index-1].velocity) * (velocity - context.toneTable[index-1].velocity) + context.toneTable[index-1].duty;
	}
	
	//period = (int)(period * 1.0);
	duty = (int)(period * duty / 100.0);
	
	//if (period == duty)
	//	period = duty = 0; // infinite beepping
	
	//Serial.print(freq); Serial.print(", "); Serial.print(period); Serial.print(", "); Serial.println(duty);
}

#if 0
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
#endif

void Beeper::playTone(int freq, int volume)
{
//	if (freq > 0 && volume > 0)
		toneGen.setTone(freq, volume);
//	else
//		toneGen.setTone(0);
}
