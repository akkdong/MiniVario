// main.ino
//

#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>
#include <SD.h>
#include <WiFi.h>
#include <WiFiServer.h>
#include <WebServer.h>
#include <DNSServer.h>
#include <HTTPClient.h>
#include <HTTPUpdate.h>
#include <EEPROM.h>
#include <Ticker.h>
#include <sys/time.h>

#include "VarioDisplay.h"
#include "BatteryVoltage.h"
#include "Keyboard.h"
#include "DeviceDefines.h"
#include "Beeper.h"
#include "BluetoothSerialEx.h"
#include "NmeaParserEx.h"
#include "VarioSentence.h"
#include "VarioLogger.h"
#include "BluetoothMan.h"
#include "Util.h"
#include "AGL.h"
#include "TaskWatchdog.h"
#include "ScreenManager.h"
#include "FirmwareUpdater.h"
#include "ArduinoJson.h"
#include "WebService.h"

#define USE_KALMAN_VARIO 						(1)

#if USE_KALMAN_VARIO
#include "KalmanVario.h"
#else
#include "KalmanSkyDrop.h"
#endif
#include "VerticalSpeedCalculator.h"


#define WEBSERVER_PORT							(80)
#define HOST_NAME								"mini-vario"

#define DAMPING_FACTOR_DELTA_HEADING			(0.1)
#define THRESHOLD_CIRCLING_HEADING				(6)

#define MAX_GLIDING_COUNT						(10)
#define MIN_GLIDING_COUNT						(0)
#define GLIDING_START_COUNT						(6)
#define GLIDING_EXIT_COUNT						(6)
#define CIRCLING_EXIT_COUNT						(3)

#define TASK_TIMEOUT_S							(1)

#define BOOT_CHECKING_TIMOUT					(1200)


////////////////////////////////////////////////////////////////////////////////////////
//
//

enum _DeviceMode
{
	DEVICE_MODE_UNDEF = 0,
	DEVICE_MODE_WAKEUP,			// (1)
	DEVICE_MODE_VARIO,			// (2)
	DEVICE_MODE_VARIO_AND_GPS,	// (3)
	DEVICE_MODE_PREFERENCE,		// (4)
	DEVICE_MODE_WEB_SERVICE		// (5)
};

enum _VarioMode
{
	VARIO_MODE_INIT = 0,		// (0)
	VARIO_MODE_LANDING,			// (1)
	VARIO_MODE_FLYING,			// (2)
	VARIO_MODE_CIRCLING,		// (3)
};


////////////////////////////////////////////////////////////////////////////////////////
//
	
//
static PinSetting ePaperPins[] = 
{
	// CS
	{ SS, PIN_MODE_OUTPUT, PIN_ACTIVE_LOW, PIN_STATE_INACTIVE },
	// DC
	{ 33, PIN_MODE_OUTPUT, PIN_ACTIVE_LOW, PIN_STATE_INACTIVE },
	// RST
	{ 32, PIN_MODE_OUTPUT, PIN_ACTIVE_LOW, PIN_STATE_INACTIVE },
	// BUSY
	#if USE_GOOD_DISPLAY
	{ 39, PIN_MODE_INPUT, PIN_ACTIVE_HIGH, PIN_STATE_INACTIVE },
	#else
	{ 39, PIN_MODE_INPUT, PIN_ACTIVE_LOW, PIN_STATE_INACTIVE },
	#endif
};

static PinSetting powerPins[] =
{
	// PWR_PERIPH
	{ 19, PIN_MODE_OUTPUT, PIN_ACTIVE_HIGH, PIN_STATE_ACTIVE },
	// PWR_SOUND
	{ 27, PIN_MODE_OUTPUT, PIN_ACTIVE_HIGH, PIN_STATE_ACTIVE },
};

static PinSetting keybdPins[] =
{
	// KEY_UP
	{ 35, PIN_MODE_INPUT, PIN_ACTIVE_HIGH, PIN_STATE_ACTIVE },
	// KEY_DOWN
	{ 34, PIN_MODE_INPUT, PIN_ACTIVE_HIGH, PIN_STATE_ACTIVE },
	// KEY_SEL
	{  0, PIN_MODE_INPUT, PIN_ACTIVE_LOW, PIN_STATE_ACTIVE },
};


//

