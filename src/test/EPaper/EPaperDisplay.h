// EPaperDisplay.h
//

#include <Adafruit_GFX.h>

#include "EPaper_Waveshare270.h"
#include "EPaper_Waveshare420.h"

#define COLOR_BLACK		0xFF
#define COLOR_WHITE		0x00


////////////////////////////////////////////////////////////////////////////////////////////////////////
//

class EPaperDisplay : public Adafruit_GFX
{
public:
	EPaperDisplay(EPaperDriver & driver);
	
public:
	///  virtual drawPixel() function to draw to the screen/framebuffer/etc. 
	//   @param x X coordinate.  
	//   @param y Y coordinate. 
	//   @param color 16-bit pixel color. 
	void 							drawPixel(int16_t x, int16_t y, uint16_t color);

	//
	void							init();
	
	void							setRotation(uint8_t r);
	void							invertDisplay(boolean i);
	
	void							fillScreen(uint16_t color); // 0x00: black, > 0x00 : white
	void							fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);
	
	void							flush(bool fast_update = false);
	
	void							sleep();
	
private:
	template <typename T> static inline void _swap_(T & a, T & b) { T t = a; a = b; b = t;  }
	static inline uint16_t __max(uint16_t a, uint16_t b) { return (a > b ? a : b); }
	static inline uint16_t __min(uint16_t a, uint16_t b) { return (a < b ? a : b); }
	
	void							_rotate(uint16_t & x, uint16_t & y, uint16_t & w, uint16_t & h);
	
protected:
	EPaperDriver &			_epd;
	
	uint8_t *					_buffer;
};

