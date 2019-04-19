// Pressure.ino
//

#include "KalmanVario.h"
#include "BluetoothSerialEx.h"

CriticalSection mCriticalSection;
Sensor_MS5611  Baro(mCriticalSection, Wire);
KalmanVario Vario(Baro);
uint32_t _tick;

BluetoothSerialEx bt;

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
	Serial.println("Pressure Variometer Test...");

	bt.begin("MiniVario");
	
	Wire.begin(-1, -1, 400000);
	
	Vario.begin();
	
	//
	_tick = millis();
}

void loop()
{
	#if 0
	if (millis() - _tick > 1000)
	{
		Serial.print("Pressure = "); Serial.print(Vario.getPressure()); Serial.print(" hPa, Temperature  ="); Serial.print(Vario.getTemperature()); Serial.println(" C");
		Serial.print("Vario = "); Serial.print(Vario.getVelocity() * 100); Serial.println("cm/s");
		Serial.println("");		
		
		_tick = millis();
	}
	#else
	float p = 1013.0;
	float t = 25.0;
	float v = 0.0;
	if (Vario.available())
	{
		#if 1
		Serial.print(Vario.getVelocity() * 100); Serial.print(", "); Serial.print(Vario.getPressure()); Serial.print(", "); Serial.println(Vario.getTemperature());
		#elif 0
		p = p * 0.8 + Vario.getPressure() * 0.2; 
		Serial.println(p);
		#elif 0
		t = t * 0.8 + Vario.getTemperature() * 0.2; 
		Serial.println(t);
		#else
		v = v * 0.8 + Vario.getVelocity() * 100.0 * 0.2; 
		Serial.println(v);;
		#endif

		Vario.flush();
	}
	#endif
}
