// Temperature.ino
//

#include "Sensor_MCP9800.h"

CriticalSection mCriticalSection;
Sensor_MCP9800 sensor(mCriticalSection, Wire);


void setup()
{
	//
	digitalWrite(19, HIGH);
	pinMode(19, OUTPUT);
	
	digitalWrite(27, HIGH);
	pinMode(19, OUTPUT);
	
	pinMode(0, INPUT);
	pinMode(34, INPUT);
	pinMode(35, INPUT);
	delay(10);
	
	//
	Serial.begin(115200);
	Serial.println("Temperature Test!");
	
	Wire.begin();
	delay(100);
	
	//
	Serial.print("Hysteresis = "); Serial.println(sensor.getHysteresis());
	Serial.print("LimitSet = "); Serial.println(sensor.getLimit());
	Serial.print("Config = "); Serial.println(sensor.getConfig());
	Serial.println("");
}

void loop()
{
	Serial.print("Temperature = "); Serial.println(sensor.getTemperature());
	delay(500);
}
