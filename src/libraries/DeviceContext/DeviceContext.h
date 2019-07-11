// DeviceContext.h
//

#ifndef __DEVICECONTEXT_H__
#define __DEVICECONTEXT_H__

#include <Arduino.h>
#include <Preferences.h>
#include "DeviceDefines.h"

#define MAX_STRING_SIZE					(16)
#define MAX_VARIO_HISTORY				(30)
#define MAX_VARIO_TONE					(12)

#define MAX_TRACK_HISTORY				(30)


////////////////////////////////////////////////////////////////////////////////////////////////
// 

struct VarioState // VarioDeviceState
{
	//
	float			altitudeGPS;		// QNH
	float			altitudeBaro;		// QNE
	float			altitudeCalibrated; // Baro + (Baro <-> GPS differential : altitudeDrift)
	float			altitudeAGL;		// QFE (from terrain)
	float			altitudeRef1;		// QFE (from take-off)
	float			altitudeRef2;		// QFE (from landing)
	float			altitudeRef3;		// QFE (from any-altitude)

	float			altitudeStart;		// take-off altitude
	float			altitudeGround;		// 
	
//	float			altitudeDrift;
	
	//
//	float			altitudeAbtain;
	float			thermalTime;
	float			thermalGain;
	float			glideRatio;	
	
	//
	float			speedGround;
	float			speedAir;
	float			speedVertActive;
	float			speedVertLazy;
//	float			speedVertMean2S;
//	float			speedVertStat;

	float			speedVertHistory[MAX_VARIO_HISTORY]; // 1s vertial speed history
	float			speedVertSumTotal;
	int16_t			speedVertSumCount;
	int16_t			speedVertNext;

	//
	float			longitude;
	float			latitude;

	float			longitudeLast;
	float			latitudeLast;

//	float			longitudeStart;	// take-off longitude
//	float			latitudeStart;	// take-off latitude

//	float			distTakeoff;	// distance from take-off place
//	float			distFlight;		// total flight distance(odometer?)

	int16_t			heading;
	int16_t			headingLast;
//	int16_t			bearing;
	
	//
	float			pressure;
	float			temperature;		// by barometer
	float			temperatureAlt;		// by thermometer

	//
	time_t			timeCurrent;
//	time_t			timeStart;
//	time_t			timeFly;
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

	uint8_t			autoTurnOn; // turn-on then sound automatically when it start flying
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
	uint8_t			statusSDCard;	// 0: empty, 1: valid, 2: logging
};

struct DeviceDefault
{
	uint8_t			enableBT;
	uint8_t			enableSound;
	uint8_t			enableSimulation;

	char			btName[MAX_STRING_SIZE];
};

struct FlightStats
{
	float			altitudeMax;
	float			altitudeMin;

	float			varioMax;
	float			varioMin;

	int16_t			totalThermaling;
	int16_t			thermalingMaxGain;
};

struct Position
{
	float			lat;
	float 			lon;
	float			alt;
};

struct TrackPoint
{
	float			lat;
	float			lon;
	float			vario;
};

struct TrackDistance
{
	float			dx;
	float			dy;
};

enum FlightMode
{
	FMODE_READY,	// READY, LANDING
	FMODE_FLYING,
	FMODE_CIRCLING,
	FMODE_GLIDING,
};

struct FlightState
{
	//
	uint32_t		takeOffTime;
	Position		takeOffPos;
	uint32_t		flightTime;

	//
	uint32_t		circlingStartTime;
	Position		circlingStartPos; // latitude, longitude, altitude
	uint32_t		circlingTime;
	int32_t			circlingGain;
	int32_t			circlingIncline; // tilt, slope ??

	int16_t			deltaHeading_AVG;
	int16_t			deltaHeading_SUM;

	//
	TrackPoint		trackPoints[MAX_TRACK_HISTORY];
	TrackDistance	trackDistance[MAX_TRACK_HISTORY];
	int16_t			frontPoint;
	int16_t			rearPoint;

	//
	Position		glidingStartPos;
	int32_t			glidingCount;
	float			glideRatio; // L/D(Lift to Drag)

	//
	int16_t			bearingTakeoff;
	int16_t			bearingNextPoint;

	//
	float			distTakeoff;
	float			distFlight;
	float			distNextPoint;

	//
	FlightMode		flightMode;
};


////////////////////////////////////////////////////////////////////////////////////////////////
// class DeviceContext

class DeviceContext
{
public:
	DeviceContext();
	
public:
	void				reset();

	bool				load(Preferences & pref);
	bool				save(Preferences & pref);

	void				updateVarioHistory();
	void				updateTrackHistory(float lat, float lon, float vario);

	void				resetFlightState() { memset(&flightState, 0, sizeof(flightState)); }
	void				resetFlightStats() { memset(&flightStats, 0, sizeof(flightStats)); }
	
public:
	// Persist Settings
	VarioSettings		varioSetting;
	
	GliderInfo			gliderInfo;
	IGCLogger			logger;
	
	VarioTone			toneTable[MAX_VARIO_TONE];
	
	VolumeSettings		volume;
	ThresholdSettings	threshold;
	
	KelmanParameters	kalman;

	DeviceDefault		deviceDefault;
	
	// Variometer State
	VarioState			varioState;
	// Device state
	DeviceState			deviceState;

	//
	FlightState			flightState;
	FlightStats			flightStats;

private:

	
private:
	static const VarioTone defaultTone[MAX_VARIO_TONE];
};

extern DeviceContext	__DeviceContext;

#endif // __DEVICECONTEXT_H__


