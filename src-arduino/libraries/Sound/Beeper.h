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

typedef struct Tone * TonePtr;

struct Melody
{
	TonePtr 	tonePtr;
	int			toneCount;

	int			volume;
	int			repeatCount;

	int			activeTone;
	int			playCount;

    int         needStacato;
};

typedef struct Melody * MelodyPtr;


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
    void                setMelody(int freq, int period, int duty, int repeat, int volume, int preemptive = PLAY_COOPERATIVE);
    
    void                setMuteMelody(int preemptive = PLAY_PREEMPTIVE);

protected:
	void				TaskProc();

private:
    void                findTone(float velocity, int & freq, int & period, int & duty);
    void                playTone(int freq, int volume = 1);

    void                init();

    MelodyPtr           getActiveMelody() { return &Melodies[activeMelody]; }
    MelodyPtr           getNextMelody() { return &Melodies[1 - activeMelody]; }

    TonePtr             getActiveMelodyTones() { return Melodies[activeMelody].tonePtr; }
    TonePtr             getNextMelodyTones() { return Melodies[1 - activeMelody].tonePtr; }

    TonePtr             getActiveBeepTone() { return &toneBeep[activeMelody][0]; }
    TonePtr             getNextBeepTone() { return &toneBeep[1 - activeMelody][0]; }

    void                switchMelody();

private:
	//
	Melody			    Melodies[2];	// active & next
	int					activeMelody;	// 0 or 1

	Tone				toneMute[1];
	Tone 				toneBeep[2][2];

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
