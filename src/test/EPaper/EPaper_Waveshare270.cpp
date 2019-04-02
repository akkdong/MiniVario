// EPaper_Waveshare270.cpp
//

#include "EPaper_Waveshare270.h"

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
#define PARTIAL_DATA_START_TRANSMISSION_1	0x14
#define PARTIAL_DATA_START_TRANSMISSION_2	0x15
#define PARTIAL_DISPLAY_REFRESH						0x16
#define LUT_FOR_VCOM                                	0x20
#define LUT_WHITE_TO_WHITE                          	0x21
#define LUT_BLACK_TO_WHITE                          	0x22
#define LUT_WHITE_TO_BLACK                          	0x23
#define LUT_BLACK_TO_BLACK                          	0x24
#define PLL_CONTROL                                 		0x30
#define TEMPERATURE_SENSOR_COMMAND       0x40
#define TEMPERATURE_SENSOR_CALIBRATION     0x41
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
#define PROGRAM_MODE                                	0xA0
#define ACTIVE_PROGRAMMING                       	0xA1
#define READ_OTP                                    			0xA2



////////////////////////////////////////////////////////////////////////////////////////////////////////
//

//full screen update LUT
const uint8_t EPaper_Waveshare270::lut_20_vcom_dc_full[] =
{
  0x00, 0x00,
  0x00, 0x08, 0x00, 0x00, 0x00, 0x02,
  0x60, 0x28, 0x28, 0x00, 0x00, 0x01,
  0x00, 0x14, 0x00, 0x00, 0x00, 0x01,
  0x00, 0x12, 0x12, 0x00, 0x00, 0x01,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

const uint8_t EPaper_Waveshare270::lut_21_ww_full[] =
{
  0x40, 0x08, 0x00, 0x00, 0x00, 0x02,
  0x90, 0x28, 0x28, 0x00, 0x00, 0x01,
  0x40, 0x14, 0x00, 0x00, 0x00, 0x01,
  0xA0, 0x12, 0x12, 0x00, 0x00, 0x01,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

const uint8_t EPaper_Waveshare270::lut_22_bw_full[] =
{
  0x40, 0x08, 0x00, 0x00, 0x00, 0x02,
  0x90, 0x28, 0x28, 0x00, 0x00, 0x01,
  0x40, 0x14, 0x00, 0x00, 0x00, 0x01,
  0xA0, 0x12, 0x12, 0x00, 0x00, 0x01,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

const uint8_t EPaper_Waveshare270::lut_23_wb_full[] =
{
  0x80, 0x08, 0x00, 0x00, 0x00, 0x02,
  0x90, 0x28, 0x28, 0x00, 0x00, 0x01,
  0x80, 0x14, 0x00, 0x00, 0x00, 0x01,
  0x50, 0x12, 0x12, 0x00, 0x00, 0x01,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

const uint8_t EPaper_Waveshare270::lut_24_bb_full[] =
{
  0x80, 0x08, 0x00, 0x00, 0x00, 0x02,
  0x90, 0x28, 0x28, 0x00, 0x00, 0x01,
  0x80, 0x14, 0x00, 0x00, 0x00, 0x01,
  0x50, 0x12, 0x12, 0x00, 0x00, 0x01,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

//partial screen update LUT
const uint8_t EPaper_Waveshare270::lut_20_vcom_dc_partial[] =
{
  0x00, 0x00,
  0x00, 0x19, 0x01, 0x00, 0x00, 0x01,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

const uint8_t EPaper_Waveshare270::lut_21_ww_partial[] =
{
  0x00, 0x19, 0x01, 0x00, 0x00, 0x01,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

const uint8_t EPaper_Waveshare270::lut_22_bw_partial[] =
{
  0x80, 0x19, 0x01, 0x00, 0x00, 0x01,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

const uint8_t EPaper_Waveshare270::lut_23_wb_partial[] =
{
  0x40, 0x19, 0x01, 0x00, 0x00, 0x01,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

const uint8_t EPaper_Waveshare270::lut_24_bb_partial[] =
{
  0x00, 0x19, 0x01, 0x00, 0x00, 0x01,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

////////////////////////////////////////////////////////////////////////////////////////////////////////
//

EPaper_Waveshare270::EPaper_Waveshare270(PinSetting * pins)
	: EPaperDriver(pins), _initial(true)
{
	
}

void EPaper_Waveshare270::init()
{
	EPaperDriver::init();
	
	#if 0
	_reset();
	_initDisplay();	
	_setLUTFull();
	_powerOn();
	#endif
	
	#if 0
	_writeCommand(DATA_START_TRANSMISSION_1);
	delay(2);
	for (int i = 0; i < WAVESHARE_270_WIDTH / 8 * WAVESHARE_270_HEIGHT; i++)
		_writeData(0xFF);
	delay(2);
	
	_writeCommand(DATA_START_TRANSMISSION_2);
	delay(2);
	for (int i = 0; i < WAVESHARE_270_WIDTH / 8 * WAVESHARE_270_HEIGHT; i++)
		_writeData(0xFF);
	delay(2);	
	#endif
}

void EPaper_Waveshare270::refresh(bool fast_mode)
{	
	if (fast_mode)
	{
		if (_state & _FULL_MODE)
		{
			// wakeUp...
			_reset();
			_initDisplay();
			_setLUTFull();
		}
		_setLUTPartial();

		// writeWindow
		Serial.println("_setPartialWindow(0x15)");
		_setPartialWindow(0x15, 0, 0, WAVESHARE_270_WIDTH, WAVESHARE_270_HEIGHT);
		for (int i = 0; i < WAVESHARE_270_WIDTH / 8 * WAVESHARE_270_HEIGHT; i++)
			_writeData(buffer[i]);
		
		// refreshWindow
		Serial.println("_refreshWindow(0x16)");
		_writeCommand(0x16);
		_writeData(0);
		_writeData(0);
		_writeData(0);
		_writeData(0);
		_writeData(WAVESHARE_270_WIDTH >> 8);
		_writeData(WAVESHARE_270_WIDTH & 0xF8);
		_writeData(WAVESHARE_270_HEIGHT >> 8);
		_writeData(WAVESHARE_270_HEIGHT & 0xFF);	
		_waitWhileBusy(partial_refresh_time+1000);
	
		// writeWindow
		Serial.println("_setPartialWindow(0x14)");
		_setPartialWindow(0x14, 0, 0, WAVESHARE_270_WIDTH, WAVESHARE_270_HEIGHT);
		for (int i = 0; i < WAVESHARE_270_WIDTH / 8 * WAVESHARE_270_HEIGHT; i++)
			_writeData(buffer[i]);
	}
	else
	{
		_reset();
		_initDisplay();
		_setLUTFull();
		
		if (_initial)
		{
			Serial.println("_writeCommand(0x10)");
			_writeCommand(0x10);
			//delay(2);
			for (int i = 0; i < WAVESHARE_270_WIDTH / 8 * WAVESHARE_270_HEIGHT; i++)
				_writeData(0xFF);
			//delay(2);
			
			_initial = false;
		}
		
		Serial.println("_writeCommand(0x13)");
		_writeCommand(0x13);
		//delay(2);
		for (int i = 0; i < WAVESHARE_270_WIDTH / 8 * WAVESHARE_270_HEIGHT; i++)
			_writeData(buffer[i]);
		//delay(2);
		
		Serial.println("_writeCommand(0x12)");
		_writeCommand(0x12);
		_waitWhileBusy(full_refresh_time+1000);
		
		Serial.println("_writeCommand(0x10)");
		_writeCommand(0x10);
		//delay(2);
		for (int i = 0; i < WAVESHARE_270_WIDTH / 8 * WAVESHARE_270_HEIGHT; i++)
			_writeData(buffer[i]);
		//delay(2);
		
		Serial.println("_deepSleep()");
		deepSleep();
	}
}

void EPaper_Waveshare270::powerOn()
{
	if (! (_state & _POWER_ON))
	{
		Serial.println("powerOn()");
		
		_powerOn();	
		_state |= _POWER_ON;
	}
}

void EPaper_Waveshare270::powerOff()
{
	if (_state & _POWER_ON)
	{
		Serial.println("powerOff()");
		
		_powerOff();	
		_state &= ~_POWER_ON;
	}
}

void EPaper_Waveshare270::deepSleep()
{
	if (! (_state & _DEEP_SLEEP))
	{
		//
		powerOff();
	
		//
		_deepSleep();
	
		_state = _DEEP_SLEEP;
	}
}

void EPaper_Waveshare270::transfer()
{
	//_transfer(DATA_START_TRANSMISSION_1, buffer);
	//_transfer(DATA_START_TRANSMISSION_2, buffer);
	//delay(1);
}


////////////////////////////////////////////////////////////////////////////////////////////////////////
//

void EPaper_Waveshare270::_initDisplay()
{
	Serial.println("_initDisplay()");
	_writeCommand(POWER_SETTING); // 0x01
	_writeData(0x03);
	_writeData(0x00);
	_writeData(0x2B);
	_writeData(0x2B);
//	_writeData(0x09);
	
	_writeCommand(BOOSTER_SOFT_START); // 0x06
	_writeData(0x07);
	_writeData(0x07);
	_writeData(0x17);
	
	#if 0
    // Power optimization
    _writeCommand(0xF8);
    _writeData(0x60);
    _writeData(0xA5);
    // Power optimization
    _writeCommand(0xF8);
    _writeData(0x89);
    _writeData(0xA5);
    // Power optimization
    _writeCommand(0xF8);
    _writeData(0x90);
    _writeData(0x00);
    // Power optimization
    _writeCommand(0xF8);
    _writeData(0x93);
    _writeData(0x2A);
    // Power optimization
    _writeCommand(0xF8);
    _writeData(0xA0);
    _writeData(0xA5);
    // Power optimization
    _writeCommand(0xF8);
    _writeData(0xA1);
    _writeData(0x00);
    // Power optimization
    _writeCommand(0xF8);
    _writeData(0x73);
    _writeData(0x41);	
	#endif
	
	_writeCommand(0x16); // RESET DFV_EN
	_writeData(0x00);
	
	_powerOn();
	
	_writeCommand(PANEL_SETTING); // 0x00
	_writeData(0xBF); // KW-BF   KWR-AF  BWROTP 0f
//	_writeData(0xAF);
	
	_writeCommand(PLL_CONTROL); // 0x30
	_writeData(0x3A); // 90 50HZ  3A 100HZ   29 150Hz 39 200HZ 31 171HZ

	#if 1
	_writeCommand(RESOLUTION_SETTING); // 0x61
	_writeData(WAVESHARE_270_WIDTH >> 8);
	_writeData(WAVESHARE_270_WIDTH & 0xFF); // 176
	_writeData(WAVESHARE_270_HEIGHT >> 8);
	_writeData(WAVESHARE_270_HEIGHT & 0xFF); // 264
	#endif
	
	_writeCommand(VCM_DC_SETTING);  // 0x82
	_writeData(0x08); // 0x28:-2.0V, 0x12:-0.9V
//	_writeData(0x12);	
	
	delay(2);
	
	_writeCommand(VCOM_AND_DATA_INTERVAL_SETTING); // 0x50
	_writeData(0x97);
	
//	_setLUTFull();
//	_powerOn();	
}
	
void EPaper_Waveshare270::_powerOn()
{
	_writeCommand(POWER_ON); // 0x04
	_waitWhileBusy(power_on_time+1000);
}

void EPaper_Waveshare270::_powerOff()
{
	_writeCommand(POWER_OFF); // 0x02
	_waitWhileBusy(power_off_time+1000);
}

void EPaper_Waveshare270::_deepSleep()
{
	_writeCommand(DEEP_SLEEP); // 0x07
	_writeData(0xA5);
}

void EPaper_Waveshare270::_setLUTFull()
{
	Serial.println("_setLUTFull()");
	_writeCommand(VCM_DC_SETTING);  // 0x82
	_writeData(0x08); 
	_writeCommand(VCOM_AND_DATA_INTERVAL_SETTING);  // 0x50
	_writeData(0x97); 

	_writeCommand(LUT_FOR_VCOM); // 0x20
	_writeData(lut_20_vcom_dc_full, sizeof(lut_20_vcom_dc_full));
		
	_writeCommand(LUT_WHITE_TO_WHITE); // 0x21
	_writeData(lut_21_ww_full, sizeof(lut_21_ww_full));
	
	_writeCommand(LUT_BLACK_TO_WHITE); // 0x22
	_writeData(lut_22_bw_full, sizeof(lut_22_bw_full));
	
	_writeCommand(LUT_WHITE_TO_BLACK); // 0x23
	_writeData(lut_23_wb_full, sizeof(lut_23_wb_full));
	
	_writeCommand(LUT_BLACK_TO_BLACK); // 0x24
	_writeData(lut_24_bb_full, sizeof(lut_24_bb_full));
	
	_state |= _FULL_MODE;
}

void EPaper_Waveshare270::_setLUTPartial()
{
	Serial.println("_setLUTPartial()");
	_writeCommand(VCM_DC_SETTING);  // 0x82
	_writeData(0x08); 
	_writeCommand(VCOM_AND_DATA_INTERVAL_SETTING);  // 0x50
	_writeData(0x17); 

	_writeCommand(LUT_FOR_VCOM); // 0x20
	_writeData(lut_20_vcom_dc_partial, sizeof(lut_20_vcom_dc_partial));
		
	_writeCommand(LUT_WHITE_TO_WHITE); // 0x21
	_writeData(lut_21_ww_partial, sizeof(lut_21_ww_partial));
	
	_writeCommand(LUT_BLACK_TO_WHITE); // 0x22
	_writeData(lut_22_bw_partial, sizeof(lut_22_bw_partial));
	
	_writeCommand(LUT_WHITE_TO_BLACK); // 0x23
	_writeData(lut_23_wb_partial, sizeof(lut_23_wb_partial));
	
	_writeCommand(LUT_BLACK_TO_BLACK); // 0x24
	_writeData(lut_24_bb_partial, sizeof(lut_24_bb_partial));
	
	_state &= ~_FULL_MODE;
}

void  EPaper_Waveshare270::_setPartialWindow(uint8_t command, uint16_t x, uint16_t y, uint16_t w, uint16_t h)
{
	_writeCommand(command);
	_writeData(x >> 8);
	_writeData(x & 0xF8);
	_writeData(y >> 8);
	_writeData(y & 0xFF);
	_writeData(w >> 8);
	_writeData(w & 0xF8);
	_writeData(h >> 8);
	_writeData(h & 0xFF);
}

void EPaper_Waveshare270::_transfer(uint8_t command, const uint8_t * buffer)
{
	Serial.println("_transfer()");
	delay(1);
	
//	_writeCommand(command);
	_setPartialWindow(command, 0, 0, WAVESHARE_270_WIDTH, WAVESHARE_270_HEIGHT);
	
	for (int i = 0; i < WAVESHARE_270_WIDTH / 8 * WAVESHARE_270_HEIGHT; i++)
		_writeData(buffer[i]);
	
	delay(1);
}