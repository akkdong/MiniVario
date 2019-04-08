// EPaper_Waveshare420.cpp
//

#include "EPaper_Waveshare420.h"

// COMMAND
#define PANEL_SETTING                               		0x00
#define POWER_SETTING                               	0x01
#define POWER_OFF                                   		0x02
#define POWER_OFF_SEQUENCE_SETTING           0x03
#define POWER_ON                                    		0x04
#define POWER_ON_MEASURE                           0x05
#define BOOSTER_SOFT_START                          	0x06
#define DEEP_SLEEP                                  			0x07
#define DATA_START_TRANSMISSION_1              0x10
#define DATA_STOP                                   		0x11
#define DISPLAY_REFRESH                             		0x12
#define DATA_START_TRANSMISSION_2              0x13
#define LUT_FOR_VCOM                                	0x20
#define LUT_WHITE_TO_WHITE                          	0x21
#define LUT_BLACK_TO_WHITE                          	0x22
#define LUT_WHITE_TO_BLACK                          	0x23
#define LUT_BLACK_TO_BLACK                          	0x24
#define PLL_CONTROL                                 		0x30
#define TEMPERATURE_SENSOR_COMMAND       0x40
#define TEMPERATURE_SENSOR_SELECTION        0x41
#define TEMPERATURE_SENSOR_WRITE               0x42
#define TEMPERATURE_SENSOR_READ                0x43
#define VCOM_AND_DATA_INTERVAL_SETTING	0x50
#define LOW_POWER_DETECTION                       0x51
#define TCON_SETTING                                		0x60
#define RESOLUTION_SETTING                          	0x61
#define GSST_SETTING                                		0x65
#define GET_STATUS                                  		0x71
#define AUTO_MEASUREMENT_VCOM                0x80
#define READ_VCOM_VALUE                             	0x81
#define VCM_DC_SETTING                              	0x82
#define PARTIAL_WINDOW                              	0x90
#define PARTIAL_IN                                  			0x91
#define PARTIAL_OUT                                 		0x92
#define PROGRAM_MODE                                	0xA0
#define ACTIVE_PROGRAMMING                       	0xA1
#define READ_OTP                                    			0xA2
#define POWER_SAVING                                	0xE3



////////////////////////////////////////////////////////////////////////////////////////////////////////
//

const uint8_t EPaper_Waveshare420::lut_20_vcom0_full[] PROGMEM =
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

