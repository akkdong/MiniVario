// DeviceContext.h
//

#ifndef __DEVICECONTEXT_H__
#define __DEVICECONTEXT_H__

#include <Arduino.h>
#include "DeviceDefines.h"

#define MAX_STRING_SIZE					(16)
#define MAX_VARIO_HISTORY				(30)
#define MAX_VARIO_TONE					(12)


////////////////////////////////////////////////////////////////////////////////////////////////
// 

struct VarioState
{
	//
	float			altitudeGPS;		// QNH
	float			altitudeBaro;		// QNE
	float			altitudeCalibrated; // Baro + (Baro <-> GPS differential : altitudeDrift)
	float			altitudeAGL;		// QFE (from terrain)
	float			altitudeRef1;		// QFE (from take-off)
	float			altitudeRef2;		// QFE (from landing)
	float			altitudeRef3;		// QFE (from any-altitude)
	
	float			altitudeDrift;
	
	//
//	float			altitudeAbtain;
//	float			thermalTime;
//	float			thermalGain;
	float			glideRatio;	
	
	//
	float			speedGround;
	float			speedAir;
	float			speedVertActive;
	float			speedVertLazy;
	float			speedVertMean2S;
//	float			speedVertStat;

	float			speedVertHistory[MAX_VARIO_HISTORY]; // 1s vertial speed history

	//
	float			logitude;
	float			latitude;

	float			heading;
	float			bearing;
	
	//
	float			pressure;
	float			temperature;		// by barometer
	float			temperatureAlt;		// by thermometer

	//
	time_t			timeCurrent;
	time_t			timeStart;
};

struct VarioSettings
{
	float			sinkThreshold;
	float			climbThreshold;
	float			sensitivity;
	
	uint8_t			sentence;			// 
	
	float			altitudeRef1;		// reference base altitude
	float			altitudeRef2;
	float			altitudeRef3;
	
	float			dampingFactor; 		//  affect to VarioState.speedVertLazy
};

struct GliderInfo
{
	uint8_t			type;
	char			manufacture[MAX_STRING_SIZE];
	char			model[MAX_STRING_SIZE];
};

struct IGCLogger
{
	uint8_t			enable;
	
	int				takeoffSpeed;
	int				landingTimeout;
	int				loggingInterval;
	
	char			pilot[MAX_STRING_SIZE];
	char			timezone;
};

struct VarioTone
{
	float			velocity;
	
	uint16_t		freq;
	uint16_t		period;
	uint16_t		duty;
};

struct VolumeSettings
{
	uint8_t			vario;
	uint8_t			effect;
};

struct ThresholdSettings
{
	float			lowBattery;
	uint32_t		shutdownHoldtime;

	uint32_t		autoShutdownVario;
};

struct KelmanParameters
{
	float			varZMeas;
	float			varZAccel;
	float			varAccelBias;
};

struct DeviceState
{
	float			batteryPower;
	
	uint8_t			statusGPS;		// 0: no-signal, 1: 
	uint8_t			statusBT;		// 0: disabled, 1: on-wait-client, 2: connected
	uint8_t			statusSDCard;	// 0: empty, 1: valid
//	uint8_t			statusSound;	// 0: mute, 1: on
	
	char			btName[MAX_STRING_SIZE];
};



////////////////////////////////////////////////////////////////////////////////////////////////
// class DeviceContext

class DeviceContext
{
public:
	DeviceContext();
	
public:
	void				reset();
	
public:
	// Variometer State
	VarioState			varioState;
	VarioSettings		varioSetting;
	
	//
	GliderInfo			gliderInfo;
	IGCLogger			logger;
	
	VarioTone			toneTable[MAX_VARIO_TONE];
	
	VolumeSettings		volume;
	ThresholdSettings	threshold;
	
	KelmanParameters	kalman;
	
	// Device state
	DeviceState			device;
	
private:
	static VarioTone	defaultTone[MAX_VARIO_TONE];

};

extern DeviceContext	__DeviceContext;

#endif // __DEVICECONTEXT_H__


