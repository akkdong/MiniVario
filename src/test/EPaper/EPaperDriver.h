// EPaperDriver.h
//

#ifndef __EPAPERDRIVER_H__
#define __EPAPERDRIVER_H__

#include <Arduino.h>
#include <SPI.h>


////////////////////////////////////////////////////////////////////////////////////////////////////////
//

#define PIN_MODE_INPUT				0
#define PIN_MODE_OUTPUT			1

#define PIN_ACTIVE_LOW				0
#define PIN_ACTIVE_HIGHT				1

#define PIN_STATE_LOW				0
#define PIN_STATE_HIGH				1


struct PinSetting
{
	int8_t		no;
	int8_t		mode;		// OUTPUT or INPUT
	int8_t		active;		// active low or high
};

enum PinType
{
	_PIN_CS = 0,
	_PIN_DC = 1,
	_PIN_RST = 2,
	_PIN_BUSY = 3,
	_PIN_Count
};


////////////////////////////////////////////////////////////////////////////////////////////////////////
//

class EPaperDriver
{
public:
	EPaperDriver(PinSetting * pins);

public:	
	//
	virtual void 		init();
	
	virtual int16_t		getWidth() = 0;
	virtual int16_t		getHeight() = 0;
	
	virtual uint8_t *	getBuffer() = 0;
	
	virtual void		transfer() = 0;
	virtual void		refresh(bool fast_mode) = 0;
	
	virtual void		powerOff() = 0; // turns off generation of panel driving voltages, avoids screen fading over time
	virtual void		deepSleep() = 0; // turns off power & sets controller to deep sleep, ONLY wakeable by RST
	
	
	static inline uint16_t __min(uint16_t a, uint16_t b) { return (a < b ? a : b); }
	static inline uint16_t __max(uint16_t a, uint16_t b) { return (a > b ? a : b); }
	
protected:
	void					_reset();
	
	void					_writeCommand(uint8_t command);
	void					_writeData(uint8_t data);
	void					_writeData(const uint8_t * data, uint16_t n);
	void					_writeDataP(const uint8_t * data, uint16_t n);
	
	void					_waitBusy(uint16_t timeout);
	
public:
	enum _State
	{
		_UNDEFINED	 = 0,
		
		_INITIALIZED = 0x01,
		_DEEP_SLEEP = 0x02,
		_POWER_ON = 0x04,
		_FULL_MODE = 0x08
	};

// protected properties
protected:
	SPISettings		_spi_settings;
	PinSetting *		_pin_settings;
	
	uint8_t				_state;
};

#endif // __EPAPERDRIVER_H__
