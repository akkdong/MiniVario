// EPaper_Waveshare270.h
//

#include "EPaperDriver.h"

#define WAVESHARE_270_WIDTH		(176)
#define WAVESHARE_270_HEIGHT	(264)



////////////////////////////////////////////////////////////////////////////////////////////////////////
//

class EPaper_Waveshare270 : public EPaperDriver
{
public:
	EPaper_Waveshare270(PinSetting * pins);
	
public:
	void 						init();
	
	int16_t						getWidth() { return WAVESHARE_270_WIDTH; }
	int16_t						getHeight() { return WAVESHARE_270_HEIGHT; }
	
	uint8_t *					getBuffer() { return &buffer[0];  }
	
	void						refresh(bool fast_mode = true);
	
	void						powerOn();
	void						powerOff(); // turns off generation of panel driving voltages, avoids screen fading over time
	void						deepSleep(); // turns off power & sets controller to deep sleep, ONLY wakeable by RST
	
private:
	void						_initDisplay();

	void						_powerOn();
	void						_powerOff();
	void						_deepSleep();
	
	void						_initFullMode();
	void						_initFastMode();
	
	void						_setPartialWindow(uint8_t command, uint16_t x, uint16_t y, uint16_t w, uint16_t h);	
	void						_refreshPartialWindow(uint16_t x, uint16_t y, uint16_t w, uint16_t h);
	
	void						_transfer(uint8_t command, const uint8_t * buffer);
	
public:	
    static const uint16_t 		power_on_time = 120; // ms, e.g. 98877us
    static const uint16_t 		power_off_time = 40; // ms, e.g. 28405us
    static const uint16_t 		full_refresh_time = 2200; // ms, e.g. 1979027us
    static const uint16_t 		partial_refresh_time = 400; // ms, e.g. 363637us
	
private:
	static const uint8_t		lut_20_vcom_dc_full[];
    static const uint8_t		lut_21_ww_full[];
    static const uint8_t		lut_22_bw_full[];
    static const uint8_t		lut_23_wb_full[];
    static const uint8_t		lut_24_bb_full[];
	
    static const uint8_t		lut_20_vcom_dc_partial[];
    static const uint8_t		lut_21_ww_partial[];
    static const uint8_t		lut_22_bw_partial[];
    static const uint8_t		lut_23_wb_partial[];
    static const uint8_t		lut_24_bb_partial[];	
	
private:
	uint8_t						buffer[WAVESHARE_270_WIDTH / 8 * WAVESHARE_270_HEIGHT];	
	bool						_initial;
};
