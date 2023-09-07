// VarioBeeper.cpp
//

#include "VarioBeeper.h"
#include "DeviceContext.h"

#define BEEP_TYPE_SILENT				(0)
#define BEEP_TYPE_SINKING				(1)
#define BEEP_TYPE_CLIMBING				(2)



/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//

/*
struct VarioTone
{
	float 	velocity;
	int 	freq;
	int 	period;
	int 	duty;
};

static VarioTone varioTone[] = 
{
#if 0
	{ -10.00,	 200,	100,	100 }, 
	{  -3.00,	 280,	100,	100 },
	{  -0.51,	 300,	500,	100 },
	{  -0.50,	 200,	800,	  5 },
	{   0.09,	 400,	600,	 10 },
	{   0.10,	 640,	600,	 50 },
	{   1.16,	 880,	552*1.0, 52 },
	{   2.67,	1180,	483*0.9, 55 },
	{   4.24,	1400,	412*0.8, 58 },
	{   6.00,	1580,	322*0.7, 62 },
	{   8.00,	1750,	241*0.6, 66 },
	{  10.00,	1900,	150*0.5, 70 },
#else	
	{ -10.0,   200, 200, 100 },
	{  -3.0,   293, 200, 100 },
	{  -2.0,   369, 200, 100 },
	{  -1.0,   440, 200, 100 },
	{  -0.5,   475, 600, 100 },
	{   0.0,   493, 600,  50 },
	{   0.37, 1000, 369,  50 },
	{   0.92, 1193, 219,  50 },
	{   1.90, 1324, 151,  50 },
	{   3.01, 1428, 112,  50 },
	{   5.35, 1567, 100,  50 },
	{  10.00, 1687,  83,  50 },
#endif
};
*/

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//

VarioBeeper::VarioBeeper(TonePlayer & tp) :	player(tp)
{
}

void VarioBeeper::setVelocity(float velocity)
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
		player.setBeep(freq, period, duty, 0, __DeviceContext.volume.vario);
	}
	else
	{
		player.setMute(0);
	}
}

#define _count_of(x)		(sizeof(x) / sizeof(x[0]))

void VarioBeeper::findTone(float velocity, int & freq, int & period, int & duty)
{
	auto& toneTable = __DeviceContext.toneTable;
	int index;	
	
	for (index = 0; index < _count_of(toneTable); index++)
	{
		if (velocity <= toneTable[index].velocity)
			break;
	}

	if (index == 0)
	{
		freq = toneTable[0].freq;
		period = toneTable[0].period;
		duty = toneTable[0].duty;
	}
	else if (index == _count_of(toneTable))
	{
		freq = toneTable[index - 1].freq;
		period = toneTable[index - 1].period;
		duty = toneTable[index - 1].duty;
	}
	else
	{
		// [toneTable[index-1].velocity .... velocity .... toneTable[index].velocity]
		// [toneTable[index-1].freq .... freq ... toneTable[index1].freq]
		//
		// [toneTable[index-1].velocity, velocity] : [toneTable[index-1].velocity, [toneTable[index].velocity] = [toneTable[index-1].freq, freq] : [toneTable[index-1].freq, [toneTable[index].freq]
		// (velocity - toneTable[index-1].velocity) / (toneTable[index].velocity - toneTable[index-1].velocity) = (freq - toneTable[index-1].freq) / (toneTable[index].freq - toneTable[index-1].freq)
		//
		// (freq - toneTable[index-1].freq) = (velocity - toneTable[index-1].velocity) / (toneTable[index].velocity - toneTable[index-1].velocity) * (toneTable[index].freq - toneTable[index-1].freq)
		// freq = (velocity - toneTable[index-1].velocity) / (toneTable[index].velocity - toneTable[index-1].velocity) * (toneTable[index].freq - toneTable[index-1].freq) + toneTable[index-1].freq
		//
		// float ratio = (velocity - toneTable[index-1].velocity) / (toneTable[index].velocity - toneTable[index-1].velocity);
		// freq = (toneTable[index].freq - toneTable[index-1].freq) * ratio + toneTable[index-1].freq

		float ratio = (velocity - toneTable[index-1].velocity) / (toneTable[index].velocity - toneTable[index-1].velocity);
		
		freq = (toneTable[index].freq - toneTable[index-1].freq) * ratio + toneTable[index-1].freq;
		period = (toneTable[index].period - toneTable[index-1].period) * ratio + toneTable[index-1].period;
		duty = (toneTable[index].duty - toneTable[index-1].duty) * ratio + toneTable[index-1].duty;
	}
	
	//period = (int)(period * 1.0);
	duty = (int)(period * duty / 100.0);
	
	if (period == duty)
		period = duty = 0; // infinite beepping
	
	//Serial.print(freq); Serial.print(", "); Serial.print(period); Serial.print(", "); Serial.println(duty);
}
