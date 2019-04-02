// Display Library for SPI e-paper panels from Dalian Good Display and boards from Waveshare.
// Requires HW SPI and Adafruit_GFX. Caution: these e-papers require 3.3V supply AND data lines!
//
// based on Demo Example from Good Display: http://www.e-paper-display.com/download_list/downloadcategoryid=34&isMode=false.html
// Controller: IL0398 : http://www.e-paper-display.com/download_detail/downloadsId=537.html
//
// Author: Jean-Marc Zingg
//
// Version: see library.properties
//
// Library: https://github.com/ZinggJM/GxEPD2

#include "GxEPD2_420.h"

GxEPD2_420::GxEPD2_420(int8_t cs, int8_t dc, int8_t rst, int8_t busy) :
  GxEPD2_EPD(cs, dc, rst, busy, LOW, 10000000, WIDTH, HEIGHT, panel, hasColor, hasPartialUpdate, hasFastPartialUpdate)
{
}

void GxEPD2_420::clearScreen(uint8_t value)
{
  if (_initial)
  {
    _Init_Full();
    _writeCommand(0x13);
    for (uint32_t i = 0; i < uint32_t(WIDTH) * uint32_t(HEIGHT) / 8; i++)
    {
      _writeData(value);
    }
    _Update_Full();
    _initial = false;
  }
  if (!_using_partial_mode) _Init_Part();
  _writeCommand(0x91); // partial in
  _setPartialRamArea(0, 0, WIDTH, HEIGHT);
  _writeCommand(0x13);
  for (uint32_t i = 0; i < uint32_t(WIDTH) * uint32_t(HEIGHT) / 8; i++)
  {
    _writeData(value);
  }
  _Update_Part();
  _setPartialRamArea(0, 0, WIDTH, HEIGHT);
  _writeCommand(0x13);
  for (uint32_t i = 0; i < uint32_t(WIDTH) * uint32_t(HEIGHT) / 8; i++)
  {
    _writeData(value);
  }
  _Update_Part();
  _writeCommand(0x92); // partial out
  _initial = false;
}

void GxEPD2_420::writeScreenBuffer(uint8_t value)
{
  if (_initial) clearScreen(value);
  else _writeScreenBuffer(value);
}

void GxEPD2_420::_writeScreenBuffer(uint8_t value)
{
  if (!_using_partial_mode) _Init_Part();
  _writeCommand(0x91); // partial in
  _setPartialRamArea(0, 0, WIDTH, HEIGHT);
  _writeCommand(0x13);
  for (uint32_t i = 0; i < uint32_t(WIDTH) * uint32_t(HEIGHT) / 8; i++)
  {
    _writeData(value);
  }
  _writeCommand(0x92); // partial out
}

void GxEPD2_420::writeImage(const uint8_t bitmap[], int16_t x, int16_t y, int16_t w, int16_t h, bool invert, bool mirror_y, bool pgm)
{
  delay(1); // yield() to avoid WDT on ESP8266 and ESP32
  int16_t wb = (w + 7) / 8; // width bytes, bitmaps are padded
  x -= x % 8; // byte boundary
  w = wb * 8; // byte boundary
  int16_t x1 = x < 0 ? 0 : x; // limit
  int16_t y1 = y < 0 ? 0 : y; // limit
  int16_t w1 = x + w < int16_t(WIDTH) ? w : int16_t(WIDTH) - x; // limit
  int16_t h1 = y + h < int16_t(HEIGHT) ? h : int16_t(HEIGHT) - y; // limit
  int16_t dx = x1 - x;
  int16_t dy = y1 - y;
  w1 -= dx;
  h1 -= dy;
  if ((w1 <= 0) || (h1 <= 0)) return;
  if (!_using_partial_mode) _Init_Part();
  _writeCommand(0x91); // partial in
  _setPartialRamArea(x1, y1, w1, h1);
  _writeCommand(0x13);
  for (int16_t i = 0; i < h1; i++)
  {
    for (int16_t j = 0; j < w1 / 8; j++)
    {
      uint8_t data;
      // use wb, h of bitmap for index!
      int16_t idx = mirror_y ? j + dx / 8 + ((h - 1 - (i + dy))) * wb : j + dx / 8 + (i + dy) * wb;
      if (pgm)
      {
#if defined(__AVR) || defined(ESP8266) || defined(ESP32)
        data = pgm_read_byte(&bitmap[idx]);
#else
        data = bitmap[idx];
#endif
      }
      else
      {
        data = bitmap[idx];
      }
      if (invert) data = ~data;
      _writeData(data);
    }
  }
  _writeCommand(0x92); // partial out
  delay(1); // yield() to avoid WDT on ESP8266 and ESP32
}

void GxEPD2_420::writeImage(const uint8_t* black, const uint8_t* color, int16_t x, int16_t y, int16_t w, int16_t h, bool invert, bool mirror_y, bool pgm)
{
  if (black)
  {
    writeImage(black, x, y, w, h, invert, mirror_y, pgm);
  }
}

