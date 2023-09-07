// DeviceContext.cpp
//

#include "DeviceContext.h"

#define RADIUS 					(6371000) // 6371e3

#define TO_RADIAN(x)			((x)*(PI/180))
#define GET_DISTANCE(angle)		(2.0 * RADIUS * sin(TO_RADIAN((angle) / 2)))




////////////////////////////////////////////////////////////////////////////////////////////////
//

DeviceContext __DeviceContext;


const VarioTone DeviceContext::defaultTone[MAX_VARIO_TONE] =
#if 0
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
#elif 0
{
	{ -10.00,    200, 200,  100 },
	{   -3.00,   290, 200,  100 },
	{   -2.00,   369, 200,  100 },
	{   -1.00,   440, 200,  100 },
	{   -0.50,   470, 600,  100 },
	{    0.00,   558, 600,   30 },
	{    1.00,   574, 525,   50 },
	{    2.00,   596, 450,   60 },
	{    4.00,   667, 300,   60 },
	{    6.00,   809, 267,   60 },
	{    8.00,  1082, 233,   60 },
	{   10.00,  1616, 200,   60 },
};
#elif 0
// y = exp((x + 320) / 180) x 0.8 + 800
//
{
	{  -10.00,   200, 200,  100 },
	{   -3.00,   290, 200,  100 },
	{   -2.00,   369, 200,  100 },
	{   -1.00,   440, 200,  100 },
	{   -0.50,   470, 600,  100 },
	{    0.00,   988, 600,   50 },	// B5
	{    1.00,  1175, 400,   54 },	// D6
	{    2.00,  1319, 300,   58 },	// E6
	{    4.00,  1397, 275,   62 },	// F6
	{    6.00,  1568, 250,   66 },	// G6
	{    8.00,  1760, 225,   68 },	// A6
	{   10.00,  1976, 200,   70 },	// B6
};
#else
{
	#if 0
	{  -10.00,	200,	200,	100 },
	{  -3.00,	300,	200,	100 },
	{  -2.00,	350,	200,	100 },
	{  -1.00,	420,	200,	100 },
	{  -0.50,	450,	800,	100 },
	{   0.00,	480,	800,	 50 },
	{   1.00,	626,	720,	 54 },
	{   2.00,	778,	640,	 58 },
	{   4.00,	1080,	500,	 62 },
	{   6.00,	1360,	380,	 66 },
	{   8.00,	1600,	280,	 68 },
	{  10.00,	1800,	200,	 70 },
	#else
	{  -10.00,	 200,	100,	100 },
	{  -3.00,	 280,	100,	100 },
	{  -0.51,	 300,	500,	100 },
	{  -0.50,	 200,	800,	  5 },
	{   0.09,	 400,	600,	 10 },
	{   0.10,	 400,	600,	 50 },
	{   1.16,	 550,	552,	 52 },
	{   2.67,	 764,	483,	 55 },
	{   4.24,	 985,	412,	 58 },
	{   6.00,	1234,	322,	 62 },
	{   8.00,	1517,	241,	 66 },
	{  10.00,	1800,	150,	 70 },
	#endif
};
#endif

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
	logger.landingSpeed = FLIGHT_STOP_MIN_SPEED;		// km/s
	logger.landingTimeout = FLIGHT_LANDING_THRESHOLD;	// ms
	logger.loggingInterval = FLIGHT_LOGGING_INTERVAL;	// ms
	
	memset(logger.pilot, 0, sizeof(logger.pilot));
	logger.timezone = VARIOMETER_TIME_ZONE; 			// GMT+9	
	
	//
	varioSetting.sinkThreshold = VARIOMETER_SINKING_THRESHOLD; // -3.0
	varioSetting.climbThreshold = VARIOMETER_CLIMBING_THRESHOLD; // 0.2
	varioSetting.sensitivity = VARIOMETER_SENSITIVITY; // 0.12
	
	varioSetting.sentence = VARIOMETER_DEFAULT_NMEA_SENTENCE;

	varioSetting.dampingFactor	= VARIOMETER_DEFAULT_DAMPING_FACTOR; // 0.1

	// vario_tone_table 
	memcpy(&toneTable[0], &defaultTone[0], sizeof(defaultTone));
	
	//
	volume.varioDefault = volume.vario = VARIOMETER_BEEP_VOLUME;
	volume.effectDefault = volume.effect = VARIOMETER_EFFECT_VOLUME;
	volume.autoTurnOn = 1;
	
	//
	threshold.lowBattery = LOW_BATTERY_THRESHOLD;
	threshold.shutdownHoldtime = SHUTDOWN_HOLD_TIME;
	threshold.autoShutdownVario = AUTO_SHUTDOWN_THRESHOLD;
	
	//
	kalman.varZMeas = KF_ZMEAS_VARIANCE;
	kalman.varZAccel = KF_ZACCEL_VARIANCE;
	kalman.varAccelBias = KF_ACCELBIAS_VARIANCE;

	//
	deviceDefault.enableBT = 0;
	deviceDefault.enableSound = 0;
	deviceDefault.enableSimulation = 0;
	deviceDefault.enableNmeaLogging = 0;

	strcpy(deviceDefault.btName, "MiniVario");
	strcpy(deviceDefault.wifiSSID, "MiniVario");
	strcpy(deviceDefault.wifiPassword, "123456789");

	//
	deviceState.statusBT = deviceDefault.enableBT ? 1 : 0;

	//
	flightState.bearingTakeoff = -1;
	flightState.glideRatio = -1;
}

