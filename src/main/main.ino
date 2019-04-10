// main.ino
//

#include "KalmanVario.h"
#include "VarioScreen.h"


////////////////////////////////////////////////////////////////////////////////////////
//


////////////////////////////////////////////////////////////////////////////////////////
//

//
PinSetting ePaperPins[] = 
{
	// CS
	{ SS, PIN_MODE_OUTPUT, PIN_ACTIVE_LOW },
	// DC
	{ 33, PIN_MODE_OUTPUT, PIN_ACTIVE_LOW },
	// RST
	{ 32, PIN_MODE_OUTPUT, PIN_ACTIVE_LOW },
	// BUSY
	{ 39, PIN_MODE_INPUT, PIN_ACTIVE_LOW },
};

PinSetting ioPins[] =
{
	// PWR_PERIPH
	{ 19, PIN_MODE_OUTPUT, PIN_ACTIVE_LOW },
	// PWR_SOUND
	{ 27, PIN_MODE_OUTPUT, PIN_ACTIVE_LOW },
	// KEY_UP
	{ 35, PIN_MODE_INPUT, PIN_ACTIVE_HIGH },
	// KEY_DOWN
	{ 34, PIN_MODE_INPUT, PIN_ACTIVE_HIGH },
	// KEY_SEL
	{  0, PIN_MODE_INPUT, PIN_ACTIVE_LOW },
};

CriticalSection cs;
Sensor_MS5611  baro(cs, Wire);
KalmanVario vario(baro);

DeviceContext context;

VarioEPaper driver(ePaperPins);
VarioScreen display(driver, context);


void initGpios(PinSetting * settings, int count);



////////////////////////////////////////////////////////////////////////////////////////
//

void setup()
{
	//
	initGpios(ioPins, sizeof(ioPins) / sizeof(ioPins[0]));
	delay(100);
	
	//
	Serial.begin(115200);
	Serial.println("Start MiniVario...\n");
	
	//
	Wire.begin();

	vario.begin();
	display.begin();	
}


void loop()
{
	if (vario.available())
	{
		context.vario.speedVertActive = vario.getVelocity();
		context.vario.speedVertLazy = context.vario.speedVertLazy * 0.8 + context.vario.speedVertActive * 0.2;
		context.vario.pressure = vario.getPressure();
		context.vario.temperature = vario.getTemperature();
		
		vario.flush();
//		Serial.print("vario = "); Serial.println(context.vario.speedVertActive);
	}
}

void initGpios(PinSetting * settings, int count)
{
	for (int i = 0; i < count ; i++)
	{
		if (settings[i].mode == PIN_MODE_OUTPUT)
		{
			digitalWrite(settings[i].no, settings[i].active == PIN_ACTIVE_LOW ? HIGH : LOW);
			pinMode(settings[i].no, OUTPUT);
		}
		else
		{
			pinMode(settings[i].no, INPUT);
		}
	}
}