const uint8_t EPaper_Waveshare420::lut_21_ww_full[] PROGMEM =
{
  0x40, 0x08, 0x00, 0x00, 0x00, 0x02,
  0x90, 0x28, 0x28, 0x00, 0x00, 0x01,
  0x40, 0x14, 0x00, 0x00, 0x00, 0x01,
  0xA0, 0x12, 0x12, 0x00, 0x00, 0x01,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

const uint8_t EPaper_Waveshare420::lut_22_bw_full[] PROGMEM =
{
  0x40, 0x08, 0x00, 0x00, 0x00, 0x02,
  0x90, 0x28, 0x28, 0x00, 0x00, 0x01,
  0x40, 0x14, 0x00, 0x00, 0x00, 0x01,
  0xA0, 0x12, 0x12, 0x00, 0x00, 0x01,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

const uint8_t EPaper_Waveshare420::lut_23_wb_full[] PROGMEM =
{
  0x80, 0x08, 0x00, 0x00, 0x00, 0x02,
  0x90, 0x28, 0x28, 0x00, 0x00, 0x01,
  0x80, 0x14, 0x00, 0x00, 0x00, 0x01,
  0x50, 0x12, 0x12, 0x00, 0x00, 0x01,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

const uint8_t EPaper_Waveshare420::lut_24_bb_full[] PROGMEM =
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

const uint8_t EPaper_Waveshare420::lut_20_vcom0_partial[] PROGMEM =
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

const uint8_t EPaper_Waveshare420::lut_21_ww_partial[] PROGMEM =
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

const uint8_t EPaper_Waveshare420::lut_22_bw_partial[] PROGMEM =
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

const uint8_t EPaper_Waveshare420::lut_23_wb_partial[] PROGMEM =
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

const uint8_t EPaper_Waveshare420::lut_24_bb_partial[] PROGMEM =
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


////////////////////////////////////////////////////////////////////////////////////////////////////////
//

EPaper_Waveshare420::EPaper_Waveshare420(PinSetting * pins)
	: EPaperDriver(pins)
{
	
}

void EPaper_Waveshare420::init()
{
	EPaperDriver::init();
	
	_reset();
	
	_initDisplay();
}

void EPaper_Waveshare420::refresh(bool fast_mode)
{
	if (! (_state & _FAST_MODE))
	{
		if (fast_mode)
			_setLUTPartial();
	}
	else
	{
		if (! fast_mode)
			_setLUTFull();
	}
	//_powerOn();
	
	if (fast_mode)
	{
		_writeCommand(PARTIAL_IN); // 0x91	
		_setPartialWindow(0, 0, WAVESHARE_420_WIDTH, WAVESHARE_420_HEIGHT);
	}
	
	_transfer(DATA_START_TRANSMISSION_2, buffer);
	
	if (fast_mode)
	{
		_writeCommand(PARTIAL_OUT); // 0x92
	}
	_delay(1);
	
	_writeCommand(0x12);
	_waitWhileBusy(fast_mode ? partial_refresh_time : full_refresh_time);	
	//_powerOff();
	
	//_transfer(DATA_START_TRANSMISSION_1, buffer);	
}

void EPaper_Waveshare420::powerOff()
{
	_powerOff();
}

void EPaper_Waveshare420::deepSleep()
{
	//
	_powerOff();
	
	//
	_writeCommand(DEEP_SLEEP);
	_writeData(0xA5);
	
	_state = _DEEP_SLEEP;
}

void EPaper_Waveshare420::transfer()
{
	//_transfer(DATA_START_TRANSMISSION_1, buffer);
	//_transfer(DATA_START_TRANSMISSION_2, buffer);
	//_delay(1);
}


////////////////////////////////////////////////////////////////////////////////////////////////////////
//

void EPaper_Waveshare420::_initDisplay()
{
	_writeCommand(BOOSTER_SOFT_START); // 0x06
	_writeData(0x17);
	_writeData(0x17);
	_writeData(0x17);
	
	#if 1
	_writeCommand(POWER_SETTING); // 0x01
	_writeData(0x03);
	_writeData(0x00);
	_writeData(0x2B);
	_writeData(0x2B);
	_writeData(0x09);
	#endif
	
	_writeCommand(PANEL_SETTING); // 0x00
	_writeData(0x3F); // 300x400 B/W mode
	
	#if 1
	_writeCommand(PLL_CONTROL);
	_writeData(0x3A);
	
	_writeCommand(RESOLUTION_SETTING);
	_writeData(0x01);
	_writeData(0x90);
	_writeData(0x01);
	_writeData(0x2C);
	
	_writeCommand(VCM_DC_SETTING);
	_writeData(0x12);
	
	_writeCommand(VCOM_AND_DATA_INTERVAL_SETTING);
	_writeData(0x87);
	#endif
	
	_setLUTFull();
	_powerOn();	
}
	
void EPaper_Waveshare420::_powerOn()
{
	if (! (_state & _POWER_ON))
	{
		_writeCommand(POWER_ON); // 0x04
		_waitWhileBusy(power_on_time);
		
		_state |= _POWER_ON;
	}
}

void EPaper_Waveshare420::_powerOff()
{
	if (_state & _POWER_ON)
	{
		_writeCommand(POWER_OFF); // 0x02
		_waitWhileBusy(power_off_time);
		
		_state &= ~_POWER_ON;
	}
}

void EPaper_Waveshare420::_setLUTFull()
{
	_writeCommand(LUT_FOR_VCOM); // 0x20
	_writeDataP(lut_20_vcom0_full, sizeof(lut_20_vcom0_full));
		
	_writeCommand(LUT_WHITE_TO_WHITE); // 0x21
	_writeDataP(lut_21_ww_full, sizeof(lut_21_ww_full));
	
	_writeCommand(LUT_BLACK_TO_WHITE); // 0x22
	_writeDataP(lut_22_bw_full, sizeof(lut_22_bw_full));
	
	_writeCommand(LUT_WHITE_TO_BLACK); // 0x23
	_writeDataP(lut_23_wb_full, sizeof(lut_23_wb_full));
	
	_writeCommand(LUT_BLACK_TO_BLACK); // 0x24
	_writeDataP(lut_24_bb_full, sizeof(lut_24_bb_full));
	
	_state &= ~_FAST_MODE;
}

void EPaper_Waveshare420::_setLUTPartial()
{
	_writeCommand(LUT_FOR_VCOM); // 0x20
	_writeDataP(lut_20_vcom0_partial, sizeof(lut_20_vcom0_partial));
		
	_writeCommand(LUT_WHITE_TO_WHITE); // 0x21
	_writeDataP(lut_21_ww_partial, sizeof(lut_21_ww_partial));
	
	_writeCommand(LUT_BLACK_TO_WHITE); // 0x22
	_writeDataP(lut_22_bw_partial, sizeof(lut_22_bw_partial));
	
	_writeCommand(LUT_WHITE_TO_BLACK); // 0x23
	_writeDataP(lut_23_wb_partial, sizeof(lut_23_wb_partial));
	
	_writeCommand(LUT_BLACK_TO_BLACK); // 0x24
	_writeDataP(lut_24_bb_partial, sizeof(lut_24_bb_partial));
	
	_state |= _FAST_MODE;
}

void  EPaper_Waveshare420::_setPartialWindow(uint16_t x, uint16_t y, uint16_t w, uint16_t h)
{
	uint16_t xe = (x + w - 1) | 0x07;
	uint16_t ye = y + h - 1;
	
	x = x & ~(0x07);
	
	_writeCommand(PARTIAL_WINDOW); // 0x90
	_writeData(x / 256);
	_writeData(x % 256);
	_writeData(xe / 256);
	_writeData(xe % 256);
	_writeData(y / 256);
	_writeData(y % 256);
	_writeData(ye / 256);
	_writeData(ye % 256);
	_writeData(0x01); // don't see any difference
}

void  EPaper_Waveshare420::_updatePartialWindow(const uint8_t * buffer, uint16_t x, uint16_t y, uint16_t w, uint16_t h)
{
	_writeCommand(PARTIAL_IN); // 0x91	
	_setPartialWindow(x, y, w, h);
	_writeCommand(DATA_START_TRANSMISSION_2); // 0x13
	for (int i = 0; i < h; i++)
	{
		for (int j = 0; j < w / 8 ; j++)
		{
			_writeData(buffer[i * w / 8 + j]);
			//Serial.print(".");
		}
	}
	_writeCommand(PARTIAL_OUT); // 0x92
	_delay(1);
	
	
#if 0
	_writeCommand(0x12);
	_waitWhileBusy(partial_refresh_time);
	
	
	_writeCommand(PARTIAL_IN); // 0x91	
	//_setPartialWindow(x, y, w, h);
	_writeCommand(DATA_START_TRANSMISSION_2); // 0x13
	for (int i = 0; i < h; i++)
	{
		for (int j = 0; j < w / 8 ; j++)
		{
			_writeData(buffer[i * w / 8 + j]);
			//Serial.print(".");
		}
	}
	_writeCommand(PARTIAL_OUT); // 0x92
	_delay(1);
#endif
}

void  EPaper_Waveshare420::_updateFullWindow(const uint8_t * buffer)
{
	_writeCommand(DATA_START_TRANSMISSION_2); // 0x13
	for (int i = 0; i < WAVESHARE_420_WIDTH / 8 * WAVESHARE_420_HEIGHT; i++)
		_writeData(buffer[i]);
	_delay(1);
}

void EPaper_Waveshare420::_transfer(uint8_t command, const uint8_t * buffer)
{
	_writeCommand(command);
	
	for (int i = 0; i < WAVESHARE_420_WIDTH / 8 * WAVESHARE_420_HEIGHT; i++)
		_writeData(buffer[i]);
}