bool DeviceContext::load(Preferences & pref)
{
	#if 0 // not support
	//
	pref.getBytes("vset", &varioSetting, sizeof(varioSetting));
	
	//
	pref.getBytes("ginfo", &gliderInfo, sizeof(gliderInfo));
	pref.getBytes("logger", &logger, sizeof(logger));
	
	//
	pref.getBytes("tones", &toneTable, sizeof(toneTable));

	//	
	pref.getBytes("volume", &volume, sizeof(volume));
	pref.getBytes("thshld", &threshold, sizeof(threshold));

	//	
	pref.getBytes("kman", &kalman, sizeof(kalman));
	
	// Device state
	pref.getBytes("device", &deviceDefault, sizeof(deviceDefault));
	#endif

	return true;
}

bool DeviceContext::save(Preferences & pref)
{
	#if 0 // not support
	// 
	pref.putBytes("vset", &varioSetting, sizeof(varioSetting));
	
	//
	pref.putBytes("ginfo", &gliderInfo, sizeof(gliderInfo));
	pref.putBytes("logger", &logger, sizeof(logger));
	
	//
	pref.putBytes("tones", &toneTable, sizeof(toneTable));

	//	
	pref.putBytes("volume", &volume, sizeof(volume));
	pref.putBytes("thshld", &threshold, sizeof(threshold));

	//	
	pref.putBytes("kman", &kalman, sizeof(kalman));
	
	// Device state
	pref.putBytes("device", &deviceDefault, sizeof(deviceDefault));
	#endif

	return true;
}

