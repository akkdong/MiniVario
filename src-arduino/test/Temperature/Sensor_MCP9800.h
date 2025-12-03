// Sensor_MCP9800.h
//

#ifndef __SENSOR_MCP9800_H__
#define __SENSOR_MCP9800_H__

#include <Arduino.h>
#include <Wire.h>
#include "CriticalSection.h"


///////////////////////////////////////////////////////////////////////////////////////////////////////////////
//

#define MCP9800_ADDRESS			0x4D

// config
#define ENABLE_ONESHOT			0x80
#define DISABLE_ONESHOT			0x00		// power-up default

#define RESOLUTION_9BIT				0x00		// power-up default
#define RESOLUTION_10BIT			0x20
#define RESOLUTION_11BIT			0x40
#define RESOLUTION_12BIT			0x60

#define FAULT_QUEUE_1				0x00		// power-up default
#define FAULT_QUEUE_2				0x08
#define FAULT_QUEUE_4				0x10
#define FAULT_QUEUE_6				0x18

#define ALERT_POLARITY_HIGH		0x04
#define ALERT_POLARITY_LOW		0x00		// power-up default

#define ENABLE_SHUTDOWN			0x01
#define DISABLE_SHUTDOWN			0x00		// power-up default


///////////////////////////////////////////////////////////////////////////////////////////////////////////////
// class Sensor_MCP9800

class Sensor_MCP9800
{
public:
	Sensor_MCP9800(CriticalSection & cs, TwoWire & wire);
	
public:
	float							getTemperature();
	uint8_t						getConfig();
	float							getHysteresis();
	float							getLimit();
	
	void							setConfig(uint8_t config);
	void							setHysteresis(float celsius);
	void							setLimit(float celsius);
	
private:
	void							write(uint8_t reg, uint8_t * data, int len);
	void							read(uint8_t reg, uint8_t * data, int len, uint32_t timeout = 0);
	
private:
	CriticalSection &		mCriticalSection;
	TwoWire &				mWire;
};

#endif // __SENSOR_MCP9800_H__
