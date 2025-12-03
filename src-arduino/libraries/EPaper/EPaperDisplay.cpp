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

void EPaperDisplay::refresh(bool fast_update)
{
	_epd.refresh(fast_update);
}

void EPaperDisplay::sleep()
{
	_epd.deepSleep();
}



	
void  EPaperDisplay::drawBitmapBM(const uint8_t *bitmap, uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color, int16_t mode)
{
  uint16_t inverse_color = (color != COLOR_WHITE) ? COLOR_WHITE : COLOR_BLACK;
  uint16_t fg_color = (mode & bm_invert) ? inverse_color : color;
  uint16_t bg_color = (mode & bm_invert) ? color : inverse_color;
  // taken from Adafruit_GFX.cpp, modified
  int16_t byteWidth = (w + 7) / 8; // Bitmap scanline pad = whole byte
  uint8_t byte = 0;
  if (mode & bm_transparent)
  {
    for (int16_t j = 0; j < h; j++)
    {
      for (int16_t i = 0; i < w; i++ )
      {
        if (i & 7) byte <<= 1;
        else
        {
#if defined(__AVR) || defined(ESP8266) || defined(ESP32)
          byte = pgm_read_byte(&bitmap[j * byteWidth + i / 8]);
#else
          byte = bitmap[j * byteWidth + i / 8];
#endif
        }
        // transparent mode
        if (bool(mode & bm_invert) != bool(byte & 0x80))
        //if (!(byte & 0x80))
        {
          uint16_t xd = x + i;
          uint16_t yd = y + j;
          if (mode & bm_flip_x) xd = x + w - i;
          if (mode & bm_flip_y) yd = y + h - j;
          drawPixel(xd, yd, color);
        }
      }
    }
  }
  else
  {
    for (int16_t j = 0; j < h; j++)
    {
      for (int16_t i = 0; i < w; i++ )
      {
        if (i & 7) byte <<= 1;
        else
        {
#if defined(__AVR) || defined(ESP8266) || defined(ESP32)
          byte = pgm_read_byte(&bitmap[j * byteWidth + i / 8]);
#else
          byte = bitmap[j * byteWidth + i / 8];
#endif
        }
        // keep using overwrite mode
        uint16_t pixelcolor = (byte & 0x80) ? fg_color  : bg_color;
        uint16_t xd = x + i;
        uint16_t yd = y + j;
        if (mode & bm_flip_x) xd = x + w - i;
        if (mode & bm_flip_y) yd = y + h - j;
        drawPixel(xd, yd, pixelcolor);
      }
    }
  }
}

