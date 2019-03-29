// EPaperDisplay.cpp
//

#include "EPaperDisplay.h"


////////////////////////////////////////////////////////////////////////////////////////////////////////
//

EPaperDisplay::EPaperDisplay(EPaperDriver & driver)
		: Adafruit_GFX(driver.getWidth(), driver.getHeight())
		, _epd(driver)
		, _buffer(driver.getBuffer())
{
}

void EPaperDisplay::drawPixel(int16_t x, int16_t y, uint16_t color)
{
	if ((x < 0) || (x >= _width) || (y < 0) || (y >= _height)) 
		return;
	
//	if (_mirror)
//		x = _width - x - 1;
	
	// check rotation, move pixel around if necessary
	switch (getRotation())
	{
	case 1:
		_swap_(x, y);
		x = _width - x - 1;
		break;
	case 2:
		x = _width - x - 1;
		y = _height - y - 1;
		break;
	case 3:
		_swap_(x, y);
		y = _height - y - 1;
	break;
	}
	
	#if 0
	// transpose partial window to 0,0
	x -= _window_x;
	y -= _window_y;
	
	// clip to (partial) window
	if ((x < 0) || (x >= _window_w) || (y < 0) || (y >= _window_h)) 
		return;
	
	if (_reverse) 
		y = _height - y - 1;
	
	if ((y < 0) || (y >= _height)) 
		return;
	
	uint16_t i = x / 8 + y * (_window_w / 8);
	#endif
	uint16_t i = x / 8 + y * (_width / 8);
	
	if (color)
		_buffer[i] = (_buffer[i] | (1 << (7 - (x % 8))));
	else
		_buffer[i] = (_buffer[i] & (~(1 << (7 - (x % 8)))));	
		//_buffer[i] = (_buffer[i] & (0xFF ^ (1 << (7 - x % 8))));	
}

void EPaperDisplay::init()
{
	_epd.init();
	
	fillScreen(COLOR_WHITE);
}

void EPaperDisplay::setRotation(uint8_t r)
{
	Adafruit_GFX::setRotation(r);
}

void EPaperDisplay::invertDisplay(boolean i)
{
	Adafruit_GFX::invertDisplay(i);
}

void EPaperDisplay::fillScreen(uint16_t color)
{
	uint8_t data = (color == COLOR_BLACK) ? COLOR_BLACK : COLOR_WHITE;
	
	for (uint16_t i = 0; i < _width / 8 * _height; i++)
		_buffer[i] = data;
}

void EPaperDisplay::fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color)
{
	Adafruit_GFX::fillRect(x, y, w, h, color);
}

void EPaperDisplay::flush(bool fast_update)
{
	_epd.transfer();
	_epd.refresh(fast_update);
}

void EPaperDisplay::sleep()
{
	_epd.deepSleep();
}
