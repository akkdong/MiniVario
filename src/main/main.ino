// main.ino
//

#include <sys/time.h>

#include "VarioDisplay.h"
#include "BatteryVoltage.h"
#include "Keyboard.h"
#include "DeviceDefines.h"
#include "VarioBeeper.h"
#include "BluetoothSerialEx.h"
#include "BluetoothCallback.h"
#include "NmeaParserEx.h"
#include "VarioSentence.h"
#include "VarioLogger.h"
#include "BluetoothMan.h"
#include "HardReset.h"
#include "Util.h"
#include "AGL.h"


#define USE_KALMAN_VARIO 	1

#if USE_KALMAN_VARIO
#include "KalmanVario.h"
#else
#include "KalmanSkyDrop.h"
#endif
#include "VerticalSpeedCalculator.h"


#define DAMPING_FACTOR_DELTA_HEADING			0.1
#define THRESHOLD_CIRCLING_HEADING		6


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
	{ 39, PIN_MODE_INPUT, PIN_ACTIVE_LOW, PIN_STATE_INACTIVE },
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
	{ 262, 1000 / 4 }, 
	{ 196, 1000 / 8 }, 
	{ 196, 1000 / 8 }, 
	{ 220, 1000 / 4 }, 
	{ 196, 1000 / 4 }, 
	{   0, 1000 / 4 }, 
	{ 247, 1000 / 4 }, 
	{ 262, 1000 / 4 },
	{   0, 1000 / 8 }, 
};
static Tone melodyTakeOff[] =
{
	{ 262, 1000 / 4 }, 
	{ 196, 1000 / 8 }, 
	{ 196, 1000 / 8 }, 
	{ 220, 1000 / 4 }, 
	{ 196, 1000 / 4 }, 
	{   0, 1000 / 4 }, 
	{ 247, 1000 / 4 }, 
	{ 262, 1000 / 4 },
	{   0, 1000 / 8 }, 
};

static Tone melodyLanding[] =
{
	{ 262, 1000 / 4 }, 
	{ 196, 1000 / 8 }, 
	{ 196, 1000 / 8 }, 
	{ 220, 1000 / 4 }, 
	{ 196, 1000 / 4 }, 
	{   0, 1000 / 4 }, 
	{ 247, 1000 / 4 }, 
	{ 262, 1000 / 4 },
	{   0, 1000 / 8 }, 
};


//
//
//

uint8_t deviceMode = DEVICE_MODE_UNDEF;
//uint8_t	varioMode; 		// sub-mode of vario-mode

static RTC_DATA_ATTR uint32_t bootCount = 0;

uint32_t deviceTick;	// global tick-count
uint32_t modeTick;		// mode-specific tick-count


#define MAX_PAGES		(4)

VarioScreen pages[MAX_PAGES];
PopupMenu	topMenu;
uint8_t		activePage = 1;


DeviceContext & context = __DeviceContext;


//
//
//

BluetoothSerialEx  	serialBluetooth;


//
//
//

SineGenerator toneGen;
TonePlayer tonePlayer(toneGen);

VarioBeeper varioBeeper(tonePlayer);

VarioDisplayDriver driver(ePaperPins);
VarioDisplay display(driver, context);

BatteryVoltage battery;
Keyboard keybd(keybdPins, sizeof(keybdPins) / sizeof(keybdPins[0]));

//
//
//

NmeaParserEx nmeaParser(Serial2);
VarioSentence varioNmea(VARIOMETER_DEFAULT_NMEA_SENTENCE);
VarioLogger logger;

//
//
//

BluetoothManEx btMan(serialBluetooth, nmeaParser, varioNmea);


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

void updateVarioState();
void updateFlightState();

