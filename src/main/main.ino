// main.ino
//

#include <sys/time.h>

#include "KalmanVario.h"
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


////////////////////////////////////////////////////////////////////////////////////////
//
//

enum _DeviceMode
{
	DEVICE_MODE_UNDEF = 0,
	DEVICE_MODE_VARIO,			// (1)
	DEVICE_MODE_UMS,			// (2)
	DEVICE_MODE_CALIBRATION,	// (3)
	DEVICE_MODE_SHUTDOWN,		// (4)
};

enum _VarioMode
{
	VARIO_MODE_INIT = 0,		// (0)
	VARIO_MODE_LANDING,			// (1)
	VARIO_MODE_FLYING,			// (2)
//	VARIO_MODE_SHUTDOWN,		// (3)
};


////////////////////////////////////////////////////////////////////////////////////////
//
	
//
PinSetting ePaperPins[] = 
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

PinSetting powerPins[] =
{
	// PWR_PERIPH
	{ 19, PIN_MODE_OUTPUT, PIN_ACTIVE_HIGH, PIN_STATE_ACTIVE },
	// PWR_SOUND
	{ 27, PIN_MODE_OUTPUT, PIN_ACTIVE_HIGH, PIN_STATE_ACTIVE },
};

PinSetting keybdPins[] =
{
	// KEY_UP
	{ 35, PIN_MODE_INPUT, PIN_ACTIVE_HIGH, PIN_STATE_ACTIVE },
	// KEY_DOWN
	{ 34, PIN_MODE_INPUT, PIN_ACTIVE_HIGH, PIN_STATE_ACTIVE },
	// KEY_SEL
	{  0, PIN_MODE_INPUT, PIN_ACTIVE_LOW, PIN_STATE_ACTIVE },
};


//
//
//

uint8_t deviceMode = DEVICE_MODE_VARIO;
uint8_t	varioMode; 		// sub-mode of vario-mode

uint32_t deviceTick;	// global tick-count
uint32_t modeTick;		// mode-specific tick-count


//
//
//

SineGenerator toneGen;
TonePlayer tonePlayer(toneGen);

VarioBeeper varioBeeper(tonePlayer);


//
//
//

CriticalSection cs;
Sensor_MS5611  baro(cs, Wire);
KalmanVario vario(baro);

DeviceContext & context = __DeviceContext;

VarioDisplayDriver driver(ePaperPins);
VarioDisplay display(driver, context);

BatteryVoltage battery;
Keyboard keybd(keybdPins, sizeof(keybdPins) / sizeof(keybdPins[0]));

VarioScreen pages[3];
PopupMenu	topMenu;

//
//
//

BluetoothSerialEx  	serialBluetooth;

//
//
//

NmeaParserEx nmeaParser(Serial2);
VarioSentence varioNmea(VARIOMETER_DEFAULT_NMEA_SENTENCE);
VarioLogger logger;

//
//
//

BluetoothMan btMan(serialBluetooth, nmeaParser, varioNmea);


//
//
//

void goDeepSleep();
void loadPages(VarioScreen * pages);
void processKey(int key);


////////////////////////////////////////////////////////////////////////////////////////
//

void setup()
{
	//
	initPins(powerPins, sizeof(powerPins) / sizeof(powerPins[0]));
	delay(100);
	
	//
	Serial.begin(115200);
	Serial2.begin(9600);
	
	if (context.device.statusBT)
	{
		serialBluetooth.register_callback(bluetoothSPPCallback);
		serialBluetooth.begin("MiniVario");
	}

	Serial.println("Start MiniVario...\n");

	//
	Wire.begin();

	
	//
	loadPages(pages);
	
	topMenu.addItem(0x5001, 0 /* IDS_BASIC_SETTINGS */);
	topMenu.addItem(0x5002, 0 /* IDS_SOUND_ONOFF */);
	topMenu.addItem(0x5003, 0 /* IDS_BLUETOOTH_ONOFF */);
	topMenu.addItem(0x5004, 0 /* IDS_POWER_OFF */);
	topMenu.addItem(0x5005, 0 /* IDS_RESET_DEVICE */);
	
	//
	logger.init();
	
	//
	toneGen.begin(SineGenerator::USE_DIFFERENTIAL, SineGenerator::SCALE_FULL, 0);
	tonePlayer.setVolume(context.volume.vario);

	//
	vario.begin();
	display.begin();
	
	display.attachScreen(&pages[0]);
	
	battery.begin();
	context.device.batteryPower = battery.getVoltage();
	
	keybd.begin();
	
	//
	//tonePlayer.setBeep(NOTE_C4, 800, 500, 2, 100);

	// just debug
	{
		struct tm tm;

		tm.tm_year = 2019 - 1900;
		tm.tm_mon = 4 - 1;
		tm.tm_mday = 1;
		tm.tm_hour = 12;
		tm.tm_min = 30;
		tm.tm_sec = 0;

		struct timeval now = { mktime(&tm), 0 };
    	settimeofday(&now, NULL);
		Serial.print("Setting time: "); Serial.println(asctime(&tm));
	}

	//
	varioMode = VARIO_MODE_INIT;
	deviceTick = millis();
}


