// VarioBeeper.h
//

#ifndef __VARIOBEEPER_H__
#define __VARIOBEEPER_H__

#include <Arduino.h>
#include "DeviceDefines.h"
#include "ToneFrequency.h"
#include "TonePlayer.h"


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//

class VarioBeeper
{
public:
	VarioBeeper(TonePlayer & sp);
	
public:
//	void			setThreshold(float climbingThreshold, float sinkingThreshold);
	void			setVelocity(float velocity);
	
private:
	void 			findTone(float velocity, int & freq, int & period, int & duty);

private:
//	float 			climbingThreshold;
//	float			sinkingThreshold;
	
	//
	TonePlayer &	player;
	
	//
	uint8_t			beepType;	// sinking, silent, (gliding), climbling
};

#endif // __VARIOBEEPER_H__