void goDeepSleep();
void resetDevice();
void loadPages(VarioScreen * pages);
void makeTopMenu();
void processKey(int key);
void startVario();
void loadPreferences();
void savePreferences();


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

	// power on peripherals : default is power on
	initPins(powerPins, sizeof(powerPins) / sizeof(powerPins[0]));
	delay(20);
	
	// initialize communication channels
	Serial.begin(115200);
	Serial2.begin(9600);
	Wire.begin();

	Serial.println("Start MiniVario...\n");
	Serial.printf( "  reboot: %d\n", ++bootCount);

	// load last device-context
	loadPreferences();
	
	//
	keybd.begin();
	//
	display.begin(deviceMode == DEVICE_MODE_WAKEUP);
	//
	toneGen.begin(SineGenerator::USE_DIFFERENTIAL, SineGenerator::SCALE_FULL, 0);
	tonePlayer.setVolume(context.volume.vario);
	//tonePlayer.setBeep(NOTE_C4, 800, 500, 2, 100);

	//
	btMan.begin();

	// start right now or ...
	if (deviceMode == DEVICE_MODE_VARIO)
		startVario();

	//
	deviceTick = millis();
}

void loop()
{
	// check key-input
	keybd.update();
	
	// beep beep beep!
	tonePlayer.update();

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
		context.varioState.speedVertLazy = varioCalculator.mVertSpeedAVG3; // 1s average
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

		if (context.deviceDefault.enableSound)
			varioBeeper.setVelocity(context.varioState.speedVertActive);

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
	
	// read & prase gps sentence
	nmeaParser.update();

	// send any prepared sentence to BT
	btMan.update();	

	// GPS may be 1Hz : execute every second
	if (nmeaParser.dataReady())
	{
		//
		context.deviceState.statusGPS = nmeaParser.isFixed() ? 1 : 0;

		// update device-context
		if (nmeaParser.isFixed())
		{
			updateVarioState();

			if (deviceMode == DEVICE_MODE_VARIO)
			{
				readyFlight();
			}
			else // if (deviceMode == DEVICE_MODE_VARIO_AND_GPS)
			{
				if (context.flightState.flightMode == FMODE_READY)
				{
					if (nmeaParser.getSpeed() > FLIGHT_START_MIN_SPEED)
					{
						startFlight();
					}
				}
				else
				{
					updateFlightState();
				}

				if (nmeaParser.getSpeed() < FLIGHT_START_MIN_SPEED)
				{
					if ((millis() - modeTick) > FLIGHT_LANDING_THRESHOLD)
					{
						stopFlight();
					}
				}
				else
				{
					// reset modeTick
					modeTick = millis();
				}
			}
		}
		else
		{
			if (deviceMode == DEVICE_MODE_VARIO_AND_GPS)
			{
				if (context.flightState.flightMode != FMODE_READY)
				{
					stopFlight();
				}

				deviceMode = DEVICE_MODE_VARIO;
			}
		}

		// check logging state
		if (logger.isLogging())
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

		nmeaParser.resetReady();
	}

	//
	if (battery.update())
		context.deviceState.batteryPower = battery.getVoltage();
	
	//
	int key = keybd.getch();
	if (key >= 0)
		processKey(key);

	//
	//yield();
}

void readyFlight()
{
	// do position calibration
	vario.calibrateAltitude(nmeaParser.getAltitude());
	vario.calculateSeaLevel(nmeaParser.getAltitude());

	//
	struct timeval now = { nmeaParser.getDateTime(), 0 };
	now.tv_sec +=  (__DeviceContext.logger.timezone * 60 * 60); // GMT(UTC+00) --> Local time
	settimeofday(&now, NULL);

	// play ready melody~~~
	tonePlayer.setMelody(&melodyVarioReady[0], sizeof(melodyVarioReady) / sizeof(melodyVarioReady[0]), 1, PLAY_PREEMPTIVE, context.volume.effect);				
	
	// now ready to fly~~~
	deviceMode = DEVICE_MODE_VARIO_AND_GPS;

	context.flightState.flightMode = FMODE_READY;
}

