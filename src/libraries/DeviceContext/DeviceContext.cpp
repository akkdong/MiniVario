// DeviceContext.cpp
//

#include "DeviceContext.h"


////////////////////////////////////////////////////////////////////////////////////////////////
//

DeviceContext __DeviceContext;


VarioTone DeviceContext::defaultTone[MAX_VARIO_TONE] =
{
	{ -10.00,    200, 200,  100 },
	{   -3.00,   290, 200,  100 },
	{   -2.00,   369, 200,  100 },
	{   -1.00,   440, 200,  100 },
	{   -0.50,   470, 600,  100 },
	{    0.00,   490, 600,   30 },
	{    0.50,   740, 460,   50 },
	{    1.00,   860, 350,   60 },
	{    2.00,  1100, 320,   60 },
	{    3.00,  1200, 300,   60 },
	{    5.00,  1400, 250,   60 },
	{   10.00,  1570, 200,   60 },
};

////////////////////////////////////////////////////////////////////////////////////////////////
// class DeviceContext

DeviceContext::DeviceContext()
{
	reset();
}
	
void DeviceContext::reset()
{
	memset(this, 0, sizeof(DeviceContext));
	
	//
	gliderInfo.type = GTYPE_PARAGLIDER;
	
	memset(gliderInfo.manufacture, 0, sizeof(gliderInfo.manufacture));
	memset(gliderInfo.model, 0, sizeof(gliderInfo.model));
	
	//
	logger.enable = true;
	
	logger.takeoffSpeed = FLIGHT_START_MIN_SPEED;		// km/s
	logger.landingTimeout = FLIGHT_LANDING_THRESHOLD;	// ms
	logger.loggingInterval = FLIGHT_LOGGING_INTERVAL;	// ms
	
	memset(logger.pilot, 0, sizeof(logger.pilot));
	logger.timezone = VARIOMETER_TIME_ZONE; 			// GMT+9	
	
	//
	varioSetting.sinkThreshold = VARIOMETER_SINKING_THRESHOLD; // -3.0
	varioSetting.climbThreshold = VARIOMETER_CLIMBING_THRESHOLD; // 0.2
	varioSetting.sensitivity = VARIOMETER_SENSITIVITY; // 0.3
	
	varioSetting.sentence = VARIOMETER_DEFAULT_NMEA_SENTENCE;

	varioSetting.dampingFactor	= VARIOMETER_DEFAULT_DAMPING_FACTOR; // 0.5

	// vario_tone_table 
	memcpy(&toneTable[0], &defaultTone[0], sizeof(defaultTone));
	
	
	//
	volume.vario = VARIOMETER_BEEP_VOLUME; // 0 ~ 80
	volume.effect = VARIOMETER_EFFECT_VOLUME;
	
	//
	threshold.lowBattery = LOW_BATTERY_THRESHOLD;
	threshold.shutdownHoldtime = SHUTDOWN_HOLD_TIME;
	threshold.autoShutdownVario = AUTO_SHUTDOWN_THRESHOLD;
	
	//
	kalman.varZMeas = KF_ZMEAS_VARIANCE;
	kalman.varZAccel = KF_ZACCEL_VARIANCE;
	kalman.varAccelBias = KF_ACCELBIAS_VARIANCE;

	//
	device.statusBT = 1;
}