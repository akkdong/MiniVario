// EPaperDisplay.h
//

#include <Adafruit_GFX.h>

#include "EPaper_Waveshare270.h"
#include "EPaper_Waveshare420.h"

#define COLOR_BLACK		0x00
#define COLOR_WHITE		0xFF

enum bm_mode //BM_ModeSet
{
	bm_normal = 0,
	bm_default = 1, // for use for BitmapExamples
	// these potentially can be combined
	bm_invert = (1 << 1),
	bm_flip_x = (1 << 2),
	bm_flip_y = (1 << 3),
	bm_r90 = (1 << 4),
	bm_r180 = (1 << 5),
	bm_r270 = bm_r90 | bm_r180,
	bm_partial_update = (1 << 6),
	bm_invert_red = (1 << 7),
	bm_transparent = (1 << 8)
};

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
	virtual void 					drawPixel(int16_t x, int16_t y, uint16_t color);

	//
	virtual void					init();
	
	void							setRotation(uint8_t r);
	void							invertDisplay(boolean i);
	
	void							fillScreen(uint16_t color); // 0x00: black, > 0x00 : white
	void							fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);
	
	void							refresh(bool fast_update = false);
	
	void							sleep();
	
	//
	void							drawBitmapBM(const uint8_t * bitmap, uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color, int16_t mode);
	
private:
	template <typename T> static inline void _swap_(T & a, T & b) { T t = a; a = b; b = t;  }
	static inline uint16_t __max(uint16_t a, uint16_t b) { return (a > b ? a : b); }
	static inline uint16_t __min(uint16_t a, uint16_t b) { return (a < b ? a : b); }
	
	void							_rotate(uint16_t & x, uint16_t & y, uint16_t & w, uint16_t & h);
	
protected:
	EPaperDriver &					_epd;
	
	uint8_t *						_buffer;
};