static Tone melodyVarioReady[] =
{
	{ -262, 1000 / 4 }, 
	{ -196, 1000 / 8 }, 
	{ -196, 1000 / 8 }, 
	{ -220, 1000 / 4 }, 
	{ -196, 1000 / 4 }, 
	{   0, 1000 / 4 }, 
	{ -247, 1000 / 4 }, 
	{ -262, 1000 / 4 },
	{   0, 1000 / 8 }, 
};
static Tone melodyTakeOff[] =
{
	{ -262, 1000 / 4 }, 
	{ -196, 1000 / 8 }, 
	{ -196, 1000 / 8 }, 
	{ -220, 1000 / 4 }, 
	{ -196, 1000 / 4 }, 
	{   0, 1000 / 4 }, 
	{ -247, 1000 / 4 }, 
	{ -262, 1000 / 4 },
	{   0, 1000 / 8 }, 
};

static Tone melodyLanding[] =
{
	{ -262, 1000 / 4 }, 
	{ -196, 1000 / 8 }, 
	{ -196, 1000 / 8 }, 
	{ -220, 1000 / 4 }, 
	{ -196, 1000 / 4 }, 
	{   0, 1000 / 4 }, 
	{ -247, 1000 / 4 }, 
	{ -262, 1000 / 4 },
	{   0, 1000 / 8 }, 
};


//
//
//

uint8_t deviceMode = DEVICE_MODE_UNDEF;
//uint8_t	varioMode; 		// sub-mode of vario-mode

RTC_DATA_ATTR uint32_t bootCount = 0;

uint32_t deviceTick;	// global tick-count
uint32_t modeTick;		// mode-specific tick-count

ScreenManager	scrnMan;
PopupTopMenu	popupTopMenu;
PopupWebService	popupWebService;


DeviceContext & context = __DeviceContext;


//
//
//

BluetoothSerialEx  	serialBluetooth;


//
//
//

#if USE_OLD_METHOD
SineGenerator toneGen;
TonePlayer tonePlayer(toneGen);
VarioBeeper varioBeeper(tonePlayer);
#else
Beeper beeper;
#endif

#if USE_DRIVER_TEMPLATE
#if USE_GOOD_DISPLAY
TVarioDisplayDriver<EPaper_GoodDisplay270> driver(ePaperPins);
#else
TVarioDisplayDriver<EPaper_Waveshare270> driver(ePaperPins);
#endif
#else
VarioDisplayDriver driver(ePaperPins);
#endif
VarioDisplay display(driver, context);

BatteryVoltage battery;
Keyboard keybd(keybdPins, sizeof(keybdPins) / sizeof(keybdPins[0]));

//
//
//

NmeaParserEx nmeaParser(Serial2, Serial);
VarioSentence varioNmea(VARIOMETER_DEFAULT_NMEA_SENTENCE);
VarioLogger logger;

//
//
//

#if USE_NORMAL_BTMAN // Normal Bluetooth manager
BluetoothMan btMan(serialBluetooth, nmeaParser, varioNmea);
#else // Logging raw NMEA sentence
BluetoothManEx btMan(serialBluetooth, nmeaParser, varioNmea);
#endif

//
//
//

CriticalSection cs;
Sensor_MS5611  baro(cs, Wire);
#if USE_KALMAN_VARIO
KalmanVario vario(baro);
#else
KalmanSkyDrop vario(baro);
#endif
VerticalSpeedCalculator varioCalculator;

//
//
//

AGL agl;


//
//
//

void readyFlight();
void startFlight();
void stopFlight();

void startCircling();
void startGliding();
void stopCircling();
void stopGliding();

void updateVarioState();
void updateFlightState();

void goDeepSleep();
void resetDevice();
void makeTopMenu();
void processKey(int key);
void startVario();
void loadPreferences();
void savePreferences();

void startWebService();
void toggleVarioSound();
void toggleBTService();
void toggleSimulation();

void setTimeZone(long offset, int daylight);


////////////////////////////////////////////////////////////////////////////////////////
//