void GxEPD2_420::writeNative(const uint8_t* data1, const uint8_t* data2, int16_t x, int16_t y, int16_t w, int16_t h, bool invert, bool mirror_y, bool pgm)
{
  if (data1)
  {
    writeImage(data1, x, y, w, h, invert, mirror_y, pgm);
  }
}

void GxEPD2_420::drawImage(const uint8_t bitmap[], int16_t x, int16_t y, int16_t w, int16_t h, bool invert, bool mirror_y, bool pgm)
{
  writeImage(bitmap, x, y, w, h, invert, mirror_y, pgm);
  refresh(x, y, w, h);
}

void GxEPD2_420::drawImage(const uint8_t* black, const uint8_t* color, int16_t x, int16_t y, int16_t w, int16_t h, bool invert, bool mirror_y, bool pgm)
{
  writeImage(black, color, x, y, w, h, invert, mirror_y, pgm);
  refresh(x, y, w, h);
}

void GxEPD2_420::drawNative(const uint8_t* data1, const uint8_t* data2, int16_t x, int16_t y, int16_t w, int16_t h, bool invert, bool mirror_y, bool pgm)
{
  writeNative(data1, data2, x, y, w, h, invert, mirror_y, pgm);
  refresh(x, y, w, h);
}

void GxEPD2_420::refresh(bool partial_update_mode)
{
  if (partial_update_mode) refresh(0, 0, WIDTH, HEIGHT);
  else
  {
    if (_using_partial_mode) _Init_Full();
    _Update_Full();
  }
}

void GxEPD2_420::refresh(int16_t x, int16_t y, int16_t w, int16_t h)
{
  x -= x % 8; // byte boundary
  w -= x % 8; // byte boundary
  int16_t x1 = x < 0 ? 0 : x; // limit
  int16_t y1 = y < 0 ? 0 : y; // limit
  int16_t w1 = x + w < int16_t(WIDTH) ? w : int16_t(WIDTH) - x; // limit
  int16_t h1 = y + h < int16_t(HEIGHT) ? h : int16_t(HEIGHT) - y; // limit
  w1 -= x1 - x;
  h1 -= y1 - y;
  if (!_using_partial_mode) _Init_Part();
  _writeCommand(0x91); // partial in
  _setPartialRamArea(x1, y1, w1, h1);
  _Update_Part();
  _writeCommand(0x92); // partial out
}

void GxEPD2_420::powerOff(void)
{
  _PowerOff();
}

void GxEPD2_420::hibernate()
{
  _PowerOff();
  if (_rst >= 0)
  {
    _writeCommand(0x07); // deep sleep
    _writeData(0xA5);    // check code
    _hibernating = true;
  }
}

void GxEPD2_420::_setPartialRamArea(uint16_t x, uint16_t y, uint16_t w, uint16_t h)
{
  uint16_t xe = (x + w - 1) | 0x0007; // byte boundary inclusive (last byte)
  uint16_t ye = y + h - 1;
  x &= 0xFFF8; // byte boundary
  _writeCommand(0x90); // partial window
  _writeData(x / 256);
  _writeData(x % 256);
  _writeData(xe / 256);
  _writeData(xe % 256);
  _writeData(y / 256);
  _writeData(y % 256);
  _writeData(ye / 256);
  _writeData(ye % 256);
  _writeData(0x01); // don't see any difference
  //_writeData(0x00); // don't see any difference
}

void GxEPD2_420::_PowerOn()
{
  if (!_power_is_on)
  {
    _writeCommand(0x04);
    _waitWhileBusy("_PowerOn", power_on_time);
  }
  _power_is_on = true;
}

void GxEPD2_420::_PowerOff()
{
  _writeCommand(0x02); // power off
  _waitWhileBusy("_PowerOff", power_off_time);
  _power_is_on = false;
  _using_partial_mode = false;
}

void GxEPD2_420::_InitDisplay()
{
  if (_hibernating) _reset();
  _writeCommand(0x06); // boost
  _writeData(0x17);
  _writeData(0x17);
  _writeData(0x17);
  _writeCommand(0x00);
  //_writeData(0x1f); // LUT from OTP Pixel with B/W.
  _writeData(0x3F); //300x400 B/W mode, LUT set by register
}

const unsigned char GxEPD2_420::lut_20_vcom0_full[] PROGMEM =
{
  0x00, 0x08, 0x00, 0x00, 0x00, 0x02,
  0x60, 0x28, 0x28, 0x00, 0x00, 0x01,
  0x00, 0x14, 0x00, 0x00, 0x00, 0x01,
  0x00, 0x12, 0x12, 0x00, 0x00, 0x01,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00
};

