// Sensor_MCP9800.cpp
//

#include "Sensor_MCP9800.h"

#define REG_AMBIENT				(0x00)
#define REG_CONFIG					(0x01)
#define REG_HYSTERESIS			(0x02)
#define REG_LIMIT_SET				(0x03)



///////////////////////////////////////////////////////////////////////////////////////////////////////////////
// class Sensor_MCP9800

Sensor_MCP9800::Sensor_MCP9800(CriticalSection & cs, TwoWire & wire) : mCriticalSection(cs), mWire(wire)
{
}

	
float Sensor_MCP9800::getTemperature()
{
	uint8_t data[2];	
	read(REG_AMBIENT, &data[0], sizeof(data));
	
	uint16_t value = ((uint16_t)data[0] << 8) + (uint16_t)data[1];	
	return ((int16_t)value >> 4) / 16.0;
}

uint8_t Sensor_MCP9800::getConfig()
{
	uint8_t data;		
	read(REG_AMBIENT, &data, sizeof(data));	
	
	return data;
}

float Sensor_MCP9800::getHysteresis()
{
	uint8_t data[2];	
	read(REG_AMBIENT, &data[0], sizeof(data));
	
	uint16_t value = ((uint16_t)data[0] << 8) + (uint16_t)data[1];	
	return ((int16_t)value >> 7) / 2.0;
}

float Sensor_MCP9800::getLimit()
{
	uint8_t data[2];
	read(REG_AMBIENT, &data[0], sizeof(data));	
	
	uint16_t value = ((uint16_t)data[0] << 8) + (uint16_t)data[1];	
	return ((int16_t)value >> 7) / 2.0;
}

void Sensor_MCP9800::setConfig(uint8_t config)
{
	write(REG_CONFIG, &config, sizeof(config));
}

void Sensor_MCP9800::setHysteresis(float celsius)
{
	uint16_t value = (uint16_t)(((int16_t)(celsius * 2)) << 7);
	uint8_t data[2] = { (uint8_t)(value >> 8), (uint8_t)(value & 0x00FF) };
	
	write(REG_HYSTERESIS, data, 2);
}

void Sensor_MCP9800::setLimit(float celsius)
{
	uint16_t value = (uint16_t)(((int16_t)(celsius * 2)) << 7);
	uint8_t data[2] = { (uint8_t)(value >> 8), (uint8_t)(value & 0x00FF) };
	
	write(REG_LIMIT_SET, data, 2);
}

void	Sensor_MCP9800::write(uint8_t reg, uint8_t * data, int len)
{
	mWire.beginTransmission(MCP9800_ADDRESS);
	mWire.write(reg);
	for (int i = 0; i < len; i++)
		mWire.write(data[i]);
	mWire.endTransmission();
}

void	Sensor_MCP9800::read(uint8_t reg, uint8_t * data, int len, uint32_t timeout)
{
	mWire.beginTransmission(MCP9800_ADDRESS);
	mWire.write(reg);
	mWire.endTransmission();
	
	mWire.requestFrom(MCP9800_ADDRESS, len);

	uint32_t tstart = millis();
	while (! mWire.available() && (timeout == 0 || (millis() - tstart) < timeout)) {}
	
	for (int i = 0; i < sizeof(data); i++)
		data[i] = mWire.read();	
}