void setup()
{
	// check wakeup reason
	switch (esp_sleep_get_wakeup_cause())
	{
	case ESP_SLEEP_WAKEUP_EXT0 :
	case ESP_SLEEP_WAKEUP_EXT1 : 
		deviceMode = DEVICE_MODE_WAKEUP;
		break;
	default :
		// if (reboot_for_preference)
		//   deviceMode = DEVICE_MODE_PREFERENCE
		// else
			deviceMode = DEVICE_MODE_VARIO;
		break;
	}

	// initialize serial(debug) & input key
	Serial.begin(115200);
	Serial.printf("Start MiniVario...\n\n");	
	//
	keybd.begin();

	// wakeup from deep-sleep
	//  --> check long key (sel)
	//  --> if not go deep-sleep again
	if (deviceMode == DEVICE_MODE_WAKEUP)
	{
		Serial.println("Wakeup from deepsleep: check long-ley input for delayed boot-up!");	
		uint32_t tick = millis();

		while (1)
		{
			keybd.update();

			if (keybd.getch() == KEY_SEL_LONG)
			{
				// wakeup confirmed!!
				break;
			}

			if ((millis() - tick) > BOOT_CHECKING_TIMOUT)
			{
				// deep-sleep again~
				Serial.println("go deep-sleep again~");
				delay(100);

				const int ext_wakeup_pin_1 = 34;
				const uint64_t ext_wakeup_pin_1_mask = 1ULL << ext_wakeup_pin_1;

				// Enabling EXT1 wakeup on pins GPIO34
				esp_sleep_enable_ext1_wakeup(ext_wakeup_pin_1_mask, ESP_EXT1_WAKEUP_ANY_HIGH);
				// Entering deep sleep
				esp_deep_sleep_start();

				while(1);
			}
		}

		deviceMode = DEVICE_MODE_VARIO;
	}

	// power on peripherals : default is power on
	initPins(powerPins, sizeof(powerPins) / sizeof(powerPins[0]));
	delay(100);

	// check sd-card & ready for use
	SD_CARD.begin();

	if (SD_CARD.valid())
	{
		FirmwareUpdater fu;

		if (fu.checkUpdate())
		{
			//
			fu.performUpdate(display);
			//
			ESP.restart();
		}		
		// else go down
	}
	else
	{
		Serial.printf("SD_CARD is invalid!!\n");
	}

	// load last device-context
	SPIFFS.begin();

	loadPreferences();
	setTimeZone(__DeviceContext.logger.timezone * -3600, 0);
	
	//
	TaskWatchdog::begin(TASK_TIMEOUT_S);
	TaskWatchdog::add(NULL);

	Serial2.begin(9600);
	Wire.begin();	
	display.begin(deviceMode == DEVICE_MODE_WAKEUP);
	beeper.begin();
	btMan.begin();
//	battery.begin();
//	vario.begin();

	// start right now or ...
	if (deviceMode == DEVICE_MODE_VARIO)
		startVario();

	//
	context.volume.vario = context.volume.varioDefault;
	context.volume.effect = context.volume.effectDefault;

	beeper.setMelody(NOTE_C4, 600, 60, 2, context.volume.effect, PLAY_PREEMPTIVE);
	deviceTick = millis();
}

