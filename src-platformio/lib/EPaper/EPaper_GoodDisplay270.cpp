// EPaper_GoodDisplay270.cpp
//

#include "EPaper_GoodDisplay270.h"

#define Y_DEC               0x00
#define Y_INC               0x02
#define X_DEC               0x00
#define X_INC               0x01
#define AM_X                0x00
#define AM_Y                0x04

#define DATA_ENTRYMODE      (AM_Y | X_INC | Y_INC)

//
// setup
//  init pins: BUSY, RESET, DC, CS
//  init SPI: settings(20Mhz, MSBFIRST, SPI_MODE0), VSPI(18, 19, 23, 5)
//
// case1
//  hw_init
//      RST = 0
//      delay 10
//      RST = 1
//      delay 10
//      wait_busy
//      write_cmd 0x12 (SWRESET)
//      wait_busy
//  write_screen
//      write_cmd 0x24 (write data to RAM)
//      (repeat epd_w / 8 * epd_h times)
//          write_data image-data
//      update
//          write_cmd 0x22 (display update control)
//          write_data 0xF7
//          write_cmd 0x20 (activate display update sequence)
//          wait_busy
//  deep_sleep
//      write_cmd 0x10 (enter deep sleep)
//      write_data 0x01
//      delay 100ms
//
// case2
//  hw_init_fast
//      RST = 0
//      delay 10
//      RST = 1
//      delay 10
//      wait_busy
//      write_cmd 0x12 (SWRESET)
//      wait_busy
//      write_cmd 0x18 (temperature sensor control)
//      write_data 0x80 (internal temperature sensor)
//      write_cmd 0x22 (display update control 2)
//      write_data 0xB1 (load temerature value)
//      write_cmd 0x20 (acive display update sequence)
//      wait_busy
//      write_cmd 0x1A (write to temperature register)
//      write_data 0x64
//      write_data 0x00
//      write_cmd 0x22
//      write_data 0x91
//      write_cmd 0x20
//      wait_busy
//  write_screen_fast
//      write_cmd 0x24 (write data to RAM)
//      (repeat epd_w / 8 * epd_h times)
//          write_data image-data
//      update_fast
//          write_cmd 0x22 (display update control)
//          write_data 0xC7
//          write_cmd 0x20 (activate display update sequence)
//          wait_busy
//  deep_sleep
//
// case3
//  hw_init_fast2
//      RST = 0
//      delay 10
//      RST = 1
//      delay 10
//      wait_busy
//      write_cmd 0x12 (SWRESET)
//      wait_busy
//      write_cmd 0x18 (read built-in temperature sensor)
//      write_data 0x80
//      write_cmd 0x22 (load temperature value)  ?????
//      write_data 0xB1
//      write_cmd 0x20
//      wait_busy
//      write_cmd 0x1A (write to temperature register)
//      write_data 0x5A
//      write_data 0x00
//      write_cmd 0x22
//      write_data 0x91
//      write_cmd 0x20
//      wait_busy
//  write_screen_fast2
//      write_cmd 0x24 (write data to RAM)
//      (repeat epd_w / 8 * epd_h times)
//          write_data image-data
//      write_cmd 0x26 (write data to RAM)
//      (repeat epd_w / 8 * epd_h times)
//          write_data 0x00
//      update_fast
//          write_cmd 0x22 (display update control)
//          write_data 0xC7
//          write_cmd 0x20 (activate display update sequence)
//          wait_busy
//  deep_sleep
//
// case4
//  hw_init
//  set_ram_value_basemap(?)
//  write_partial or write_partial_all
//      write_partial: x, y, w, h, data
//          write_partial_data
//              RST = 0
//              delay 10
//              RST = 1
//              delay 10
//              write_cmd 0x3C (boarder-waveform)
//              write_data 0x80
//              write_cmd 0x44 (set x-address start/end)
//              write_data x / 8
//              write_data x / 8 + w / 8
//              write_cmd 0x45 (set y-address start/end)
//              write_data y % 256
//              write_data y / 256
//              write_data (y + h) % 256
//              write_data (y + h) / 256
//              write_cmd 0x4E (set x-address count to 0)
//              write_data x / 8
//              write_cmd 0x4F (set y-address count to 0x127)
//              wirte_data y % 256
//              write_data y / 256
//              write_cmd 0x24 (write image to RAM)
//              (repeat w / 8 * h times)
//                  write_data partial-data
//          partial_update
//              write_cmd 0x22 (display update control)
//              write_data 0xFF
//              write_cmd 0x20 (activate display update sequence)
//              wait_busy
//
//      write_partial_all
//          write_fullscreen
//              RST = 0
//              delay 10
//              RST = 1
//              delay 10
//              write_cmd 0x3C (boarder-waveform)
//              write_data 0x80
//              write_cmd 0x24 (write image to RAM)
//              (repeat epd_w / 8 * epd_h times)
//                  write_data image-data
//          update_partial
//              ...
//  deep_sleep


