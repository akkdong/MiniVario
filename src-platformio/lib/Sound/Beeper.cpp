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

	return 0;
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
	memset(Melodies, 0, sizeof(Melodies));
	memset(toneMute, 0, sizeof(toneMute));
	memset(toneBeep, 0, sizeof(toneBeep));

	activeMelody = 0;

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
		setMelody(freq, period, duty, 0, volume);
	}
	else
	{
		setMuteMelody(0);
	}    
}

void Beeper::setMelody(Tone * tonePtr, int toneCount, int repeat, int volume, int preemptive)
{
	cs.Enter();
	{
		MelodyPtr melody = getNextMelody();

		melody->tonePtr = tonePtr;
		melody->toneCount = toneCount;
		melody->volume = volume;
		melody->repeatCount = repeat;
		melody->activeTone = 0;
		melody->playCount = 0;
        melody->needStacato = 0;
		
		if (! getActiveMelodyTones() || preemptive)
			switchMelody();
	}
	cs.Leave();
}

void Beeper::setMelody(int freq, int period, int duty, int repeat, int volume, int preemptive)
{
	//Serial.printf("setMelody: %d, %d, %d, %d, %d\n", freq, period, duty, repeat, volume);
	cs.Enter();
	{
		duty = constrain(duty, 0, 100);

		//
        TonePtr tone = getNextBeepTone();

		if (period == 0 || duty == 100)
		{
			tone[0].freq = freq;
			tone[0].length = 100;

			tone[1].freq = freq;
			tone[1].length = 100;
		}
		else
		{
			tone[0].freq = freq;
			tone[0].length = period * duty / 100;

			tone[1].freq = 0;
			tone[1].length = period * (100 - duty) / 100;
		}

		//
		MelodyPtr melody = getNextMelody();

		melody->tonePtr = tone;
		melody->toneCount = 2;
		melody->volume = volume;
		melody->repeatCount = repeat;
		melody->activeTone = 0;
		melody->playCount = 0;
        melody->needStacato = 0;

		if (! getActiveMelodyTones() || preemptive)
			switchMelody();
	}
	cs.Leave();
}

void Beeper::setMuteMelody(int preemptive)
{
	cs.Enter();
	{
        MelodyPtr melody = getNextMelody();

		melody->tonePtr = &toneMute[0];
		melody->toneCount = 1;
		melody->volume = 0;
		melody->repeatCount = 1; // infinite repeat
		melody->activeTone = 0;
		melody->playCount = 0;
        melody->needStacato = 0;
		
		if (! getActiveMelodyTones() || preemptive)
			switchMelody();
	}
	cs.Leave();
}

void Beeper::switchMelody()
{
    // reset active melody
    MelodyPtr activePtr = getActiveMelody();
    memset(activePtr, 0, sizeof(Melody));

    // swap melody
    activeMelody = 1 - activeMelody;
}

void Beeper::TaskProc()
{
    const TickType_t xDelay10 = 10 / portTICK_PERIOD_MS;
	const TickType_t xDelay100 = 100 / portTICK_PERIOD_MS;

    while (! exitTask)
    {
		TickType_t xDelay = 0;

		cs.Enter();
		{
            MelodyPtr active = getActiveMelody();

            if (active->tonePtr)
            {
                if (active->needStacato) 
                {
                    // stacato delay
                    playTone(0);

                    xDelay = xDelay10;
                    active->needStacato = 0;
                }
                else if (active->activeTone < active->toneCount)
                {
                    // play active tone ...
                    TonePtr tone = &active->tonePtr[active->activeTone];
                    active->activeTone += 1;
                    active->needStacato = tone->freq < 0 ? 1 : 0; // negative frequency means stacato playing
                    xDelay = tone->length > 0 ? tone->length / portTICK_PERIOD_MS : xDelay100;

                    //Serial.printf("Play #%d: %d, %d, %d\n", active->activeTone, tone->freq, tone->length, active->volume);
                    playTone(abs(tone->freq), active->volume);
                }
                else
                {
                    // all tones are played : increase play count
                    active->playCount += 1;

                    if ((active->repeatCount == 0 && getNextMelodyTones() == NULL) || (active->playCount < active->repeatCount))
                    {
                        // case 1: infinite tone but beeper has not the next melody
                        // case 2: hasn't played all yet
                        //
                        // play melody again  : go back to first tone
                        active->activeTone = 0;
                    }
                    else
                    {
                        // other case: switch melody
                        switchMelody();

                        if (! getActiveMelodyTones())
                        {
                            //Serial.printf("Stop play: mute\n");
                            playTone(0);
                        }
                    }
                }
            }
            else
            {
                // no active melody : sleep 100ms
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
	
    //Serial.printf("findTone: %d, %d, %d\n", freq, period, duty);
}

#define MUTE_TEMPORARY 0

void Beeper::playTone(int freq, int volume)
{
	#if !MUTE_TEMPORARY
	//	if (freq > 0 && volume > 0)
		toneGen.setTone(freq, volume);
	//	else
	//	toneGen.setTone(0);
	#endif
}