void loop()
{
	//
	TaskWatchdog::reset();

	// check key-input
	keybd.update();
	
	// beeper update function is executed in task proc
	// beep beep beep!

	if (deviceMode == DEVICE_MODE_WAKEUP)
	{
		if ((millis() - deviceTick) > 10000)
		{
			goDeepSleep();
			while(1);
		}

		if (keybd.getch() == KEY_SEL)
			startVario();

		return;
	}

	if (vario.available())
	{
		//
		context.varioState.speedVertActive = vario.getVelocity();
		varioCalculator.add(context.varioState.speedVertActive);

//		context.varioState.speedVertLazy = context.varioState.speedVertLazy + (context.varioState.speedVertActive - context.varioState.speedVertLazy) * context.varioSetting.dampingFactor;
		context.varioState.speedVertLazy = varioCalculator.mVertSpeedAVG2; // 2s average
		context.varioState.altitudeBaro = vario.getAltitude();
		context.varioState.altitudeCalibrated = vario.getCalibratedAltitude();
		context.varioState.pressure = vario.getPressure();
		context.varioState.temperature = vario.getTemperature();

		//context.updateVarioHistory();
		context.varioState.speedVertSumCount = (context.varioState.speedVertSumCount + 1) % DEF_FREQUENCY;
		if (context.varioState.speedVertSumCount == 0)
		{
			context.varioState.speedVertHistory[context.varioState.speedVertNext] = varioCalculator.mVertSpeedAVG2; // 2s average
			context.varioState.speedVertNext = (context.varioState.speedVertNext + 1) % MAX_VARIO_HISTORY;
		}

		beeper.setVelocity(context.varioState.speedVertActive, context.volume.vario);

		//
		{
			//if (vario.getVelocity() < STABLE_SINKING_THRESHOLD || STABLE_CLIMBING_THRESHOLD < vario.getVelocity())
			if (context.varioState.speedVertLazy < STABLE_SINKING_THRESHOLD || STABLE_CLIMBING_THRESHOLD < context.varioState.speedVertLazy)
				deviceTick = millis(); // reset tick because it's not quiet.
			
			// if (commandReceiveFlag)
			// {
			// 	deviceTick =xx millis(); // reset tick because it's not quiet.
			// 	commandReceiveFlag = 0;
			// }
			
			if ((context.threshold.autoShutdownVario) && ((millis() - deviceTick) > context.threshold.autoShutdownVario))
				goDeepSleep();
		}			

		// update vario sentence periodically
		if (varioNmea.checkInterval())
			varioNmea.begin(vario.getAltitude(), vario.getVelocity(), vario.getTemperature(), vario.getPressure(), battery.getVoltage());
		
		//
		vario.flush();
	}
	
	// read & parse gps sentence
	nmeaParser.update();

	// update GPS fixed status
	context.deviceState.statusGPS = nmeaParser.isFixed() ? 1 : 0;

	// update vario state, if gps data is available
	if (nmeaParser.isDataReady())
	{
		updateVarioState();

		if (deviceMode == DEVICE_MODE_VARIO)
		{
			readyFlight();
		}
		else if (deviceMode == DEVICE_MODE_VARIO_AND_GPS)
		{
			if (context.flightState.flightMode == FMODE_READY)
			{
				if (nmeaParser.getSpeed() > context.logger.takeoffSpeed) // FLIGHT_START_MIN_SPEED)
				{
					startFlight();
				}
			}
			else
			{
				updateFlightState();

				if (nmeaParser.getSpeed() < context.logger.landingSpeed) // FLIGHT_START_MIN_SPEED)
				{
					if ((millis() - modeTick) > context.logger.landingTimeout) // FLIGHT_LANDING_THRESHOLD)
					{
						stopFlight();
					}
				}
				else
				{
					// reset modeTick
					modeTick = millis();
				}
				
				deviceTick = millis();
			}
		}

		nmeaParser.resetDataReady();
	}			
	
	#if _BLOCKED_ 
	// I wish it would be unfixed temporary. So I didn't reset mode. Wait unitil it is fixed
	if (deviceMode == DEVICE_MODE_VARIO_AND_GPS && !nmeaParser.isFixed())
	{
		
		if (context.flightState.flightMode != FMODE_READY)
			stopFlight();

		deviceMode = DEVICE_MODE_VARIO;
	}
	#endif

	// check logging state
	if (logger.isLogging() && nmeaParser.availableIGC())
	{
		// nmeaParser parses GPS sentence and converts it to IGC sentence
		
		//static unsigned long tick = millis();
		//static int index = 0;
		
		//if ((millis()-tick) > time_interval)
		{
			//
			float altitude = vario.getAltitude(); // getCalibratedAltitude or getAltitude
			logger.updateBaroAltitude(altitude);

			while (nmeaParser.availableIGC())
				logger.write(nmeaParser.readIGC());
		}
	}

	// send any prepared sentence to BT
	btMan.update();	

	//
	if (battery.update())
	{
		context.deviceState.batteryPower = battery.getVoltage();

		//if (context.deviceState.batteryPower < context.threshold.lowBattery)
		//{
		//	go shutdown
		//}
	}

	//
	int key = keybd.getch();
	if (key >= 0)
		processKey(key);

	//
	//WebService.update();
}

void readyFlight()
{
	// do position calibration
	vario.calibrateAltitude(nmeaParser.getAltitude());
	vario.calculateSeaLevel(nmeaParser.getAltitude());

	//
	struct timeval now = { nmeaParser.getDateTime(), 0 };
	//now.tv_sec +=  (__DeviceContext.logger.timezone * 60 * 60); // GMT(UTC+00) --> Local time
	settimeofday(&now, NULL);

	// play ready melody~~~
	//beeper.setMelody(&melodyVarioReady[0], sizeof(melodyVarioReady) / sizeof(melodyVarioReady[0]), 1, context.volume.effect/*, PLAY_PREEMPTIVE*/);
	beeper.setMelody(NOTE_C4, 400, 50, 3, context.volume.effect, PLAY_PREEMPTIVE);
	
	// now ready to fly~~~
	deviceMode = DEVICE_MODE_VARIO_AND_GPS;

	context.flightState.flightMode = FMODE_READY;
}

