// Beeper.h
//

#ifndef __BEEPER_H__
#define __BEEPER_H__

#include <Arduino.h>
#include "Task.h"
#include "SineGenerator.h"
#include "ToneFrequency.h"
#include "CriticalSection.h"
#include "DeviceContext.h"

#define PLAY_COOPERATIVE		(0)
#define PLAY_PREEMPTIVE			(1)


////////////////////////////////////////////////////////////////////////////////////
//

struct Tone 
{
	int			freq;
	int			length;
};

struct PlayTones
{
	Tone *		tonePtr;
	int			toneCount;

	int			volume;
	int			repeatCount;

	int			activeTone;
	int			playCount;
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

    void                setVelocity(float velocity, int volume);
    void                setMelody(Tone * tonePtr, int toneCount, int repeat, int volume, int preemptive = PLAY_COOPERATIVE);
    void                setBeep(int freq, int period, int duty, int repeat, int volume); // duty --> ms
    void                setMute(int preemptive = PLAY_PREEMPTIVE);

protected:
	void				TaskProc();

private:
    void                findTone(float velocity, int & freq, int & period, int & duty);
    void                playTone(int freq, int volume = 1);

	#if 0
    int                 playCheck();
    void                playNext();
	#endif

    void                init();

private:
	//
	PlayTones			playTones[2];		// active & candidate
	int					activeTones;	// 0 or 1

	Tone				muteTone[1];
	Tone 				beepTone[2];
	Tone				beepToneNext[2];

	//
    uint8_t             beepType; // sinking, silent, (gliding), climbing
	
	//
	SineGenerator 	    toneGen;
    CriticalSection     cs;
	DeviceContext &		context;

    //
    volatile bool       exitTask;
};


#endif // __BEEPER_H__