void startFlight()
{
	//
	if (context.volume.autoTurnOn)
		context.deviceDefault.enableSound = 1;
	
	// play take-off melody
	tonePlayer.setMelody(&melodyTakeOff[0], sizeof(melodyTakeOff) / sizeof(melodyTakeOff[0]), 1, PLAY_PREEMPTIVE, context.volume.effect);

	//
	btMan.startLogging(nmeaParser.getDateTime());
	
	// start logging & change mode
	if (logger.begin(nmeaParser.getDateTime()))
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

	context.flightStats.altitudeMax = context.flightStats.altitudeMin = context.varioState.altitudeGPS;

	// set mode-tick
	modeTick = millis();	
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

	if (context.flightState.flightMode == FMODE_FLYING)
	{
		if (abs(context.flightState.deltaHeading_SUM) > 360)
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
		}
	}
	else if (context.flightState.flightMode == FMODE_CIRCLING)
	{
		// update time & gain
		context.flightState.circlingTime = nmeaParser.getDateTime() - context.flightState.circlingStartTime;
		context.flightState.circlingGain = nmeaParser.getAltitude() - context.flightState.circlingStartPos.alt;

		// check circling
		if (abs(context.flightState.deltaHeading_AVG) < THRESHOLD_CIRCLING_HEADING)
		{
			context.flightState.glidingCount = _MIN(6, context.flightState.glidingCount + 1);

			if (context.flightState.glidingCount >= 5)
			{
				//
				context.flightState.flightMode = FMODE_FLYING;

				// update flight-statistics : thermaling count, max-gain
				if (context.flightState.circlingGain > 10 && context.flightState.circlingTime > 0)
				{
					context.flightStats.totalThermaling += 1;
					context.flightStats.thermalingMaxGain = _MAX(context.flightStats.thermalingMaxGain, context.flightState.circlingGain);
				}

				context.flightState.deltaHeading_AVG = 0;
				context.flightState.deltaHeading_SUM = 0;
				context.flightState.circlingStartTime = 0;
				context.flightState.glidingCount = 0;
			}
		}
		else
		{
			// 
			context.flightState.glidingCount = _MAX(0, context.flightState.glidingCount - 1);
		}					
	}
	else if (context.flightState.flightMode == FMODE_GLIDING)
	{

	}
}

void stopFlight()
{
	//
	context.flightState.flightMode = FMODE_READY;

	//
	context.flightState.bearingTakeoff = -1;
	
	// play landing melody
	tonePlayer.setMelody(&melodyLanding[0], sizeof(melodyLanding) / sizeof(melodyLanding[0]), 1, PLAY_PREEMPTIVE, context.volume.effect);

	//
	btMan.stopLogging();
	
	// stop logging & change mode
	if (logger.isLogging())
	{
		logger.end(nmeaParser.getDateTime());
		context.deviceState.statusSDCard = 1;
	}	
}

void goDeepSleep()
{
	//
	btMan.stopLogging();
	btMan.end();

	// close logging-file
	logger.end(nmeaParser.getDateTime());
	//
	toneGen.end();

	// sleep display & device
	display.deepSleep();
	while(1);
}

void resetDevice()
{
	//
	btMan.stopLogging();
	btMan.end();

	// close logging-file
	logger.end(nmeaParser.getDateTime());
	//
	toneGen.end();

	// turn-off peripherals
	setPinState(&powerPins[0], PIN_STATE_INACTIVE);
	setPinState(&powerPins[1], PIN_STATE_INACTIVE);
	delay(100);

	//
	#if 1
	ESP.restart();
	#else
	HardReset();
	#endif
}