void startFlight()
{
	// trun on source if auto is set
	if (context.volume.autoTurnOn)
		context.volume.effect = context.volume.vario = 100;
	
	// play take-off melody
	//beeper.setMelody(&melodyTakeOff[0], sizeof(melodyTakeOff) / sizeof(melodyTakeOff[0]), 1, context.volume.effect/*, PLAY_PREEMPTIVE*/);
	beeper.setMelody(NOTE_C4, 1000, 80, 1, context.volume.effect, PLAY_PREEMPTIVE);

	//
	if (context.deviceDefault.enableNmeaLogging)
		btMan.startLogging(nmeaParser.getDateTime());
	
	// start logging & change mode
	if (context.logger.enable && logger.begin(nmeaParser.getDateTime()))
		context.deviceState.statusSDCard = 2;

	//
	context.varioSetting.altitudeRef1 = context.varioState.altitudeGPS;

	context.resetFlightState();
	context.resetFlightStats();

	context.flightState.takeOffTime = nmeaParser.getDateTime();
	context.flightState.takeOffPos.lat = context.varioState.latitude;
	context.flightState.takeOffPos.lon = context.varioState.longitude;
	context.flightState.takeOffPos.alt = context.varioState.altitudeGPS;
	context.flightState.bearingTakeoff = -1;
	context.flightState.flightMode = FMODE_FLYING;
	context.flightState.frontPoint = context.flightState.rearPoint = 0;

	context.flightStats.altitudeMax = context.flightStats.altitudeMin = context.varioState.altitudeGPS;

	// set mode-tick
	modeTick = millis();

	//
	//display.attachScreen(scrnMan.getActiveScreen());
	scrnMan.showActiveScreen(display);
}

void updateVarioState()
{
	context.varioState.latitudeLast = context.varioState.latitude;
	context.varioState.longitudeLast = context.varioState.longitude;
	context.varioState.headingLast = context.varioState.heading;

	context.varioState.latitude = nmeaParser.getLatitude();
	context.varioState.longitude = nmeaParser.getLongitude();
	context.varioState.speedGround = nmeaParser.getSpeed();
	context.varioState.heading = nmeaParser.getHeading();	
	context.varioState.timeCurrent = nmeaParser.getDateTime();
	context.varioState.altitudeGPS = nmeaParser.getAltitude();
	context.varioState.altitudeGround = agl.getGroundLevel(context.varioState.latitude, context.varioState.longitude);
	context.varioState.altitudeAGL = context.varioState.altitudeGPS - context.varioState.altitudeGround;
	context.varioState.altitudeRef1 = context.varioState.altitudeGPS - context.varioSetting.altitudeRef1;
	context.varioState.altitudeRef2 = context.varioState.altitudeGPS - context.varioSetting.altitudeRef2;
	context.varioState.altitudeRef3 = context.varioState.altitudeGPS - context.varioSetting.altitudeRef3;
}

