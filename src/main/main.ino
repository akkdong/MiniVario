// main.ino
//

#include "KalmanVario.h"
#include "VarioDisplay.h"
#include "BatteryVoltage.h"
#include "Keyboard.h"
#include "DeviceDefines.h"


////////////////////////////////////////////////////////////////////////////////////////
//


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


CriticalSection cs;
Sensor_MS5611  baro(cs, Wire);
KalmanVario vario(baro);

DeviceContext context;

VarioDisplayDriver driver(ePaperPins);
VarioDisplay display(driver, context);

BatteryVoltage battery;
Keyboard keybd(keybdPins, sizeof(keybdPins) / sizeof(keybdPins[0]));

VarioScreen pages[3];
PopupMenu	topMenu;

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

////////////////////////////////////////////////////////////////////////////////////////
//

void setup()
{
	//
	loadPages(pages);
	
	topMenu.addItem(0x5001, 0 /* IDS_BASIC_SETTINGS */);
	topMenu.addItem(0x5002, 0 /* IDS_SOUND_ONOFF */);
	topMenu.addItem(0x5003, 0 /* IDS_BLUETOOTH_ONOFF */);
	topMenu.addItem(0x5004, 0 /* IDS_POWER_OFF */);
	
	//
	initPins(powerPins, sizeof(powerPins) / sizeof(powerPins[0]));
	delay(100);
	
	//
	Serial.begin(115200);
	Serial.println("Start MiniVario...\n");
	
	//
	Wire.begin();

	//
	vario.begin();
	display.begin();
	
	display.attachScreen(&pages[0]);
	
	battery.begin();
	context.device.batteryPower = battery.getVoltage();
	
	keybd.begin();
}


void loop()
{
	if (vario.available())
	{		
		context.varioState.speedVertActive = vario.getVelocity();
		context.varioState.speedVertLazy = context.varioState.speedVertLazy * 0.8 + context.varioState.speedVertActive * 0.2;
		context.varioState.altitudeBaro = vario.getAltitude();
		context.varioState.pressure = vario.getPressure();
		context.varioState.temperature = vario.getTemperature();
		
		vario.flush();
	}
	
	//
	if (battery.update())
		context.device.batteryPower = battery.getVoltage();
	
	//
	keybd.update();
	
	//
	int key = keybd.getch();
	if (key >= 0)
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
			case CMD_SHOW_NEXT_PAGE :
			case CMD_SHOW_PREV_PAGE :
				break;
			case CMD_SHOW_PREFERENCE :
				display.showPopup(NULL);
				//display.activatePreference(xxx);
				break;
			case CMD_SHOW_TOP_MENU :
				// enter menu
				display.showPopup(&topMenu);
				break;
				
			case CMD_TOGGLE_SOUND :
				context.device.statusSound = context.device.statusSound ? 0 : 1;
				display.showPopup(NULL);
				break;
			case CMD_TOGGLE_BLUETOOTH :
				context.device.statusBT = context.device.statusBT ? 0 : 1;
				display.showPopup(NULL);
				break;
				
			case CMD_HIDE_POPUP :
				// releave menu
				display.showPopup(NULL);
				break;
				
			case CMD_SHUTDOWN :
				// stop logging
				// close alarm
				
				// and then go to deep sleep
				display.deepSleep();
				while(1);
			}
		}
	}
}


