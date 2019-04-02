#if defined(ESP8266) || defined(ESP32)
#include <pgmspace.h>
#else
#include <avr/pgmspace.h>
#endif
// 24 x 24 gridicons_resize
const unsigned char gridicons_resize[] PROGMEM = { /* 0X01,0X01,0XB4,0X00,0X40,0X00, */
0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
0xFF, 0xFF, 0xFF, 0xFF, 0xF0, 0x1F, 0xFF, 0xF0, 
0x1F, 0xFF, 0xF0, 0xFF, 0xFF, 0xF0, 0x7F, 0xFF, 
0xF2, 0x3F, 0xFF, 0xF3, 0x1F, 0xFF, 0xF3, 0x8F, 
0xFF, 0xFF, 0xC7, 0xFF, 0xFF, 0xE3, 0xFF, 0xFF, 
0xF1, 0xCF, 0xFF, 0xF8, 0xCF, 0xFF, 0xFC, 0x4F, 
0xFF, 0xFE, 0x0F, 0xFF, 0xFF, 0x0F, 0xFF, 0xF8, 
0x0F, 0xFF, 0xF8, 0x0F, 0xFF, 0xFF, 0xFF, 0xFF, 
0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF
};