void DeviceContext::set(JsonDocument & doc)
{
	if (! doc["vario_climb_threshold"].isNull())
		varioSetting.climbThreshold = doc["vario_climb_threshold"]; // 0.2
	if (! doc["vario_sink_threshold"].isNull())
		varioSetting.sinkThreshold = doc["vario_sink_threshold"]; // -3
	if (! doc["vario_sensitivity"].isNull())
		varioSetting.sensitivity = doc["vario_sensitivity"]; // 0.12
	if (! doc["vario_ref_altitude_1"].isNull())
		varioSetting.altitudeRef1 = doc["vario_ref_altitude_1"]; // 0
	if (! doc["vario_ref_altitude_2"].isNull())
		varioSetting.altitudeRef2 = doc["vario_ref_altitude_2"]; // 0
	if (! doc["vario_ref_altitude_3"].isNull())
		varioSetting.altitudeRef3 = doc["vario_ref_altitude_3"]; // 0
	if (! doc["vario_damping_factor"].isNull())
		varioSetting.dampingFactor = doc["vario_damping_factor"]; // 0.05
	if (! doc["glider_type"].isNull())
		gliderInfo.type = doc["glider_type"]; // 1
	if (! doc["glider_manufacture"].isNull())
		strcpy(gliderInfo.manufacture, (const char *)doc["glider_manufacture"]); // "Ozone"
	if (! doc["glider_model"].isNull())
		strcpy(gliderInfo.model, (const char *)doc["glider_model"]); // "Zeno"
	if (! doc["igc_enable_logging"].isNull())
		logger.enable = doc["igc_enable_logging"]; // true
	if (! doc["igc_takeoff_speed"].isNull())
		logger.takeoffSpeed = doc["igc_takeoff_speed"]; // 6
	if (! doc["igc_landing_speed"].isNull())
		logger.landingSpeed = doc["igc_landing_speed"]; // 2
	if (! doc["igc_landing_timeout"].isNull())
		logger.landingTimeout = doc["igc_landing_timeout"]; // 10000
	if (! doc["igc_logging_interval"].isNull())
		logger.loggingInterval = doc["igc_logging_interval"]; // 1000
	if (! doc["igc_pilot"].isNull())
		strcpy(logger.pilot, (const char *)doc["igc_pilot"]); // "akkdong"
	if (! doc["igc_timezone"].isNull())
		logger.timezone = doc["igc_timezone"]; // 9
	if (! doc["volume_enable_vario"].isNull())
		volume.varioDefault = volume.vario = doc["volume_enable_vario"] ? 100 : 0; // false
	if (! doc["volume_enable_effect"].isNull())
		volume.effectDefault = volume.effect = doc["volume_enable_effect"] ? 100 : 0; // false
	if (! doc["volume_auto_turnon"].isNull())
		volume.autoTurnOn = doc["volume_auto_turnon"]; // true
	if (! doc["threshold_low_battery"].isNull())
		threshold.lowBattery = doc["threshold_low_battery"]; // 2.9
	if (! doc["threshold_auto_shutdown"].isNull())
		threshold.autoShutdownVario = doc["threshold_auto_shutdown"]; // 600000
	if (! doc["kalman_var_zmeas"].isNull())
		kalman.varZMeas = doc["kalman_var_zmeas"]; // 400
	if (! doc["kalman_var_zaccel"].isNull())
		kalman.varZAccel = doc["kalman_var_zaccel"]; // 1000
	if (! doc["kalman_var_abias"].isNull())
		kalman.varAccelBias = doc["kalman_var_abias"]; // 1
	if (! doc["device_enable_bt"].isNull())
		deviceDefault.enableBT = doc["device_enable_bt"]; // false
	if (! doc["device_enable_sound"].isNull())
		deviceDefault.enableSound = doc["device_enable_sound"]; // false
	if (! doc["device_bt_name"].isNull())
		strcpy(deviceDefault.btName, (const char *)doc["device_bt_name"]); // "MiniVario"
	if (! doc["device_enable_simulation"].isNull())
		deviceDefault.enableSimulation = doc["device_enable_simulation"]; // false
	if (! doc["device_enable_nmea_logging"].isNull())
		deviceDefault.enableNmeaLogging = doc["device_enable_nmea_logging"]; // false
	if (! doc["wifi_ssid"].isNull())
		strcpy(deviceDefault.wifiSSID, (const char *)doc["wifi_ssid"]); // "MiniVario"
	if (! doc["wifi_password"].isNull())
		strcpy(deviceDefault.wifiPassword, (const char *)doc["wifi_password"]); // "123456789"
}

#define VARIO_UPDATE_FREQ		(50)	// vario update every 20ms --> 50Hz
#define VARIO_AVERAGE_INTERVAL	(2)		// average for 2 seconds

void DeviceContext::updateVarioHistory()
{
	varioState.speedVertSumTotal += varioState.speedVertLazy; // varioState.speedVertActive;
	varioState.speedVertSumCount += 1;

	// 
	if (varioState.speedVertSumCount == VARIO_UPDATE_FREQ * VARIO_AVERAGE_INTERVAL) 
	{
		varioState.speedVertHistory[varioState.speedVertNext] = varioState.speedVertSumTotal / varioState.speedVertSumCount;
		//Serial.printf("#%d: %.1f -> %.1f\n", varioState.speedVertNext, varioState.speedVertSumTotal, varioState.speedVertHistory[varioState.speedVertNext]);

		varioState.speedVertSumTotal = 0.0;
		varioState.speedVertSumCount = 0;
		varioState.speedVertNext = (varioState.speedVertNext + 1) % MAX_VARIO_HISTORY;
	}
}