void loadPages(VarioScreen * pages)
{
	// config pages <-- load page information from somewhere....
	//
	// below is just a test
	int widget = 0;
	
	// Screen: 176 x 264
	//
	// StatusBar: 176 x 24
	// Text-Box small: 88 x 52
	// Text-Box normal: 176 x 64
	//
	// 240 / 4 = 60, 240 / 5 = 48
	//
	int x = 0, y = 0;
	
#define STATUS_TIME_HEIGHT	24
#define STATUS_TIME_WIDTH	40
#define TEXTBOX_S_HEIGHT	56
#define TEXTBOX_S_WIDTH		88

#define SCREEN_WIDTH		176
#define SCREEN_HEIGHT		264


#define NORMAL_STATUS		(WS_FONT_NORMAL_2 | WS_TA_CENTER | WS_TA_MIDDLE)
#define NORMAL_TEXT			(WS_FONT_BOLD_3 | WS_TA_RIGHT | WS_TA_BOTTOM)
#define NORMAL_BOX			(WS_BORDER_LEFT | WS_BORDER_TOP)

#if 0	
	pages[0].widget[widget].setStyle(Widget_StatusBar, 0, WidgetContent_Status_Bar);
	pages[0].widget[widget].setPosition(x, y, SCREEN_WIDTH, STATUS_TIME_HEIGHT);
	widget++;
	
	pages[0].widget[widget].setStyle(Widget_SimpleText, NORMAL_STATUS, WidgetContent_Battery);
	pages[0].widget[widget].setPosition(x + SCREEN_WIDTH - STATUS_TIME_WIDTH - STATUS_TIME_WIDTH, y, STATUS_TIME_WIDTH, STATUS_TIME_HEIGHT);
	widget++;
	
	pages[0].widget[widget].setStyle(Widget_SimpleText, NORMAL_STATUS, WidgetContent_Time_Current);
	pages[0].widget[widget].setPosition(x + SCREEN_WIDTH - STATUS_TIME_WIDTH, y, STATUS_TIME_WIDTH, STATUS_TIME_HEIGHT);
	widget++;
#endif	
	y += STATUS_TIME_HEIGHT;
	
	pages[0].getWidget(widget)->setStyle(Widget_TextBox, NORMAL_TEXT | NORMAL_BOX, WidgetContent_Altitude_GPS);
	pages[0].getWidget(widget)->setPosition(x, y, TEXTBOX_S_WIDTH, TEXTBOX_S_HEIGHT);
	widget++;
	x += TEXTBOX_S_WIDTH;

	pages[0].getWidget(widget)->setStyle(Widget_TextBox, NORMAL_TEXT | NORMAL_BOX | WS_BORDER_RIGHT, WidgetContent_Speed_Ground);
	pages[0].getWidget(widget)->setPosition(x, y, TEXTBOX_S_WIDTH, TEXTBOX_S_HEIGHT);
	widget++;
	x = 0;
	y += TEXTBOX_S_HEIGHT;

	pages[0].getWidget(widget)->setStyle(Widget_TextBox, NORMAL_TEXT | NORMAL_BOX, WidgetContent_Heading);
	pages[0].getWidget(widget)->setPosition(x, y, TEXTBOX_S_WIDTH, TEXTBOX_S_HEIGHT);
	widget++;
	x += TEXTBOX_S_WIDTH;
 
	pages[0].getWidget(widget)->setStyle(Widget_TextBox, NORMAL_TEXT | NORMAL_BOX | WS_BORDER_RIGHT, WidgetContent_Vario_Lazy);
	pages[0].getWidget(widget)->setPosition(x, y, TEXTBOX_S_WIDTH, TEXTBOX_S_HEIGHT);
	widget++;
	x = 0;
	y += TEXTBOX_S_HEIGHT;
	
//	pages[0].getWidget(widget)->setStyle(Widget_TextBox, NORMAL_TEXT | NORMAL_BOX | WS_BORDER_BOTTOM, WidgetContent_Altitude_Baro);
	pages[0].getWidget(widget)->setStyle(Widget_TextBox, NORMAL_TEXT | NORMAL_BOX, WidgetContent_Altitude_Baro);
	pages[0].getWidget(widget)->setPosition(x, y, TEXTBOX_S_WIDTH, TEXTBOX_S_HEIGHT);
	widget++;
	x += TEXTBOX_S_WIDTH;

//	pages[0].getWidget(widget)->setStyle(Widget_TextBox, NORMAL_TEXT | NORMAL_BOX | WS_BORDER_RIGHT | WS_BORDER_BOTTOM, WidgetContent_Temperature);
	pages[0].getWidget(widget)->setStyle(Widget_TextBox, NORMAL_TEXT | NORMAL_BOX | WS_BORDER_RIGHT, WidgetContent_Temperature);
	pages[0].getWidget(widget)->setPosition(x, y, TEXTBOX_S_WIDTH, TEXTBOX_S_HEIGHT);
	widget++;
	x = 0;
	y += TEXTBOX_S_HEIGHT;

	pages[0].getWidget(widget)->setStyle(Widget_Compass, NORMAL_BOX | WS_BORDER_BOTTOM, WidgetContent_Heading);
	pages[0].getWidget(widget)->setPosition(x, y, SCREEN_HEIGHT - y, SCREEN_HEIGHT - y);
	widget++;
	x += SCREEN_HEIGHT - y;

	pages[0].getWidget(widget)->setStyle(Widget_VarioHistory, NORMAL_BOX | WS_BORDER_RIGHT | WS_BORDER_BOTTOM, WidgetContent_Vario_History);
	pages[0].getWidget(widget)->setPosition(x, y, SCREEN_WIDTH - x, SCREEN_HEIGHT - y);

	//
#define W_W		(SCREEN_WIDTH/2)
#define W_H		(60)

	widget = 0;
	x = 0, y = STATUS_TIME_HEIGHT;

	pages[1].getWidget(widget)->setStyle(Widget_VarioBar, NORMAL_BOX, WidgetContent_Vario_Lazy);
	pages[1].getWidget(widget)->setPosition(x, y, 48, W_H + W_H);
	widget++;
	x += 48;

	pages[1].getWidget(widget)->setStyle(Widget_Compass, NORMAL_BOX, WidgetContent_Heading);
	pages[1].getWidget(widget)->setPosition(x, y, 52, W_H);
	widget++;
	x += 52;

	pages[1].getWidget(widget)->setStyle(Widget_TextBox, NORMAL_TEXT | NORMAL_BOX | WS_BORDER_RIGHT, WidgetContent_Speed_Ground);
	pages[1].getWidget(widget)->setPosition(x, y, SCREEN_WIDTH - x, W_H);
	widget++;
	x = 48; y += W_H;

	pages[1].getWidget(widget)->setStyle(Widget_TextBox, NORMAL_TEXT | NORMAL_BOX | WS_BORDER_RIGHT, WidgetContent_Altitude_GPS);
	pages[1].getWidget(widget)->setPosition(x, y, SCREEN_WIDTH - x, W_H);
	widget++;
	x = 0; y += W_H;

	pages[1].getWidget(widget)->setStyle(Widget_TextBox, NORMAL_TEXT | NORMAL_BOX, WidgetContent_Vario_Lazy);
	pages[1].getWidget(widget)->setPosition(x, y, W_W, W_H);
	widget++;
	x += W_W;

	pages[1].getWidget(widget)->setStyle(Widget_TextBox, NORMAL_TEXT | NORMAL_BOX | WS_BORDER_RIGHT, WidgetContent_Time_Flight);
	pages[1].getWidget(widget)->setPosition(x, y, W_W, W_H);
	widget++;
	x = 0; y += W_H;

	pages[1].getWidget(widget)->setStyle(Widget_TextBox, NORMAL_TEXT | NORMAL_BOX | WS_BORDER_BOTTOM, WidgetContent_Altitude_Ref1);
	pages[1].getWidget(widget)->setPosition(x, y, W_W, W_H);
	widget++;
	x += W_W;

	pages[1].getWidget(widget)->setStyle(Widget_TextBox, NORMAL_TEXT | NORMAL_BOX | WS_BORDER_RIGHT | WS_BORDER_BOTTOM, WidgetContent_Temperature);
	pages[1].getWidget(widget)->setPosition(x, y, W_W, W_H);


	//
#define MIN_H	48
	widget = 0;
	x = 0, y = STATUS_TIME_HEIGHT;

	pages[2].getWidget(widget)->setStyle(Widget_TextBox, NORMAL_TEXT | NORMAL_BOX, WidgetContent_Altitude_GPS);
	pages[2].getWidget(widget)->setPosition(x, y, TEXTBOX_S_WIDTH, MIN_H);
	widget++;
	x += TEXTBOX_S_WIDTH;

	pages[2].getWidget(widget)->setStyle(Widget_TextBox, NORMAL_TEXT | NORMAL_BOX | WS_BORDER_RIGHT, WidgetContent_Speed_Ground);
	pages[2].getWidget(widget)->setPosition(x, y, TEXTBOX_S_WIDTH, MIN_H);
	widget++;
	x = 0;
	y += MIN_H;

	pages[2].getWidget(widget)->setStyle(Widget_TextBox, NORMAL_TEXT | NORMAL_BOX, WidgetContent_Ground_Level);
	pages[2].getWidget(widget)->setPosition(x, y, TEXTBOX_S_WIDTH, MIN_H);
	widget++;
	x += TEXTBOX_S_WIDTH;

	pages[2].getWidget(widget)->setStyle(Widget_TextBox, NORMAL_TEXT | NORMAL_BOX | WS_BORDER_RIGHT, WidgetContent_Vario_Lazy);
	pages[2].getWidget(widget)->setPosition(x, y, TEXTBOX_S_WIDTH, MIN_H);
	widget++;
	x = 0;
	y += MIN_H;

	pages[2].getWidget(widget)->setStyle(Widget_TextBox, NORMAL_TEXT | NORMAL_BOX, WidgetContent_Heading);
	pages[2].getWidget(widget)->setPosition(x, y, TEXTBOX_S_WIDTH, MIN_H);
	widget++;
	x += TEXTBOX_S_WIDTH;

	pages[2].getWidget(widget)->setStyle(Widget_TextBox, NORMAL_TEXT | NORMAL_BOX | WS_BORDER_RIGHT, WidgetContent_Bearing_Takeoff);
	pages[2].getWidget(widget)->setPosition(x, y, TEXTBOX_S_WIDTH, MIN_H);
	widget++;
	x = 0;
	y += MIN_H;

	pages[2].getWidget(widget)->setStyle(Widget_TextBox, NORMAL_TEXT | NORMAL_BOX, WidgetContent_Thermaling_Gain);
	pages[2].getWidget(widget)->setPosition(x, y, TEXTBOX_S_WIDTH, MIN_H);
	widget++;
	x += TEXTBOX_S_WIDTH;

	pages[2].getWidget(widget)->setStyle(Widget_TextBox, NORMAL_TEXT | NORMAL_BOX | WS_BORDER_RIGHT, WidgetContent_Thermaling_Time);
	pages[2].getWidget(widget)->setPosition(x, y, TEXTBOX_S_WIDTH, MIN_H);
	widget++;
	x = 0;
	y += MIN_H;

	pages[2].getWidget(widget)->setStyle(Widget_TextBox, NORMAL_TEXT | NORMAL_BOX | WS_BORDER_BOTTOM, WidgetContent_Distance_Takeoff);
	pages[2].getWidget(widget)->setPosition(x, y, TEXTBOX_S_WIDTH, MIN_H);
	widget++;
	x += TEXTBOX_S_WIDTH;

	pages[2].getWidget(widget)->setStyle(Widget_TextBox, NORMAL_TEXT | NORMAL_BOX | WS_BORDER_RIGHT | WS_BORDER_BOTTOM, WidgetContent_Distance_Flight);
	pages[2].getWidget(widget)->setPosition(x, y, TEXTBOX_S_WIDTH, MIN_H);
	widget++;
	x = 0;
	y += MIN_H;

	//
	widget = 0;
	x = 0, y = STATUS_TIME_HEIGHT;

	pages[3].getWidget(widget)->setStyle(Widget_TextBox, NORMAL_TEXT | NORMAL_BOX, WidgetContent_Altitude_GPS);
	pages[3].getWidget(widget)->setPosition(x, y, TEXTBOX_S_WIDTH, MIN_H);
	widget++;
	x += TEXTBOX_S_WIDTH;

	pages[3].getWidget(widget)->setStyle(Widget_TextBox, NORMAL_TEXT | NORMAL_BOX | WS_BORDER_RIGHT, WidgetContent_Speed_Ground);
	pages[3].getWidget(widget)->setPosition(x, y, TEXTBOX_S_WIDTH, MIN_H);
	widget++;
	x = 0;
	y += MIN_H;

	pages[3].getWidget(widget)->setStyle(Widget_TextBox, NORMAL_TEXT | NORMAL_BOX, WidgetContent_Ground_Level);
	pages[3].getWidget(widget)->setPosition(x, y, TEXTBOX_S_WIDTH, MIN_H);
	widget++;
	x += TEXTBOX_S_WIDTH;

	pages[3].getWidget(widget)->setStyle(Widget_TextBox, NORMAL_TEXT | NORMAL_BOX | WS_BORDER_RIGHT, WidgetContent_Altitude_AGL);
	pages[3].getWidget(widget)->setPosition(x, y, TEXTBOX_S_WIDTH, MIN_H);
	widget++;
	x = 0;
	y += MIN_H;

	pages[3].getWidget(widget)->setStyle(Widget_TextBox, NORMAL_TEXT | NORMAL_BOX, WidgetContent_Heading);
	pages[3].getWidget(widget)->setPosition(x, y, TEXTBOX_S_WIDTH, MIN_H);
	widget++;
	x += TEXTBOX_S_WIDTH;

	pages[3].getWidget(widget)->setStyle(Widget_TextBox, NORMAL_TEXT | NORMAL_BOX | WS_BORDER_RIGHT, WidgetContenxt_FlightState_glidingCount);
	pages[3].getWidget(widget)->setPosition(x, y, TEXTBOX_S_WIDTH, MIN_H);
	widget++;
	x = 0;
	y += MIN_H;

	pages[3].getWidget(widget)->setStyle(Widget_TextBox, NORMAL_TEXT | NORMAL_BOX, WidgetContenxt_FlightState_deltaHeading_AVG);
	pages[3].getWidget(widget)->setPosition(x, y, TEXTBOX_S_WIDTH, MIN_H);
	widget++;
	x += TEXTBOX_S_WIDTH;

	pages[3].getWidget(widget)->setStyle(Widget_TextBox, NORMAL_TEXT | NORMAL_BOX | WS_BORDER_RIGHT, WidgetContenxt_FlightState_deltaHeading_SUM);
	pages[3].getWidget(widget)->setPosition(x, y, TEXTBOX_S_WIDTH, MIN_H);
	widget++;
	x = 0;
	y += MIN_H;

	pages[3].getWidget(widget)->setStyle(Widget_TextBox, NORMAL_TEXT | NORMAL_BOX | WS_BORDER_BOTTOM, WidgetContent_Thermaling_Gain);
	pages[3].getWidget(widget)->setPosition(x, y, TEXTBOX_S_WIDTH, MIN_H);
	widget++;
	x += TEXTBOX_S_WIDTH;

	pages[3].getWidget(widget)->setStyle(Widget_TextBox, NORMAL_TEXT | NORMAL_BOX | WS_BORDER_RIGHT | WS_BORDER_BOTTOM, WidgetContent_Thermaling_Time);
	pages[3].getWidget(widget)->setPosition(x, y, TEXTBOX_S_WIDTH, MIN_H);
	widget++;
	x = 0;
	y += MIN_H;	
}

