// EPaper_Waveshare270.h
//

#include "EPaperDriver.h"

#define WAVESHARE_270_WIDTH		(176)
#define WAVESHARE_170_HEIGHT		(240)


////////////////////////////////////////////////////////////////////////////////////////////////////////
//

class EPaper_Waveshare270 : public EPaperDriver
{
public:
	EPaper_Waveshare270(int8_t cs, int8_t dc, int8_t rst, int8_t busy) : EPaperDriver(cs, dc, rst, busy, WAVESHARE_270_WIDTH, WAVESHARE_170_HEIGHT) {}
};
