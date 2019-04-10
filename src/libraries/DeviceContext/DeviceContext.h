// DeviceContext.h
//

#ifndef __DEVICECONTEXT_H__
#define __DEVICECONTEXT_H__

#include <Arduino.h>

////////////////////////////////////////////////////////////////////////////////////////////////
// struct VarioState

struct VarioState
{
	float			altitudeGPS;
	float			altitudeBaro;
	float			altitudeAGL;
	float			altitudeRef1;
	float			altitudeRef2;
	float			altitudeRef3;
	float			altitudeRef4;
	
//	float			altitudeAbtain;
//	float			thermalTime;
//	float			thermalGain;
	
	float			speedGround;
	float			speedAir;
	float			speedVertActive;
	float			speedVertLazy;

	float			logitude;
	float			latitude;

	float			heading;
	float			bearing;
	
	float			glideRatio;	

	float			pressure;
	float			temperature;
};


////////////////////////////////////////////////////////////////////////////////////////////////
// class DeviceContext

class DeviceContext
{
public:
	DeviceContext();
	
public:
	void			reset();
	
public:
	// Variometer State
	VarioState		vario;
	
	// Device state
	float			batteryPower;
	uint8_t			stateGPS;		// no-signal, signal
	uint8_t			stateBT;		// unconnected, connected
	uint8_t			stateStorage;	// empty, valid
	uint8_t			stateFlight;	// ???
	
	time_t			timeCurrent;
	time_t			timeFly;
};

#endif // __DEVICECONTEXT_H__