void loop()
{
	if (vario.available())
	{
		//
		context.varioState.speedVertActive = vario.getVelocity();
		context.varioState.speedVertLazy = context.varioState.speedVertLazy * 0.8 + context.varioState.speedVertActive * 0.2;
		context.varioState.altitudeBaro = vario.getAltitude();
		context.varioState.pressure = vario.getPressure();
		context.varioState.temperature = vario.getTemperature();

		if (context.volume.vario)
			varioBeeper.setVelocity(context.varioState.speedVertActive);

		//
		{
			if (vario.getVelocity() < STABLE_SINKING_THRESHOLD || STABLE_CLIMBING_THRESHOLD < vario.getVelocity())
				deviceTick = millis(); // reset tick because it's not quiet.
			
			// if (commandReceiveFlag)
			// {
			// 	deviceTick =xx millis(); // reset tick because it's not quiet.
			// 	commandReceiveFlag = 0;
			// }
			
			if ((context.threshold.autoShutdownVario) && ((millis() - deviceTick) > context.threshold.autoShutdownVario))
				goDeepSleep();
		}			

		//
		float altitude = vario.getCalibratedAltitude(); // getCalibratedAltitude or getAltitude
		logger.update(altitude);

		// update vario sentence periodically
		if (varioNmea.checkInterval())
			varioNmea.begin(altitude, vario.getVelocity(), vario.getTemperature(), vario.getPressure(), battery.getVoltage());
		
		//
		vario.flush();
	}
	
	// beep beep beep!
	tonePlayer.update();
	
	//
	if (battery.update())
		context.device.batteryPower = battery.getVoltage();
	
	// read & prase gps sentence
	nmeaParser.update();

	// send any prepared sentence to BT
	btMan.update();	

	//if (nmeaParser.availableIGC())
	{
		context.varioState.altitudeGPS = nmeaParser.getAltitude();
		context.varioState.latitude = nmeaParser.getLatitude();
		context.varioState.longitude = nmeaParser.getLongitude();
		context.varioState.speedGround = nmeaParser.getSpeed();
		context.varioState.heading = nmeaParser.getHeading();	
// 		context.varioState.timeCurrent = nmeaParser.getDateTime();
	}	

	// IGC sentence is available when it received a valid GGA. -> altitude is valid
	if (varioMode == VARIO_MODE_INIT  && nmeaParser.availableIGC())
	{
		// do position calibration
		vario.calibrateAltitude(nmeaParser.getAltitude());
		
		// now ready to fly~~~
		varioMode = VARIO_MODE_LANDING;

		//
		struct timeval now = { nmeaParser.getDateTime(), 0 };
		now.tv_sec +=  (__DeviceContext.logger.timezone * 60 * 60); // GMT(UTC+00) --> Local time
		settimeofday(&now, NULL);

		context.varioState.timeStart = nmeaParser.getDateTime();
		context.varioState.timeFly = 0;

		// play ready melody~~~
		//tonePlayer.setMelody(&melodyVarioReady[0], sizeof(melodyVarioReady) / sizeof(melodyVarioReady[0]), 1, PLAY_PREEMPTIVE, Config.volume.effect);
	}
	else if (varioMode == VARIO_MODE_LANDING)
	{
		if (nmeaParser.getSpeed() > FLIGHT_START_MIN_SPEED && nmeaParser.getDateTime())
		{
			//
			varioMode = VARIO_MODE_FLYING;
			
			// play take-off melody
			//tonePlayer.setMelody(&melodyTakeOff[0], sizeof(melodyTakeOff) / sizeof(melodyTakeOff[0]), 1, PLAY_PREEMPTIVE, Config.volume.effect);
			
			// start logging & change mode
			logger.begin(nmeaParser.getDateTime());
			
			// set mode-tick
			modeTick = millis();
		}
	}
	else if (varioMode == VARIO_MODE_FLYING)
	{
		if (nmeaParser.getSpeed() < FLIGHT_START_MIN_SPEED)
		{
			if ((millis() - modeTick) > FLIGHT_LANDING_THRESHOLD)
			{
				//
				varioMode = VARIO_MODE_LANDING;
				
				// play landing melody
				//tonePlayer.setMelody(&melodyLanding[0], sizeof(melodyLanding) / sizeof(melodyLanding[0]), 1, PLAY_PREEMPTIVE, Config.volume.effect);
				
				// stop logging & change mode
				logger.end(nmeaParser.getDateTime());
			}
		}
		else
		{
			// reset modeTick
			modeTick = millis();

			//
			context.varioState.timeFly = nmeaParser.getDateTime() - context.varioState.timeStart;
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
			while (nmeaParser.availableIGC())
				logger.write(nmeaParser.readIGC());
		}
	}	
	
	//
	keybd.update();
	
	//
	int key = keybd.getch();
	if (key >= 0)
		processKey(key);
}