void DeviceContext::updateTrackHistory(float lat, float lon, float vario)
{
	// add new track point
	int nextPoint = (flightState.frontPoint + 1) % MAX_TRACK_HISTORY;

	if (nextPoint == flightState.rearPoint) // is full ?
		flightState.rearPoint = (flightState.rearPoint + 1) % MAX_TRACK_HISTORY;

	flightState.trackPoints[flightState.frontPoint].lat = lat;
	flightState.trackPoints[flightState.frontPoint].lon = lon;
	flightState.trackPoints[flightState.frontPoint].vario = vario;

	flightState.frontPoint = nextPoint;

	// calculate relative distance : distance from lastest point to each point
	int lastest = (flightState.frontPoint + MAX_TRACK_HISTORY - 1) % MAX_TRACK_HISTORY;

	for (int i = flightState.rearPoint; i != lastest; )
	{
		flightState.trackDistance[i].dx = -GET_DISTANCE(flightState.trackPoints[lastest].lon - flightState.trackPoints[i].lon);
		flightState.trackDistance[i].dy = GET_DISTANCE(flightState.trackPoints[lastest].lat - flightState.trackPoints[i].lat);

		i = (i + 1) % MAX_TRACK_HISTORY;
	}

	flightState.trackDistance[lastest].dx = 0;
	flightState.trackDistance[lastest].dy = 0;	
}

void DeviceContext::dump()
{
	Serial.printf("DeviceDefault.enableBT = %d\n", deviceDefault.enableBT);
	Serial.printf("DeviceDefault.enableSound = %d\n", deviceDefault.enableSound);
	Serial.printf("DeviceDefault.enableSimulation = %d\n", deviceDefault.enableSimulation);
	Serial.printf("DeviceDefault.enableNmeaLogging = %d\n", deviceDefault.enableNmeaLogging);
	Serial.printf("DeviceDefault.btName = %s\n", deviceDefault.btName);
	Serial.printf("DeviceDefault.wifiSSID = %s\n", deviceDefault.wifiSSID);
	Serial.printf("DeviceDefault.wifiPassword = %s\n", deviceDefault.wifiPassword);
	
	Serial.printf("VolumeSettings.vario = %d\n", volume.vario);
	Serial.printf("VolumeSettings.effect = %d\n", volume.effect);
	Serial.printf("VolumeSettings.autoTurnOn = %d\n", volume.autoTurnOn);

	Serial.printf("VarioSettings.sinkThreshold = %f\n", varioSetting.sinkThreshold);
	Serial.printf("VarioSettings.climbThreshold = %f\n", varioSetting.climbThreshold);
	Serial.printf("VarioSettings.sensitivity = %f\n", varioSetting.sensitivity);
	Serial.printf("VarioSettings.sentence = %d\n", varioSetting.sentence);
	Serial.printf("VarioSettings.altitudeRef1 = %f\n", varioSetting.altitudeRef1);
	Serial.printf("VarioSettings.altitudeRef2 = %f\n", varioSetting.altitudeRef2);
	Serial.printf("VarioSettings.altitudeRef3 = %f\n", varioSetting.altitudeRef3);
	Serial.printf("VarioSettings.dampingFactor = %f\n", varioSetting.dampingFactor);

	Serial.printf("KalmanParameter.varZMeas = %f\n", kalman.varZMeas);
	Serial.printf("KalmanParameter.varZAccel = %f\n", kalman.varZAccel);
	Serial.printf("KalmanParameter.varAccelBias = %f\n", kalman.varAccelBias);

	Serial.printf("GliderInfo.type = %d\n", gliderInfo.type);
	Serial.printf("GliderInfo.manufacture = %s\n", gliderInfo.manufacture);
	Serial.printf("GliderInfo.model = %s\n", gliderInfo.model);

	Serial.printf("IGCLogger.enable = %d\n", logger.enable);
	Serial.printf("IGCLogger.takeoffSpeed = %d\n", logger.takeoffSpeed);
	Serial.printf("IGCLogger.landingSpeed = %d\n", logger.landingSpeed);
	Serial.printf("IGCLogger.landingTimeout = %d\n", logger.landingTimeout);
	Serial.printf("IGCLogger.loggingInterval = %d\n", logger.loggingInterval);
	Serial.printf("IGCLogger.pilot = %s\n", logger.pilot);
	Serial.printf("IGCLogger.timezone = %f\n", logger.timezone);
}