////////////////////////////////////////////////////////////////////////////////////////////////////////
// class EPaper_GoodDisplay270

EPaper_GoodDisplay270::EPaper_GoodDisplay270(PinSetting * pins)
	: EPaperDriver(pins)
{
}


void EPaper_GoodDisplay270::init()
{
	EPaperDriver::init();
	
	//_reset();

    //_waitWhileBusy(-1);
    //_writeCommand(0x12); // SWRESET
    //_waitWhileBusy(-1);
}

void EPaper_GoodDisplay270::refresh(bool fast_mode)
{	
	if (fast_mode)
	{
        partialUpdate();
	}
	else
	{
        fastUpdateAlt();
        // or
        //fullUpdate();
	}
}

void EPaper_GoodDisplay270::refresh(uint32_t update)
{
    if (update == 0)
    {
        fullUpdate();
    }
    else
    {
        if ((update % 10) == 0)
            fastUpdate(); // or fastUpdateAlt()
        else
            partialUpdate();
    }
}

void EPaper_GoodDisplay270::powerOn()
{
	//if (! (_state & _POWER_ON))
	//	_powerOn();	
}

void EPaper_GoodDisplay270::powerOff()
{
	//if (_state & _POWER_ON)
	//	_powerOff();	
}

void EPaper_GoodDisplay270::deepSleep()
{
	//if (! (_state & _DEEP_SLEEP))
	{
		//
		//powerOff();

		//
		_deepSleep();
	}
}




////////////////////////////////////////////////////////////////////////////////////////////////////////
//
	
void EPaper_GoodDisplay270::_powerOn()
{
	//_writeCommand(POWER_ON); // 0x04
	//_waitWhileBusy(power_on_time);
	
	_state |= _POWER_ON;
}

void EPaper_GoodDisplay270::_powerOff()
{
	//_writeCommand(POWER_OFF); // 0x02
	//_waitWhileBusy(power_off_time);
	
	_state &= ~(_POWER_ON | _FAST_MODE);
}

void EPaper_GoodDisplay270::_deepSleep()
{
	_writeCommand(0x10);
	_writeData(0x01);
	
	_state |= _DEEP_SLEEP;
}


void EPaper_GoodDisplay270::fullUpdate()
{
	// EPD_HW_Init
	setPinState(&_pin_settings[_PIN_RST], PIN_STATE_ACTIVE);
	_delay(10);
	setPinState(&_pin_settings[_PIN_RST], PIN_STATE_INACTIVE);
	_delay(10);

    _waitWhileBusy(-1);
    _writeCommand(0x12);
    _waitWhileBusy(-1);

    //_writeCommand(0x11);
    //_writeData(DATA_ENTRYMODE);    

    // EPD_WriteScreen_All
    _writeCommand(0x24);
    for (int i = 0; i < GOODDISPLAY_270_WIDTH * GOODDISPLAY_270_HEIGHT / 8; i++)
        _writeData(buffer[i]);

    // EPD_Update
    _writeCommand(0x22);
    _writeData(0xF7);
    _writeCommand(0x20);
    _waitWhileBusy(-1);

    // EPD_DeepSleep
    _deepSleep();
}

