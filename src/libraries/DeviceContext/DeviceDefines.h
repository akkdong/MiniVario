// DeviceDefines.h
//

#ifndef __DEVICEDEFINES_H__
#define __DEVICEDEFINES_H__


// Variometer default settings
//

#define VARIOMETER_TIME_ZONE					(9)	// GMT+9
#define VARIOMETER_BEEP_VOLUME					(100)	// percentage (0 ~ 100)
#define VARIOMETER_EFFECT_VOLUME				(100)

#define VARIOMETER_SINKING_THRESHOLD 			(-3.0)
#define VARIOMETER_CLIMBING_THRESHOLD 			(0.2)
#define VARIOMETER_SENSITIVITY 					(0.15)
#define VARIOMETER_MIN_VELOCITY					(-10)
#define VARIOMETER_MAX_VELOCITY					(10)

#define VARIOMETER_LXNAV_SENTENCE				(1)
#define VARIOMETER_LK8_SENTENCE					(2)
#define VARIOMETER_DEFAULT_NMEA_SENTENCE		VARIOMETER_LK8_SENTENCE

#define VARIOMETER_SENTENCE_DELAY				(500)

#define VARIOMETER_DEFAULT_DAMPING_FACTOR 		(0.5)



// Auto shutdown threshold
//

#define STABLE_SINKING_THRESHOLD				(-0.2)
#define STABLE_CLIMBING_THRESHOLD				(0.2)
#define AUTO_SHUTDOWN_THRESHOLD					(10*60*1000)	// 10min (600s)


// Low battery threshold
//

#define LOW_BATTERY_THRESHOLD					(2.8)	// VBAT drop down about 0.4v. so rear threshold voltage is -0.4
#define SHUTDOWN_HOLD_TIME						(2000)


// EEPROM(AT25C256) settings
//

#define EEPROM_TOTAL_SIZE						(32*1024)
#define EEPROM_PAGE_SIZE						(64)

#define EEPROM_ADDRESS							(0x50)	// 1 0 1 0 A2 A1 A0 R/W


// Kalman filter settings
//

// Variometer class configuration
#define POSITION_MEASURE_STANDARD_DEVIATION 	(0.1)
#define ACCELERATION_MEASURE_STANDARD_DEVIATION (0.3)

// KalmanFilter class configuration
#define KF_ZMEAS_VARIANCE       				(400.0f)
#define KF_ZACCEL_VARIANCE      				(1000.0f)
#define KF_ACCELBIAS_VARIANCE   				(1.0f)


// Flight detection settings
//

#define FLIGHT_START_MIN_SPEED 					(8.0)		// 8Km/h
#define FLIGHT_LANDING_THRESHOLD 				(30000)		// 30s
#define FLIGHT_LOGGING_INTERVAL					(1000)		// 1s



// Nmea parsing settings
//

#define MAX_NMEA_PARSER_BUFFER					(128)

#define MAX_IGC_SENTENCE						(37)	// B-sentence max size
														// ex: B1602405407121N00249342WA0028000421\r\n
														//     0123456789012345678901234567890123456789
														//     BHHMMSSDDMMmmmNDDDMMmmmWAPPPPPGGGGGCL
														//                   S        EV          RF

#define IGC_OFFSET_START						(0)
#define IGC_OFFSET_TIME							(1)
#define IGC_OFFSET_LATITUDE						(7)
#define IGC_OFFSET_LATITUDE_					(14)
#define IGC_OFFSET_LONGITUDE					(15)
#define IGC_OFFSET_LONGITUDE_					(23)
#define IGC_OFFSET_VALIDITY						(24)
#define IGC_OFFSET_PRESS_ALT					(25)
#define IGC_OFFSET_GPS_ALT						(30)
#define IGC_OFFSET_RETURN						(35)
#define IGC_OFFSET_NEWLINE						(36)
#define IGC_OFFSET_TERMINATE					(37)

#define IGC_SIZE_TIME							(6)
#define IGC_SIZE_LATITUDE						(7)
#define IGC_SIZE_LATITUDE_						(1)
#define IGC_SIZE_LONGITUDE						(8)
#define IGC_SIZE_LONGITUDE_						(1)
#define IGC_SIZE_VALIDITY						(1)
#define IGC_SIZE_PRESS_ALT						(5)
#define IGC_SIZE_GPS_ALT						(5)


enum InputKeyType
{
	KEY_UP,
	KEY_DOWN,
	KEY_SEL,
	KEY_UP_LONG,
	KEY_DOWN_LONG,
	KEY_SEL_LONG,
	KEY_Count
};

enum GliderType
{
	GTYPE_UNKNOWNM,
	GTYPE_PARAGLIDER,
	GTYPE_HG_FLEX,
	GTYPE_HG_RIGID,
	GTYPE_SAILPLANE,
	GTYPE_PPG_FOOT,
	GTYPE_PPG_WHEEL
};

#endif // __DEVICEDEFINES_H__
