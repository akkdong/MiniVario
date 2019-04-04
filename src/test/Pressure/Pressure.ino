// Pressure.ino
//

#include "KalmanVario.h"

CriticalSeciton mCriticalSection;
Sensor_MS5611  Baro(mCriticalSection, Wire);
KalmanVario Vario(Baro);
uint32_t _tick;

void setup()
{
	//
	digitalWrite(19, HIGH);
	pinMode(19, OUTPUT);
	delay(10);
	
	//
	Serial.begin(115200);
	Serial.println("Pressure Variometer Test...");
	
	Wire.begin();
	
	Vario.begin();
	
	//
	_tick = millis();
}

void loop()
{
	#if 0
	if (Vario.available())
	{
		Serial.print("Vario = "); Serial.println(Vario.getVelocity());
		Vario.flush();
	}
	
	if (millis() - _tick > 1000)
	{
		Serial.print("Pressure = "); Serial.print(Baro.getPressure()); Serial.print(", Temperature  ="); Serial.println(Baro.getTemperature());
		Serial.println("");		
		
		_tick = millis();
	}
	#else
	if (Vario.available())
	{
		//Serial.print(Vario.getVelocity()); Serial.print(","); Serial.print(Baro.getPressure()); Serial.print(","); Serial.println(Baro.getTemperature())
		Serial.println(Vario.getVelocity());;
	}
	#endif
}