void makeTopMenu()
{
	topMenu.addItem(TMID_SHOW_PREFERENCE, 0 /* IDS_BASIC_SETTINGS */);
	topMenu.addItem(TMID_TOGGLE_SOUND, 0 /* IDS_SOUND_ONOFF */);
	topMenu.addItem(TMID_TOGGLE_BLUETOOTH, 0 /* IDS_BLUETOOTH_ONOFF */);
	topMenu.addItem(TMID_RESET_DEVICE, 0 /* IDS_RESET_DEVICE */);
	topMenu.addItem(TMID_POWER_OFF, 0 /* IDS_POWER_OFF */);
}

void processKey(int key)
{
	Serial.print("key = "); Serial.println(key);
	DisplayObject * dispObject = display.getActiveObject();
	if (dispObject)
	{
		uint32_t ret = dispObject->processKey(key);
		uint32_t cmd = GET_COMMAND(ret);
		Serial.print("cmd = "); Serial.println(cmd);
		
		switch (cmd)
		{
		// from main scren
		case CMD_SHOW_NEXT_PAGE :
			activePage = (activePage + 1) % MAX_PAGES;
			display.attachScreen(&pages[activePage]);
			break;
		case CMD_SHOW_PREV_PAGE :
			activePage = activePage == 0 ? (MAX_PAGES - 1) : activePage - 1;
			display.attachScreen(&pages[activePage]);
			break;
		case CMD_SHOW_TOP_MENU :
			// enter menu
			display.showPopup(&topMenu);
			break;
			
		// from top-menu
		case CMD_LEAVE_TOPMENU :
			// releave menu
			display.showPopup(NULL);
		
			switch (GET_PARAM(ret))
			{
			case TMID_SHOW_PREFERENCE : // show preference
				break;
				
			case TMID_TOGGLE_SOUND : // toggle sound
				context.deviceDefault.enableSound = context.deviceDefault.enableSound ? 0 : 1;
				if (! context.deviceDefault.enableSound)
				{
					varioBeeper.setVelocity(0);			
					toneGen.setFrequency(0);
				}
				//savePreferences();
				break;
				
			case TMID_TOGGLE_BLUETOOTH : // toggle bluetooth
				context.deviceDefault.enableBT = context.deviceState.statusBT = context.deviceState.statusBT ? 0 : 1;
				if (context.deviceState.statusBT)
				{
					serialBluetooth.register_callback(bluetoothSPPCallback);
					serialBluetooth.begin("MiniVario");
				}					
				else
				{
					serialBluetooth.register_callback(NULL);
					serialBluetooth.end();
				}
				//savePreferences();
				break;
				
			case TMID_POWER_OFF : // turn-off device
				// confirm ?
				// ...
				
				// stop logging
				// close alarm				
				// and then go to deep sleep
				goDeepSleep();
				while(1);
				
			case TMID_RESET_DEVICE : // restart(reset) device
				resetDevice();
				break;
			}
			break;
		}
	}
}

