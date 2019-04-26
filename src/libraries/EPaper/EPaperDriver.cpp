// EPaperDriver.cpp
//

#include "EPaperDriver.h"


////////////////////////////////////////////////////////////////////////////////////////////////////////
//

EPaperDriver::EPaperDriver(PinSetting * pins)
	: _spi_settings(4000000, MSBFIRST, SPI_MODE0)
	, _pin_settings(pins)
	, _state(_UNDEFINED)
{
}

void EPaperDriver::init()
{
	//
	initPins(_pin_settings, _PIN_Count);
	
	//
	SPIv.begin();
	
	_state |= _INITIALIZED;
}

void EPaperDriver::_reset()
{
	//
	setPinState(&_pin_settings[_PIN_RST], PIN_STATE_ACTIVE);
	_delay(10);
	setPinState(&_pin_settings[_PIN_RST], PIN_STATE_INACTIVE);
	_delay(10);
	
	//
	_state &= ~(_DEEP_SLEEP | _POWER_ON | _FAST_MODE);
}

void EPaperDriver::_writeCommand(uint8_t command)
{
	SPIv.beginTransaction(_spi_settings);
	setPinState(&_pin_settings[_PIN_DC], PIN_STATE_ACTIVE);
	setPinState(&_pin_settings[_PIN_CS], PIN_STATE_ACTIVE);
	SPIv.transfer(command);
	setPinState(&_pin_settings[_PIN_CS], PIN_STATE_INACTIVE);
	setPinState(&_pin_settings[_PIN_DC], PIN_STATE_INACTIVE);
	SPIv.endTransaction();
}

void EPaperDriver::_writeData(uint8_t data)
{
	SPIv.beginTransaction(_spi_settings);
	setPinState(&_pin_settings[_PIN_CS], PIN_STATE_ACTIVE);
	SPIv.transfer(data);
	setPinState(&_pin_settings[_PIN_CS], PIN_STATE_INACTIVE);
	SPIv.endTransaction();
}

void EPaperDriver::_writeData(const uint8_t * data, uint16_t n)
{
	SPIv.beginTransaction(_spi_settings);
	setPinState(&_pin_settings[_PIN_CS], PIN_STATE_ACTIVE);
	while (n--)
		SPIv.transfer(*data++);
	setPinState(&_pin_settings[_PIN_CS], PIN_STATE_INACTIVE);
	SPIv.endTransaction();
}

void EPaperDriver::_writeDataP(const uint8_t * data, uint16_t n)
{
	SPIv.beginTransaction(_spi_settings);
	setPinState(&_pin_settings[_PIN_CS], PIN_STATE_ACTIVE);
	while (n--)
		SPIv.transfer(pgm_read_byte(&*data++));
	setPinState(&_pin_settings[_PIN_CS], PIN_STATE_INACTIVE);
	SPIv.endTransaction();
}

void EPaperDriver::_waitWhileBusy(uint16_t timeout)
{
	unsigned long start = millis();
	
	while (1)
	{
		_delay(2);

		if (getPinState(&_pin_settings[_PIN_BUSY]) != PIN_STATE_ACTIVE)
		{
//			Serial.print("NOT BUSY!! : "); Serial.println(millis() - start);
			break;
		}
		
		if ((millis() - start) > timeout)
		{
//			Serial.println("TIMEOUT~~");
			break;
		}
	}
}

void EPaperDriver::_waitWhileBusy(uint16_t minWait, uint16_t maxWait)
{
	unsigned long start = millis();
	_delay(minWait);
	
	while (1)
	{
		if (getPinState(&_pin_settings[_PIN_BUSY]) != PIN_STATE_ACTIVE)
		{
//			Serial.print("NOT BUSY!! : "); Serial.println(millis() - start);
			break;
		}
		
		if ((millis() - start) > maxWait)
		{
//			Serial.println("TIMEOUT~~");
			break;
		}

		_delay(1);
	}
}