void EPaper_GoodDisplay270::fastUpdate()
{
    // EPD_HW_Init_Fast
	setPinState(&_pin_settings[_PIN_RST], PIN_STATE_ACTIVE);
	_delay(10);
	setPinState(&_pin_settings[_PIN_RST], PIN_STATE_INACTIVE);
	_delay(10);

    _waitWhileBusy(-1);
    _writeCommand(0x12);
    _waitWhileBusy(-1);

    _writeCommand(0x18);
    _writeData(0x80);

    _writeCommand(0x22); // Load temperature value
    _writeData(0xB1);
    _writeCommand(0x20);
    _waitWhileBusy(-1);

    _writeCommand(0x1A); // Write to temperature register
    _writeData(0x64);
    _writeData(0x00);
            
    _writeCommand(0x22); // Load temperature value
    _writeData(0x91);
    _writeCommand(0x20); 
    _waitWhileBusy(-1);

    //_writeCommand(0x11);
    //_writeData(DATA_ENTRYMODE);    

    // EPD_WriteScreen_All_Fast
    _writeCommand(0x24);
    for (int i = 0; i < GOODDISPLAY_270_WIDTH * GOODDISPLAY_270_HEIGHT / 8; i++)
        _writeData(buffer[i]);

    // EPD_Update_Fast
    _writeCommand(0x22); //Display Update Control
    _writeData(0xC7);   
    _writeCommand(0x20); //Activate Display Update Sequence
    _waitWhileBusy(-1);

    // EPD_DeepSleep
    _deepSleep();
}

void EPaper_GoodDisplay270::fastUpdateAlt()
{
    // EPD_HW_Init_Fast2
	setPinState(&_pin_settings[_PIN_RST], PIN_STATE_ACTIVE);
	_delay(10);
	setPinState(&_pin_settings[_PIN_RST], PIN_STATE_INACTIVE);
	_delay(10);

    _waitWhileBusy(-1);
    _writeCommand(0x12);
    _waitWhileBusy(-1);

    _writeCommand(0x18); //Read built-in temperature sensor
    _writeData(0x80);  
        
    _writeCommand(0x22); // Load temperature value
    _writeData(0xB1);
    _writeCommand(0x20);
    _waitWhileBusy(-1);

    _writeCommand(0x1A); // Write to temperature register
    _writeData(0x5A);
    _writeData(0x00);
            
    _writeCommand(0x22); // Load temperature value
    _writeData(0x91);
    _writeCommand(0x20);
    _waitWhileBusy(-1);

    //_writeCommand(0x11);
    //_writeData(DATA_ENTRYMODE);

    // EPD_WhiteScreen_ALL_Fast2
    _writeCommand(0x24);
    for (int i = 0; i < GOODDISPLAY_270_WIDTH * GOODDISPLAY_270_HEIGHT / 8; i++)
        _writeData(buffer[i]);

    _writeCommand(0x26);
    for (int i = 0; i < GOODDISPLAY_270_WIDTH * GOODDISPLAY_270_HEIGHT / 8; i++)
        _writeData(0x00);

    // EPD_Update_Fast
    _writeCommand(0x22); //Display Update Control
    _writeData(0xC7);   
    _writeCommand(0x20); //Activate Display Update Sequence
    _waitWhileBusy(-1);

    // EPD_DeepSleep
    _deepSleep();
}

void EPaper_GoodDisplay270::partialUpdate()
{
    // EPD_Dis_PartAll
	setPinState(&_pin_settings[_PIN_RST], PIN_STATE_ACTIVE);
	_delay(10);
	setPinState(&_pin_settings[_PIN_RST], PIN_STATE_INACTIVE);
	_delay(10);

    _writeCommand(0x3C); // Boarder Waveform
    _writeData(0x80);

    //_writeCommand(0x11);
    //_writeData(DATA_ENTRYMODE);    

    _writeCommand(0x24);
    for (int i = 0; i < GOODDISPLAY_270_WIDTH * GOODDISPLAY_270_HEIGHT / 8; i++)
        _writeData(buffer[i]);

    // EPD_Part_Update
    _writeCommand(0x22);
    _writeData(0xFF);
    _writeCommand(0x20);
    _waitWhileBusy(-1);

    // EPD_DeepSleep
    _deepSleep();
}
