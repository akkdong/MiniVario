// EPaper_GoodDisplay270.h
//

#include "EPaperDriver.h"

#define GOODDISPLAY_270_WIDTH		(176)
#define GOODDISPLAY_270_HEIGHT	    (264)



////////////////////////////////////////////////////////////////////////////////////////////////////////
//

class EPaper_GoodDisplay270 : public EPaperDriver
{
public:
	EPaper_GoodDisplay270(PinSetting * pins);
	
public:
	void 						init();
	
	int16_t						getWidth() { return GOODDISPLAY_270_WIDTH; }
	int16_t						getHeight() { return GOODDISPLAY_270_HEIGHT; }
	
	uint8_t *					getBuffer() { return &buffer[0];  }
	
	void						refresh(bool fast_mode = true);
	
	void						powerOn();
	void						powerOff(); // turns off generation of panel driving voltages, avoids screen fading over time
	void						deepSleep(); // turns off power & sets controller to deep sleep, ONLY wakeable by RST
	
private:
	void						_powerOn();
	void						_powerOff();
	void						_deepSleep();



    //
    void                        fullUpdate();
    void                        fastUpdate();
    void                        fastUpdateAlt();
    void                        partialUpdate();
	
public:	
    static const uint16_t 		power_on_time = 120; // ms, e.g. 98877us
    static const uint16_t 		power_off_time = 40; // ms, e.g. 28405us
    static const uint16_t 		full_refresh_time = 2200; // ms, e.g. 1979027us
    static const uint16_t 		partial_refresh_time = 400; // ms, e.g. 363637us
	
private:
	
private:
	uint8_t						buffer[GOODDISPLAY_270_WIDTH / 8 * GOODDISPLAY_270_HEIGHT];	
};