void startVario()
{
	//
	loadPages(pages);
	makeTopMenu();

	display.attachScreen(&pages[activePage]);
	display.wakeupConfirmed();

	//
	logger.init();
	context.deviceState.statusSDCard = logger.isInitialized() ? 1 : 0;
	//
	battery.begin();
	context.deviceState.batteryPower = battery.getVoltage();
	//
	if (context.deviceDefault.enableBT)
	{
		serialBluetooth.register_callback(bluetoothSPPCallback);
		serialBluetooth.begin(context.deviceDefault.btName);
	}
	context.deviceState.statusBT = context.deviceDefault.enableBT ? 1 : 0;

	//
	vario.begin();

	context.varioState.speedVertActive = vario.getVelocity();
	context.varioState.speedVertLazy = context.varioState.speedVertLazy + (context.varioState.speedVertActive - context.varioState.speedVertLazy) * context.varioSetting.dampingFactor;
	
	deviceMode = DEVICE_MODE_VARIO;
	//varioMode = VARIO_MODE_INIT;
	deviceTick = millis();
}

void loadPreferences()
{
	Preferences pref;
	pref.begin("vario", false);
	context.load(pref);
	pref.end();
}

void savePreferences()
{
	Preferences pref;
	pref.begin("vario", false);
	context.save(pref);
	pref.end();
}
