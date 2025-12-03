// BatteryVoltage.h
//

#ifndef __BATTERYVOLTAGE_H__
#define __BATTERYVOLTAGE_H__

#include <Arduino.h>


///////////////////////////////////////////////////////////////////////////
// class BatteryVoltage

class BatteryVoltage
{
public:
	int						begin();
	int						update();
	
	float					getVoltage();

private:
	int						updateVoltage();
	
private:
	float					measVoltage;
	uint32_t				lastTick;
	
	uint32_t				measData;
	int						sampleCount;
};


inline float BatteryVoltage::getVoltage()
	{ return measVoltage / 1000.0; }
	

#endif // __BATTERYVOLTAGE_H__
