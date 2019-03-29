// EPaperDriver.cpp
//

#include "EPaperDriver.h"

#if defined(ESP8266) || defined(ESP32)
#include <pgmspace.h>
#else
#include <avr/pgmspace.h>
#endif

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
	for (int i = 0; i < _PIN_Count; i++)
	{
		if (_pin_settings[i].mode == PIN_MODE_OUTPUT)
		{
			digitalWrite(_pin_settings[i].no, _pin_settings[i].active == PIN_ACTIVE_LOW ? HIGH : LOW);
			pinMode(_pin_settings[i].no, OUTPUT);
		}
		else
		{
			pinMode(_pin_settings[i].no, INPUT);
		}
	}
	
	//
	SPI.begin();
	
	_state |= _INITIALIZED;
}

void EPaperDriver::_reset()
{
	// 
	digitalWrite(_pin_settings[_PIN_RST].no, _pin_settings[_PIN_RST].active == PIN_ACTIVE_LOW ? HIGH : LOW);
	delay(20);
	digitalWrite(_pin_settings[_PIN_RST].no, _pin_settings[_PIN_RST].active == PIN_ACTIVE_LOW ? LOW : HIGH);
	delay(20);
	digitalWrite(_pin_settings[_PIN_RST].no, _pin_settings[_PIN_RST].active == PIN_ACTIVE_LOW ? HIGH : LOW);
	
	//
	_state &= ~_DEEP_SLEEP;
}

void EPaperDriver::_writeCommand(uint8_t command)
{
	SPI.beginTransaction(_spi_settings);
	digitalWrite(_pin_settings[_PIN_DC].no, LOW);
	digitalWrite(_pin_settings[_PIN_CS].no, LOW);
	SPI.transfer(command);
	digitalWrite(_pin_settings[_PIN_CS].no, HIGH);
	digitalWrite(_pin_settings[_PIN_DC].no, HIGH);
	SPI.endTransaction();
}

void EPaperDriver::_writeData(uint8_t data)
{
	SPI.beginTransaction(_spi_settings);
	digitalWrite(_pin_settings[_PIN_CS].no, LOW);
	SPI.transfer(data);
	digitalWrite(_pin_settings[_PIN_CS].no, HIGH);
	SPI.endTransaction();
}

void EPaperDriver::_writeData(const uint8_t * data, uint16_t n)
{
	SPI.beginTransaction(_spi_settings);
	digitalWrite(_pin_settings[_PIN_CS].no, LOW);
	while (n--)
		SPI.transfer(*data++);
	digitalWrite(_pin_settings[_PIN_CS].no, HIGH);
	SPI.endTransaction();
}

void EPaperDriver::_writeDataP(const uint8_t * data, uint16_t n)
{
	SPI.beginTransaction(_spi_settings);
	digitalWrite(_pin_settings[_PIN_CS].no, LOW);
	while (n--)
		SPI.transfer(pgm_read_byte(&*data++));
	digitalWrite(_pin_settings[_PIN_CS].no, HIGH);
	SPI.endTransaction();
}

void EPaperDriver::_waitBusy(uint16_t timeout)
{
	unsigned long start = millis();
	
	while (1)
	{
		delay(1);
		
		if (digitalRead(_pin_settings[_PIN_BUSY].no) != _pin_settings[_PIN_BUSY].active)
			break;
		
		if ((millis() - start) > timeout)
			break;
	}
}
