// Beeper.h
//

#ifndef __BEEPER_H__
#define __BEEPER_H__

#include <Arduino.h>
#include "Task.h"
#include "SineGenerator.h"
#include "ToneFrequency.h"
#include "CriticalSection.h"


#define PLAY_COOPERATIVE		(0)
#define PLAY_PREEMPTIVE			(1)


////////////////////////////////////////////////////////////////////////////////////
//

struct Tone 
{
	int			freq;
	int			length;
};

struct PlayContext
{
	//
	Tone *			tonePtr;
	int				toneCount;
	
	//
	int				playVolume;
	int				playType;
	
	//
	int				repeatCount;
};


////////////////////////////////////////////////////////////////////////////////////
// class Beeper

class Beeper : public Task
{
public:
    Beeper();

public:
    int                 begin();
    void                end(); 

    void                setVelocity(float velocity);
    void                setMelody(Tone * tonePtr, int toneCount, int repeat, int volume = -1, int preemptive = PLAY_COOPERATIVE);
    void                setBeep(int freq, int period, int duty, int repeat, int volume = -1); // duty --> ms
    void                setMute(int preemptive = PLAY_PREEMPTIVE);

protected:
	void				TaskProc();

private:
    void                findTone(float velocity, int & freq, int & period, int & duty);

    int                 playCheck();
    void                playNext();
    void                playTone(int freq, int volume = -1);

    void                init();

private:
	//
	PlayContext		    tonePlay;
	PlayContext		    toneNext;

	//
	int				    toneIndex;
	unsigned long	    toneStartTick;

	//
	int				    playCount;
	int				    playVolume;

    
    uint8_t             beepType; // sinking, silent, (gliding), climbing
	
	//
	SineGenerator 	    toneGen;
    CriticalSection     cs;

    //
    volatile bool       exitTask;
};


#endif // __BEEPER_H__