void updateFlightState()
{
	// update flight time
	context.flightState.flightTime = nmeaParser.getDateTime() - context.flightState.takeOffTime;
	// update bearing & distance from takeoff
	context.flightState.bearingTakeoff = GET_BEARING(context.varioState.latitude, context.varioState.longitude, 
			context.flightState.takeOffPos.lat, context.flightState.takeOffPos.lon);
	context.flightState.distTakeoff = GET_DISTANCE(context.varioState.latitude, context.varioState.longitude, 
			context.flightState.takeOffPos.lat, context.flightState.takeOffPos.lon);
	// and update total flight distance
	context.flightState.distFlight += GET_DISTANCE(context.varioState.latitude, context.varioState.longitude, 
			context.varioState.latitudeLast, context.varioState.longitudeLast);
	// add new track point & calculate relative distance
	context.updateTrackHistory(context.varioState.latitude, context.varioState.longitude, context.varioState.speedVertLazy);

	// update flight statistics
	context.flightStats.altitudeMax = _MAX(context.flightStats.altitudeMax, context.varioState.altitudeGPS);
	context.flightStats.altitudeMin = _MIN(context.flightStats.altitudeMin, context.varioState.altitudeGPS);

	context.flightStats.varioMax = _MAX(context.flightStats.varioMax, context.varioState.speedVertLazy);
	context.flightStats.varioMin = _MIN(context.flightStats.varioMin, context.varioState.speedVertLazy);
	

	// check circling
	int16_t deltaHeading = context.varioState.heading - context.varioState.headingLast;

	if (deltaHeading > 180)
		deltaHeading = deltaHeading - 360;
	if (deltaHeading < -180)
		deltaHeading = deltaHeading + 360;

	//context.flightState.deltaHeading_AVG += (int16_t)((deltaHeading - context.flightState.deltaHeading_AVG) * DAMPING_FACTOR_DELTA_HEADING);
	context.flightState.deltaHeading_AVG = deltaHeading;
	context.flightState.deltaHeading_SUM += context.flightState.deltaHeading_AVG;
	context.flightState.deltaHeading_SUM = _CLAMP(context.flightState.deltaHeading_SUM, -450, 450); // one and a half turns

	if (abs(context.flightState.deltaHeading_AVG) < THRESHOLD_CIRCLING_HEADING)	
		context.flightState.glidingCount = _MIN(MAX_GLIDING_COUNT, context.flightState.glidingCount + 1);
	else
		context.flightState.glidingCount = _MAX(MIN_GLIDING_COUNT, context.flightState.glidingCount - 1);

	switch (context.flightState.flightMode)
	{
	case FMODE_FLYING :
		if (abs(context.flightState.deltaHeading_SUM) > 360)
		{
			startCircling();
		}
		else if (context.flightState.glidingCount > GLIDING_START_COUNT)
		{
			startGliding();
		}
		break;

	case FMODE_CIRCLING :
		// update time & gain
		context.flightState.circlingTime = nmeaParser.getDateTime() - context.flightState.circlingStartTime;
		context.flightState.circlingGain = nmeaParser.getAltitude() - context.flightState.circlingStartPos.alt;

		// checking exit
		if (context.flightState.glidingCount > CIRCLING_EXIT_COUNT)
		{
			// CIRCLING --> FLYING(NORMAL)
			stopCircling();
		}
		break;

	case FMODE_GLIDING :
		// update gliding ratio(L/D)
		{
			float dist = GET_DISTANCE(context.flightState.glidingStartPos.lat, context.flightState.glidingStartPos.lon,
				context.varioState.latitude, context.varioState.longitude);

			if (dist > 100)
			{
				if (context.flightState.glidingStartPos.alt > context.varioState.altitudeGPS)
				{
					float diff = context.flightState.glidingStartPos.alt - context.varioState.altitudeGPS;
					context.flightState.glideRatio = dist / diff;
				}
				else
				{
					context.flightState.glideRatio = 0; // INF
				}
			}
		}

		// checking exit
		if (context.flightState.glidingCount < GLIDING_EXIT_COUNT)
		{
			// GLIDING --> FLYING(NORMAL)
			stopGliding();
		}
		break;
	}
}

void stopFlight()
{
	//
	context.flightState.flightMode = FMODE_READY;

	//
	context.flightState.bearingTakeoff = -1;
	
	// play landing melody
	//beeper.setMelody(&melodyLanding[0], sizeof(melodyLanding) / sizeof(melodyLanding[0]), 1, context.volume.effect/*, PLAY_PREEMPTIVE*/);
	beeper.setMelody(NOTE_C3, 1000, 80, 1, context.volume.effect, PLAY_PREEMPTIVE);

	//
	btMan.stopLogging();
	
	// stop logging & change mode
	if (logger.isLogging())
	{
		logger.end(nmeaParser.getDateTime());
		context.deviceState.statusSDCard = 1;
	}

	//
	//display.attachScreen(scrnMan.getStatisticScreen());	
	scrnMan.showStatisticScreen(display);

	// restore volume if it is turn on start flying
	if (context.volume.autoTurnOn)
	{
		context.volume.vario = context.volume.varioDefault;
		context.volume.effect = context.volume.effectDefault;
	}
}

void startCircling()
{
	// start of circling
	context.flightState.flightMode = FMODE_CIRCLING;

	// save circling state
	context.flightState.circlingStartTime = nmeaParser.getDateTime();
	context.flightState.circlingStartPos.lon = nmeaParser.getLongitude();
	context.flightState.circlingStartPos.lat = nmeaParser.getLatitude();
	context.flightState.circlingStartPos.alt = nmeaParser.getAltitude();
	context.flightState.circlingIncline = -1;

	context.flightState.circlingTime = 0;
	context.flightState.circlingGain = 0;

	context.flightState.glidingCount = 0;

	//
	//display.attachScreen(scrnMan.getCirclingScreen());
	scrnMan.showCirclingScreen(display);
}

void startGliding()
{
	// start of circling
	context.flightState.flightMode = FMODE_GLIDING;

	//
	context.flightState.glidingStartPos.lon = nmeaParser.getLongitude();
	context.flightState.glidingStartPos.lat = nmeaParser.getLatitude();
	context.flightState.glidingStartPos.alt = nmeaParser.getAltitude();

	context.flightState.glideRatio = -1;
}