void goDeepSleep()
{
	// close logging-file
	logger.end(nmeaParser.getDateTime());

	// sleep display & device
	display.deepSleep();

	while(1);
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
	
	pages[0].widget[widget].setStyle(Widget_TextBox, NORMAL_TEXT | NORMAL_BOX, WidgetContent_Altitude_GPS);
	pages[0].widget[widget].setPosition(x, y, TEXTBOX_S_WIDTH, TEXTBOX_S_HEIGHT);
	widget++;
	x += TEXTBOX_S_WIDTH;

	pages[0].widget[widget].setStyle(Widget_TextBox, NORMAL_TEXT | NORMAL_BOX | WS_BORDER_RIGHT, WidgetContent_Speed_Ground);
	pages[0].widget[widget].setPosition(x, y, TEXTBOX_S_WIDTH, TEXTBOX_S_HEIGHT);
	widget++;
	x = 0;
	y += TEXTBOX_S_HEIGHT;

	pages[0].widget[widget].setStyle(Widget_TextBox, NORMAL_TEXT | NORMAL_BOX, WidgetContent_Heading);
	pages[0].widget[widget].setPosition(x, y, TEXTBOX_S_WIDTH, TEXTBOX_S_HEIGHT);
	widget++;
	x += TEXTBOX_S_WIDTH;

	pages[0].widget[widget].setStyle(Widget_TextBox, NORMAL_TEXT | NORMAL_BOX | WS_BORDER_RIGHT, WidgetContent_Vario_Lazy);
	pages[0].widget[widget].setPosition(x, y, TEXTBOX_S_WIDTH, TEXTBOX_S_HEIGHT);
	widget++;
	x = 0;
	y += TEXTBOX_S_HEIGHT;
	
	pages[0].widget[widget].setStyle(Widget_TextBox, NORMAL_TEXT | NORMAL_BOX | WS_BORDER_BOTTOM, WidgetContent_Altitude_Baro);
	pages[0].widget[widget].setPosition(x, y, TEXTBOX_S_WIDTH, TEXTBOX_S_HEIGHT);
	widget++;
	x += TEXTBOX_S_WIDTH;

	pages[0].widget[widget].setStyle(Widget_TextBox, NORMAL_TEXT | NORMAL_BOX | WS_BORDER_RIGHT | WS_BORDER_BOTTOM, WidgetContent_Temperature);
	pages[0].widget[widget].setPosition(x, y, TEXTBOX_S_WIDTH, TEXTBOX_S_HEIGHT);
	widget++;
	x = 0;
	y += TEXTBOX_S_HEIGHT;

	pages[0].widget[widget].setStyle(Widget_TextBox, WS_FONT_BOLD_4 | WS_TA_LEFT | WS_TA_BOTTOM | NORMAL_BOX | WS_BORDER_RIGHT | WS_BORDER_BOTTOM, WidgetContent_Pressure);
	pages[0].widget[widget].setPosition(x, y, 176, 264 - y);
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
		case CMD_SHOW_PREV_PAGE :
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
			case 0x5001 : // show preference
				break;
				
			case 0x5002 : // toggle sound
				context.volume.vario = context.volume.vario ? 0 : 1;
				if (! context.volume.vario)
				{
					varioBeeper.setVelocity(0);			
					toneGen.setFrequency(0);
				}
				break;
				
			case 0x5003 : // toggle bluetooth
				context.device.statusBT = context.device.statusBT ? 0 : 1;
				if (context.device.statusBT)
				{
					serialBluetooth.register_callback(bluetoothSPPCallback);
					serialBluetooth.begin("MiniVario");
				}					
				else
				{
					serialBluetooth.register_callback(NULL);
					serialBluetooth.end();
				}
				break;
				
			case 0x5004 : // turn-off device
				// confirm ?
				// ...
				
				// stop logging
				// close alarm
				
				// and then go to deep sleep
				display.deepSleep();
				while(1);
				
			case 0x5005 : // restart(reset) device
				#if 0
				ESP.restart();
				#else
				HardReset();
				#endif
				break;
			}
			break;
			
		case CMD_SHUTDOWN :
			// stop logging
			// close alarm
			
			// and then go to deep sleep
			goDeepSleep();
			while(1);
		}
	}
}
