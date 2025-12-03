// Sensor_MS5611.cpp
//

#include "Sensor_MS5611.h"

#define CONVERT_TIME				(9)		// 9 ms: the measure period need to be greater than 8.22 ms


///////////////////////////////////////////////////////////////////////////////////////////////
// class Sensor_MS5611

Sensor_MS5611::Sensor_MS5611(CriticalSection & _cs, TwoWire & _i2c) 
	: mCriticalSection(_cs)
	, mWire(_i2c)
	, baroState(MS5611_STEP_READY)
	, baroUpdated(0)
	, compensatedTemperature(0)
	, compensatedPressure(0)
{
}

void Sensor_MS5611::begin()
{
	//
	resetBaro();
	delay(MS5611_RESET_DELAY * 10);
	
	//
	c1 = getPROMValue(0);
	c2 = getPROMValue(1);
	c3 = getPROMValue(2);
	c4 = getPROMValue(3);
	c5 = getPROMValue(4);
	c6 = getPROMValue(5);
	
	#if 0
	Serial.println("MS5611 PROM...");
	Serial.print("c1 = "); Serial.println(c1);
	Serial.print("c2 = "); Serial.println(c2);
	Serial.print("c3 = "); Serial.println(c3);
	Serial.print("c4 = "); Serial.println(c4);
	Serial.print("c5 = "); Serial.println(c5);
	Serial.print("c6 = "); Serial.println(c6);
	#endif
	
	// set sampling rate
	// ???

	//
	baroState = MS5611_STEP_READY;
	baroUpdated = false;
}

void Sensor_MS5611::end()
{
	//
	baroState = MS5611_STEP_READY;
	baroUpdated = false;
}

int Sensor_MS5611::read(float * p, float * t)
{
	if (baroUpdated)
	{
		//
		if (p)
			*p = compensatedPressure;
		
		if (t)
			*t = compensatedTemperature;
		
		//
		baroUpdated = false;
		
		return 1;
	}
	
	return 0;
}

float Sensor_MS5611::getAltitude(float p, float seaLevel)
{
	float alti;
	
	alti = pow((p / seaLevel), 0.1902949572); // 0.1902949572 = 1 / 5.255
	alti = (1 - alti) * (288.15 / 0.0065);
	
	return alti;
}

void Sensor_MS5611::resetBaro()
{
	mWire.beginTransmission(MS5611_ADDRESS);
	mWire.write(MS5611_CMD_RESET);
	mWire.endTransmission();
}

uint16_t Sensor_MS5611::getPROMValue(int address, uint32_t timeout)
{
	uint8_t data[2];
	
	mWire.beginTransmission(MS5611_ADDRESS);
	mWire.write(MS5611_CMD_READ_PROM + (address * 2));
	mWire.endTransmission();
	
//	mWire.beginTransmission(MS5611_ADDRESS);
	mWire.requestFrom(MS5611_ADDRESS, sizeof(data));
	
	uint32_t tstart = millis();
	while (mWire.available() < sizeof(data) && (timeout == 0 || (millis() - tstart) < timeout)) {}
	
	for (int i = 0; i < sizeof(data); i++)
		data[i] = mWire.read();	

	return ((uint16_t)data[0] << 8) + (uint16_t)data[1];
}

uint32_t Sensor_MS5611::getDigitalValue(uint32_t timeout)
{
	uint8_t data[3];
	
	mWire.beginTransmission(MS5611_ADDRESS);
	mWire.write(MS5611_CMD_ADC_READ);
	mWire.endTransmission();
	
//	mWire.beginTransmission(MS5611_ADDRESS);
	mWire.requestFrom(MS5611_ADDRESS, sizeof(data));

	uint32_t tstart = millis();
	while (mWire.available() < sizeof(data) && (timeout == 0 || (millis() - tstart) < timeout)) {}
	
	for (int i = 0; i < sizeof(data); i++)
		data[i] = mWire.read();	
	
	uint32_t value = data[0];
	value <<= 8;
	value += data[1];
	value <<= 8;
	value += data[2];
	
	return value;
}

void Sensor_MS5611::convert()
{
	const TickType_t xDelay = CONVERT_TIME / portTICK_PERIOD_MS;

	//
	convertD1();
	vTaskDelay(xDelay);
	d1i = getDigitalValue();

	//
	convertD2();
	vTaskDelay(xDelay);
	d2i = getDigitalValue();

	//
	updateBaro();
}

void Sensor_MS5611::startConvert()
{
	convertD1();
}

void Sensor_MS5611::convertNext()
{
	if (baroState == MS5611_STEP_READY)
		return;
	
	if (baroState == MS5611_STEP_READ_PRESSURE)
	{
		d1i = getDigitalValue();
		
		convertD2();
	}
	else
	{
		d2i = getDigitalValue();
		
		convertD1();
	}

	if (baroState == MS5611_STEP_READ_PRESSURE)
		updateBaro();
}

void Sensor_MS5611::convertD1()
{
	mWire.beginTransmission(MS5611_ADDRESS);
	mWire.write(MS5611_CMD_CONV_D1);
	mWire.endTransmission();
	
	baroState = MS5611_STEP_READ_PRESSURE;
}

void Sensor_MS5611::convertD2()
{
	mWire.beginTransmission(MS5611_ADDRESS);
	mWire.write(MS5611_CMD_CONV_D2);
	mWire.endTransmission();

	baroState = MS5611_STEP_READ_TEMP;
}

void Sensor_MS5611::updateBaro()
{
	// compute temperature
	int32_t dt, temp;

	int32_t c5s = c5;
	c5s <<= 8;
	dt = d2i - c5s;

	int32_t c6s = c6;
	c6s *= dt;
	c6s >>= 23;

	temp = 2000 + c6s;

	// compute compensation
	int64_t off, sens;

	// offset
	int64_t c2d = c2;
	c2d <<=  16;

	int64_t c4d = c4;
	c4d *= dt;
	c4d >>= 7;

	off = c2d + c4d;

	// sens
	int64_t c1d = c1;
	c1d <<= 15;

	int64_t c3d = c3;
	c3d *= dt;
	c3d >>= 8;

	sens = c1d + c3d;

	// second order compensation
	int64_t t2, off2, sens2;

	if( temp < 2000 )
	{
		t2 = dt;
		t2 *= t2;
		t2 >>= 31;

		off2 = temp-2000;
		off2 *= off2;
		off2 *= 5;
		sens2 = off2;
		off2 >>= 1;
		sens2 >>= 2;

		if( temp < -1500 )
		{
			int64_t dtemp = temp + 1500;
			dtemp *= dtemp;
			off2 += 7 * dtemp;
			dtemp *= 11;
			dtemp >>= 1;
			sens2 += dtemp;
		}
		
		temp = temp - t2;
		off = off - off2;
		sens = sens - sens2;
	}

	// compute pressure
	int64_t p;

	p = d1i * sens;
	p >>= 21;
	p -= off;
	//p >>= 15 !!! done with doubles, see below

	// save result
	compensatedTemperature = temp / 100.0;
	compensatedPressure = (p / 32768.0) / 100.0;  //32768 = 2^15	

	//
	baroUpdated = true;
}