void stopCircling()
{
	// now normal flying
	context.flightState.flightMode = FMODE_FLYING;

	// update flight-statistics : thermaling count, max-gain
	if (context.flightState.circlingGain > 10 && context.flightState.circlingTime > 0)
	{
		context.flightStats.totalThermaling += 1;
		context.flightStats.thermalingMaxGain = _MAX(context.flightStats.thermalingMaxGain, context.flightState.circlingGain);
	}

	context.flightState.deltaHeading_AVG = 0;
	context.flightState.deltaHeading_SUM = 0;

	//
	//display.attachScreen(scrnMan.getActiveScreen());
	scrnMan.showActiveScreen(display);
}

void stopGliding()
{
	// now normal flying
	context.flightState.flightMode = FMODE_FLYING;

	//
	context.flightState.glideRatio = -1;
}


void goDeepSleep()
{
	//
	btMan.stopLogging();
	btMan.end();

	// close logging-file
	logger.end(nmeaParser.getDateTime());
	//
	//beeper.setMelody(NOTE_B2, 400, 50, 3, context.volume.effect, PLAY_PREEMPTIVE);

	//
	TaskWatchdog::remove(NULL);
	TaskWatchdog::end();

	// sleep display & device
	display.deepSleep();
	while(1);
}

void resetDevice()
{
	//
	display.showPopup(NULL);

	//
	btMan.stopLogging();
	btMan.end();

	// close logging-file
	logger.end(nmeaParser.getDateTime());

	// turn-off peripherals
	setPinState(&powerPins[0], PIN_STATE_INACTIVE);
	setPinState(&powerPins[1], PIN_STATE_INACTIVE);
	delay(1000); // delay will trigger Task watchdog!

	// or manually reset
	ESP.restart();
}

void makeTopMenu()
{
//	popupTopMenu.addItem(TMID_SHOW_PREFERENCE, 0 /* IDS_BASIC_SETTINGS */);
	popupTopMenu.addItem(TMID_TOGGLE_WEBSERVICE, 0 /* IDS_WEBSERVICE_ONOFF */);
	popupTopMenu.addItem(TMID_TOGGLE_SOUND, 0 /* IDS_SOUND_ONOFF */);
	popupTopMenu.addItem(TMID_TOGGLE_BLUETOOTH, 0 /* IDS_BLUETOOTH_ONOFF */);
	popupTopMenu.addItem(TMID_SIMULATION_MODE, 0 /* IDS_SIMULATION_MODE */);
	popupTopMenu.addItem(TMID_RESET_DEVICE, 0 /* IDS_RESET_DEVICE */);
	popupTopMenu.addItem(TMID_POWER_OFF, 0 /* IDS_POWER_OFF */);
}

void processKey(int key)
{
	//Serial.print("key = "); Serial.println(key);
	DisplayObject * dispObject = display.getActiveObject();
	if (dispObject)
	{
		uint32_t ret = dispObject->processKey(key);
		uint32_t cmd = GET_COMMAND(ret);
		//Serial.printf("[D] Cmd = %X\n", cmd);
		
		switch (cmd)
		{
		// from main screen
		case CMD_SHOW_NEXT_PAGE :
			//display.attachScreen(scrnMan.getNextScreen());
			scrnMan.showNextActiveScreen(display);
			break;
		case CMD_SHOW_PREV_PAGE :
			//display.attachScreen(scrnMan.getPrevScreen());
			scrnMan.showPrevActiveScreen(display);
			break;
		case CMD_SHOW_TOP_MENU :
			// enter menu
			//if (context.flightState.flightMode == FMODE_READY)
				display.showPopup(&popupTopMenu);
			break;

		case CMD_SAVE_SCREENSHOT :
			// save current screen to file
			display.saveScreenShot();
			break;
		case CMD_TOOGLE_SOUND :
			toggleVarioSound();
			break;

		// from top-menu
		case CMD_LEAVE_TOPMENU :
			// releave menu
			display.showPopup(NULL);
		
			switch (GET_PARAM(ret))
			{
			case TMID_SHOW_PREFERENCE : // show preference
				break;

			case TMID_TOGGLE_WEBSERVICE : // start web-service
				startWebService();
				break;

			case TMID_TOGGLE_SOUND : // toggle sound
				toggleVarioSound();
				break;
				
			case TMID_TOGGLE_BLUETOOTH : // toggle bluetooth
				toggleBTService();
				break;
				
			case TMID_POWER_OFF : // turn-off device
				// confirm ?
				// ...
				
				// stop logging
				// close alarm				
				// and then go to deep sleep
				goDeepSleep();
				while(1);

			case TMID_SIMULATION_MODE : // toggle simulation mode
				toggleSimulation();
				break;
				
			case TMID_RESET_DEVICE : // restart(reset) device
				resetDevice();
				break;
			}
			break;

		case CMD_LEAVE_WEBSERVICE :
			resetDevice();
			break;
		}
	}
}

