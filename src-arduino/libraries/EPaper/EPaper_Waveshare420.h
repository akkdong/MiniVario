// EPaper_Waveshare420.h
//

#include "EPaperDriver.h"

#define WAVESHARE_420_WIDTH		(400)
#define WAVESHARE_420_HEIGHT		(300)


////////////////////////////////////////////////////////////////////////////////////////////////////////
//

class EPaper_Waveshare420 : public EPaperDriver
{
public:
	EPaper_Waveshare420(PinSetting * pins);
	
public:
	void 							init();
	
	int16_t						getWidth() { return WAVESHARE_420_WIDTH; }
	int16_t						getHeight() { return WAVESHARE_420_HEIGHT; }
	
	uint8_t *					getBuffer() { return &buffer[0];  }
	
	void							transfer();
	void							refresh(bool fast_mode = true);
	
	void							powerOff(); // turns off generation of panel driving voltages, avoids screen fading over time
	void							deepSleep(); // turns off power & sets controller to deep sleep, ONLY wakeable by RST

	
private:
	void							_initDisplay();

	void							_powerOn();
	void							_powerOff();
	
	void							_setLUTFull();
	void							_setLUTPartial();
	
	void							_setPartialWindow(uint16_t x, uint16_t y, uint16_t w, uint16_t h);
	
	void							_updatePartialWindow(const uint8_t * buffer, uint16_t x, uint16_t y, uint16_t w, uint16_t h);
	void							_updateFullWindow(const uint8_t * buffer);
	
	void							_transfer(uint8_t command, const uint8_t * buffer);


public:	
    static const uint16_t 	power_on_time = 40; // ms, e.g. 36996us
    static const uint16_t 	power_off_time = 42; // ms, e.g. 40026us
    static const uint16_t 	full_refresh_time = 4200; // ms, e.g. 4108541us
    static const uint16_t 	partial_refresh_time = 2000; // ms, e.g. 995320us
	
private:
	static const uint8_t		lut_20_vcom0_full[];
    static const uint8_t		lut_21_ww_full[];
    static const uint8_t		lut_22_bw_full[];
    static const uint8_t		lut_23_wb_full[];
    static const uint8_t		lut_24_bb_full[];
	
    static const uint8_t		lut_20_vcom0_partial[];
    static const uint8_t		lut_21_ww_partial[];
    static const uint8_t		lut_22_bw_partial[];
    static const uint8_t		lut_23_wb_partial[];
    static const uint8_t		lut_24_bb_partial[];	
	
private:
	uint8_t						buffer[WAVESHARE_420_WIDTH / 8 * WAVESHARE_420_HEIGHT];
};
