// main.ino
//

#include "KalmanVario.h"
#include "VarioScreen.h"
#include "BatteryVoltage.h"


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

PinSetting ioPins[] =
{
	// PWR_PERIPH
	{ 19, PIN_MODE_OUTPUT, PIN_ACTIVE_HIGH, PIN_STATE_ACTIVE },
	// PWR_SOUND
	{ 27, PIN_MODE_OUTPUT, PIN_ACTIVE_HIGH, PIN_STATE_ACTIVE },
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

VarioEPaper driver(ePaperPins);
VarioScreen display(driver, context);

BatteryVoltage battery;


////////////////////////////////////////////////////////////////////////////////////////
//

void setup()
{
	//
	initPins(ioPins, sizeof(ioPins) / sizeof(ioPins[0]));
	delay(100);
	
	//
	Serial.begin(115200);
	Serial.println("Start MiniVario...\n");
	
	//
	Wire.begin();

	vario.begin();
	display.begin();
	
	battery.begin();
	context.batteryPower = battery.getVoltage();
}


void loop()
{
	if (vario.available())
	{		
		context.vario.speedVertActive = vario.getVelocity();
		context.vario.speedVertLazy = context.vario.speedVertLazy * 0.8 + context.vario.speedVertActive * 0.2;
		context.vario.altitudeBaro = vario.getAltitude();
		context.vario.pressure = vario.getPressure();
		context.vario.temperature = vario.getTemperature();
		
		vario.flush();
//		Serial.print("vario = "); Serial.println(context.vario.speedVertActive);
	}
	
	if (battery.update())
		context.batteryPower = battery.getVoltage();
	
//	if (! digitalRead(0))
	if (getPinState(&ioPins[4]) == PIN_STATE_ACTIVE)
	{
		display.deepSleep();
		while(1);
	}
}