void startVario()
{
	//
	makeTopMenu();

	//display.attachScreen(scrnMan.getActiveScreen());
	display.wakeupConfirmed();

	scrnMan.loadScreens();
	scrnMan.showActiveScreen(display);

	//
	//logger.init();
	//context.deviceState.statusSDCard = logger.isInitialized() ? 1 : 0;
	context.deviceState.statusSDCard = SD_CARD.valid() ? 1 : 0;

	//
	battery.begin();
	context.deviceState.batteryPower = battery.getVoltage();
	//
	if (context.deviceDefault.enableBT)
		serialBluetooth.begin(context.deviceDefault.btName);
	context.deviceState.statusBT = context.deviceDefault.enableBT ? 1 : 0;

	//
	vario.begin(context.kalman.varZMeas, context.kalman.varZAccel, context.kalman.varAccelBias);

	context.varioState.speedVertActive = vario.getVelocity();
	context.varioState.speedVertLazy = context.varioState.speedVertActive; // context.varioState.speedVertLazy + (context.varioState.speedVertActive - context.varioState.speedVertLazy) * context.varioSetting.dampingFactor;
	
	deviceMode = DEVICE_MODE_VARIO;
	//varioMode = VARIO_MODE_INIT;
	deviceTick = millis();
}

void loadPreferences()
{
	#if USE_OLD_METHOD
	Preferences pref;
	pref.begin("vario", false);
	context.load(pref);
	context.dump();
	pref.end();
	#else
	File file = SPIFFS.open("/config.json");
	if (file)
	{
		const size_t capacity = JSON_OBJECT_SIZE(31) + 700;
		DynamicJsonDocument doc(capacity);

		DeserializationError error = deserializeJson(doc, file);
		file.close();

		if (! error)
			context.set(doc);

		context.dump();
	}	
	#endif
}

void savePreferences()
{
	#if USE_OLD_METHOD
	Preferences pref;
	pref.begin("vario", false);
	context.save(pref);
	pref.end();
	#else
	#endif
}

void startWebService()
{
	WebService.start();

	TaskWatchdog::reset();
	TaskWatchdog::remove(NULL);

	deviceMode = DEVICE_MODE_WEB_SERVICE;

	// activate web-service popup
	display.showPopup(&popupWebService);

	// local loop
	while(1)
	{
		//
		keybd.update();

		int key = keybd.getch();
		if (key >= 0)
			processKey(key);

		//
		WebService.update();
	}
}

void toggleVarioSound()
{
	context.volume.effectDefault = context.volume.varioDefault = context.volume.effect = context.volume.vario = context.volume.vario ? 0 : 100;

	if (! context.volume.vario)
	{
		// silent if not
		beeper.setMuteMelody();
	}
	else
	{
		// beep beep
		beeper.setMelody(NOTE_C4, 600, 60, 2, context.volume.effect, PLAY_PREEMPTIVE);
	}
}

void toggleBTService()
{
	context.deviceDefault.enableBT = context.deviceState.statusBT = context.deviceState.statusBT ? 0 : 1;

	if (context.deviceState.statusBT)
		serialBluetooth.begin(context.deviceDefault.btName);
	else
		serialBluetooth.end();
}

void toggleSimulation()
{
	context.deviceDefault.enableSimulation = context.deviceDefault.enableSimulation ? 0 : 1;

	nmeaParser.enableSimulation(context.deviceDefault.enableSimulation ? true : false);
}

void setTimeZone(long offset, int daylight)
{
    char cst[17] = {0};
    char cdt[17] = "DST";
    char tz[33] = {0};

    if(offset % 3600){
        sprintf(cst, "UTC%ld:%02u:%02u", offset / 3600, abs((offset % 3600) / 60), abs(offset % 60));
    } else {
        sprintf(cst, "UTC%ld", offset / 3600);
    }
    if(daylight != 3600){
        long tz_dst = offset - daylight;
        if(tz_dst % 3600){
            sprintf(cdt, "DST%ld:%02u:%02u", tz_dst / 3600, abs((tz_dst % 3600) / 60), abs(tz_dst % 60));
        } else {
            sprintf(cdt, "DST%ld", tz_dst / 3600);
        }
    }
    sprintf(tz, "%s%s", cst, cdt);
    setenv("TZ", tz, 1);
    tzset();
}
