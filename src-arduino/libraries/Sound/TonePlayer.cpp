// TonePlayer.cpp
//

#include "TonePlayer.h"


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
//

TonePlayer::TonePlayer(SineGenerator & gen) : toneGen(gen)
{
	playTone.playType	= PLAY_NONE;
	nextTone.playType	= PLAY_NONE;
	
	/*
	//
	soundPlayPtr	= 0;
	soundPlayCount 	= 0;
	soundPlayType	= PLAY_NONE;
	
	//
	soundNextPtr 	= 0;
	soundNextCount	= 0;
	soundNextType	= PLAY_NONE;
	*/
	
	//
	toneIndex 		= 0;
	toneStartTick 	= 0;
	//
	playCount 		= 0;
	
	//
	playVolume		= MAX_VOLUME;

	//
	//Timer.pause();
	//Timer.setMode(timerCh, TIMER_OUTPUT_PWM);
	//Timer.setPeriod(1000000)
	//Timer.setCompare(timerCh, 0); // always LOW
	//Timer.refresh();
	//Timer.resume();
}

void TonePlayer::setMelody(Tone * tonePtr, int toneCount, int repeat, int preemptive, int volume)
{
	nextTone.tonePtr 		= tonePtr;
	nextTone.toneCount		= toneCount;
	nextTone.repeatCount	= repeat;
	nextTone.playVolume		= volume;
	nextTone.playType		= PLAY_MELODY;
	
	if (preemptive)
		playNext();
}

void TonePlayer::setMelodyEx(Tone * tonePtr, int toneCount, int volume)
{
	nextTone.tonePtr 		= tonePtr;
	nextTone.toneCount		= toneCount;
	nextTone.repeatCount	= 1;
	nextTone.playVolume		= volume;
	nextTone.playType		= PLAY_MELODY;
	
	playNext();
}

void TonePlayer::setBeep(int freq, int period, int duty, int repeat, int volume) 
{
	// update next beep tone
	beepToneNext[0].freq = freq;
	beepToneNext[0].length	= duty;

	beepToneNext[1].freq = 0;
	beepToneNext[1].length	= period - duty;
	
	//
	nextTone.tonePtr 		= &beepTone[0];	// beepToneNext? ??? ?? nextTone ???? beepTone?? ????.
	nextTone.toneCount		= sizeof(beepTone) / sizeof(beepTone[0]); // 2
	nextTone.repeatCount	= repeat; // infinite repeat
	nextTone.playVolume		= volume;
	nextTone.playType		= PLAY_BEEP;
}

void TonePlayer::setMute(int preemptive)
{
	nextTone.tonePtr 		= &muteTone[0];
	nextTone.toneCount		= sizeof(muteTone) / sizeof(muteTone[0]); // 1
	nextTone.repeatCount	= 0; // infinite repeat
	nextTone.playVolume		= -1;
	nextTone.playType		= PLAY_MUTE;
	
	if (preemptive)
		playNext();
}

void TonePlayer::setTone(int freq, int volume)
{
	if (volume < MIN_VOLUME)
		volume = playVolume; // use default volume
	
	if (freq > 0)
		toneGen.setTone(freq, volume);
	else
		toneGen.setTone(0);
}

void TonePlayer::setVolume(int volume)
{
	if (volume < MIN_VOLUME)
		volume = MIN_VOLUME;
	else if (volume > MAX_VOLUME)
		volume = MAX_VOLUME;

	playVolume = volume;
}

void TonePlayer::update()
{
	if (playCheck())
		playNext();
}

int TonePlayer::playCheck()
{
	if (playTone.playType == PLAY_NONE)
	{
		//if (nextTone.playType != PLAY_NONE)
		//	return 1;
		//
		//return 0;
		
		return 1;
	}
	
	if (playTone.tonePtr[toneIndex].length > 0)
	{
		unsigned long now = millis();

		if (toneStartTick + playTone.tonePtr[toneIndex].length <= now)
		{
			toneIndex += 1;
			
			if (toneIndex < playTone.toneCount)
			{
				toneStartTick = now;
				setTone(playTone.tonePtr[toneIndex].freq, playTone.playVolume);
			}
			else
			{
				// now all tone is played
				// move next or repeat again ?
				playCount += 1;
				
				if (playTone.repeatCount == 0 || playCount < playTone.repeatCount)
				{
					// interrupt continuous tone if next tone exist
					if (playTone.repeatCount == 0 && nextTone.playType != PLAY_NONE)
						return 1; // play next
					
					// else replay from first tone
					toneIndex = 0;
					toneStartTick = now;
					setTone(playTone.tonePtr[toneIndex].freq, playTone.playVolume);
				}
				else // repeatCount > 0 && playerCount == repeatCount
				{
					playTone.playType = PLAY_NONE; // stop play
					setTone(); // mute
					
					return 1; // play next
				}
			}
		}
		else
		{
			// insert mute gap between tones.
			if (toneStartTick + playTone.tonePtr[toneIndex].length - GAP_BETWEEN_TONE <= now)
				setTone(); // mute
		}
	}
	else
	{
		// 
		if (nextTone.playType != PLAY_NONE)
			return 1;
	}

	return 0;
}

void TonePlayer::playNext()
{
	if (nextTone.playType == PLAY_NONE)
		return;
	
	// copy next context to current 
	playTone = nextTone;
	// copy beepToneNext to beepTone if next play type is beep
	if (playTone.playType == PLAY_BEEP)
	{
		beepTone[0] = beepToneNext[0];
		beepTone[1] = beepToneNext[1];
	}

	// reset next play
	nextTone.playType = PLAY_NONE;
	
	//
	toneIndex = 0;
	toneStartTick = millis();
	playCount = 0;
	
	setTone(playTone.tonePtr[toneIndex].freq, playTone.playVolume);
}

// synchronous function
void TonePlayer::beep(uint32_t freq, uint32_t duration, int count, int volume)
{
	while (count > 0)
	{
		setTone(freq, volume);
		delay(duration);
		setTone();
		delay(duration);
		
		count -= 1;
	}
}
