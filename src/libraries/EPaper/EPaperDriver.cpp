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
	SPIv.begin();
	
	_state |= _INITIALIZED;
}

void EPaperDriver::_reset()
{
	// 
	digitalWrite(_pin_settings[_PIN_RST].no, _pin_settings[_PIN_RST].active == PIN_ACTIVE_LOW ? LOW : HIGH);
	_delay(10);
	digitalWrite(_pin_settings[_PIN_RST].no, _pin_settings[_PIN_RST].active == PIN_ACTIVE_LOW ? HIGH : LOW);
	_delay(10);
	
	//
	_state &= ~(_DEEP_SLEEP | _POWER_ON | _FAST_MODE);
}

void EPaperDriver::_writeCommand(uint8_t command)
{
	SPIv.beginTransaction(_spi_settings);
	digitalWrite(_pin_settings[_PIN_DC].no, LOW);
	digitalWrite(_pin_settings[_PIN_CS].no, LOW);
	SPIv.transfer(command);
	digitalWrite(_pin_settings[_PIN_CS].no, HIGH);
	digitalWrite(_pin_settings[_PIN_DC].no, HIGH);
	SPIv.endTransaction();
}

void EPaperDriver::_writeData(uint8_t data)
{
	SPIv.beginTransaction(_spi_settings);
	digitalWrite(_pin_settings[_PIN_CS].no, LOW);
	SPIv.transfer(data);
	digitalWrite(_pin_settings[_PIN_CS].no, HIGH);
	SPIv.endTransaction();
}

void EPaperDriver::_writeData(const uint8_t * data, uint16_t n)
{
	SPIv.beginTransaction(_spi_settings);
	digitalWrite(_pin_settings[_PIN_CS].no, LOW);
	while (n--)
		SPIv.transfer(*data++);
	digitalWrite(_pin_settings[_PIN_CS].no, HIGH);
	SPIv.endTransaction();
}

void EPaperDriver::_writeDataP(const uint8_t * data, uint16_t n)
{
	SPIv.beginTransaction(_spi_settings);
	digitalWrite(_pin_settings[_PIN_CS].no, LOW);
	while (n--)
		SPIv.transfer(pgm_read_byte(&*data++));
	digitalWrite(_pin_settings[_PIN_CS].no, HIGH);
	SPIv.endTransaction();
}

void EPaperDriver::_waitWhileBusy(uint16_t timeout)
{
	unsigned long start = millis();
	
	while (1)
	{
		_delay(2);
		
		if (digitalRead(_pin_settings[_PIN_BUSY].no) != _pin_settings[_PIN_BUSY].active)
		{
			//Serial.print("NOT BUSY!! : "); Serial.println(millis() - start);
			break;
		}
		
		if ((millis() - start) > timeout)
		{
			//Serial.println("TIMEOUT~~");
			break;
		}
	}
}