const unsigned char GxEPD2_420::lut_21_ww_full[] PROGMEM =
{
  0x40, 0x08, 0x00, 0x00, 0x00, 0x02,
  0x90, 0x28, 0x28, 0x00, 0x00, 0x01,
  0x40, 0x14, 0x00, 0x00, 0x00, 0x01,
  0xA0, 0x12, 0x12, 0x00, 0x00, 0x01,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

const unsigned char GxEPD2_420::lut_22_bw_full[] PROGMEM =
{
  0x40, 0x08, 0x00, 0x00, 0x00, 0x02,
  0x90, 0x28, 0x28, 0x00, 0x00, 0x01,
  0x40, 0x14, 0x00, 0x00, 0x00, 0x01,
  0xA0, 0x12, 0x12, 0x00, 0x00, 0x01,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

const unsigned char GxEPD2_420::lut_23_wb_full[] PROGMEM =
{
  0x80, 0x08, 0x00, 0x00, 0x00, 0x02,
  0x90, 0x28, 0x28, 0x00, 0x00, 0x01,
  0x80, 0x14, 0x00, 0x00, 0x00, 0x01,
  0x50, 0x12, 0x12, 0x00, 0x00, 0x01,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

const unsigned char GxEPD2_420::lut_24_bb_full[] PROGMEM =
{
  0x80, 0x08, 0x00, 0x00, 0x00, 0x02,
  0x90, 0x28, 0x28, 0x00, 0x00, 0x01,
  0x80, 0x14, 0x00, 0x00, 0x00, 0x01,
  0x50, 0x12, 0x12, 0x00, 0x00, 0x01,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

// original wavetable from GxEPD, optimized for the display I have (modified Ben Krasnow version)
#define TP0A  2 // sustain phase for bb and ww, change phase for bw and wb
#define TP0B 45 // change phase for bw and wb

// same waveform as demo wavetable from Good Display:
//#define TP0A  0   // sustain phase for bb and ww, change phase for bw and wb
//#define TP0B 0x19 // change phase for bw and wb

const unsigned char GxEPD2_420::lut_20_vcom0_partial[] PROGMEM =
{
  0x00,
  TP0A, TP0B, 0x01, 0x00, 0x01,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

const unsigned char GxEPD2_420::lut_21_ww_partial[] PROGMEM =
{
  0x80, // 10 00 00 00
  TP0A, TP0B, 0x01, 0x00, 0x01,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

const unsigned char GxEPD2_420::lut_22_bw_partial[] PROGMEM =
{
  0xA0, // 10 10 00 00
  TP0A, TP0B, 0x01, 0x00, 0x01,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

const unsigned char GxEPD2_420::lut_23_wb_partial[] PROGMEM =
{
  0x50, // 01 01 00 00
  TP0A, TP0B, 0x01, 0x00, 0x01,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

const unsigned char GxEPD2_420::lut_24_bb_partial[] PROGMEM =
{
  0x40, // 01 00 00 00
  TP0A, TP0B, 0x01, 0x00, 0x01,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

void GxEPD2_420::_Init_Full()
{
  _InitDisplay();
  _writeCommand(0x20);
  _writeDataPGM(lut_20_vcom0_full, sizeof(lut_20_vcom0_full));
  _writeCommand(0x21);
  _writeDataPGM(lut_21_ww_full, sizeof(lut_21_ww_full));
  _writeCommand(0x22);
  _writeDataPGM(lut_22_bw_full, sizeof(lut_22_bw_full));
  _writeCommand(0x23);
  _writeDataPGM(lut_23_wb_full, sizeof(lut_23_wb_full));
  _writeCommand(0x24);
  _writeDataPGM(lut_24_bb_full, sizeof(lut_24_bb_full));
  _PowerOn();
  _using_partial_mode = false;
}

void GxEPD2_420::_Init_Part()
{
  _InitDisplay();
  _writeCommand(0x20);
  _writeDataPGM(lut_20_vcom0_partial, sizeof(lut_20_vcom0_partial));
  _writeCommand(0x21);
  _writeDataPGM(lut_21_ww_partial, sizeof(lut_21_ww_partial));
  _writeCommand(0x22);
  _writeDataPGM(lut_22_bw_partial, sizeof(lut_22_bw_partial));
  _writeCommand(0x23);
  _writeDataPGM(lut_23_wb_partial, sizeof(lut_23_wb_partial));
  _writeCommand(0x24);
  _writeDataPGM(lut_24_bb_partial, sizeof(lut_24_bb_partial));
  _PowerOn();
  _using_partial_mode = true;
}

void GxEPD2_420::_Update_Full()
{
  _writeCommand(0x12); //display refresh
  _waitWhileBusy("_Update_Full", full_refresh_time);
}

void GxEPD2_420::_Update_Part()
{
  _writeCommand(0x12); //display refresh
  _waitWhileBusy("_Update_Part", partial_refresh_time);
}
