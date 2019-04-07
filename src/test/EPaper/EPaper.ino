// EPaper.ino
//

// Waveshare E-Paper  2.7, 4.2 Test


#include "EPaperDisplay.h"
#include "Bitmap_Paragliding.h"

#include <Fonts/FreeMonoBold24pt7b.h>
#include <Fonts/FreeMonoBold9pt7b.h>


PinSetting ePaperPins[] = 
{
	#if TEST_420
	// CS
	{ SS, PIN_MODE_OUTPUT, PIN_ACTIVE_LOW },
	// DC
	{ 17, PIN_MODE_OUTPUT, PIN_ACTIVE_LOW },
	// RST
	{ 16, PIN_MODE_OUTPUT, PIN_ACTIVE_LOW },
	// BUSY
	{ 4, PIN_MODE_INPUT, PIN_ACTIVE_LOW },
	#else // 270
	// CS
	{ SS, PIN_MODE_OUTPUT, PIN_ACTIVE_LOW },
	// DC
	{ 33, PIN_MODE_OUTPUT, PIN_ACTIVE_LOW },
	// RST
	{ 32, PIN_MODE_OUTPUT, PIN_ACTIVE_LOW },
	// BUSY
	{ 39, PIN_MODE_INPUT, PIN_ACTIVE_LOW },
	#endif
};

#if TEST_420
EPaper_Waveshare420 driver(ePaperPins);
#else // 270
EPaper_Waveshare270 driver(ePaperPins);
#endif
EPaperDisplay display(driver);


void setup()
{
	digitalWrite(19, HIGH);
	pinMode(19, OUTPUT);
	delay(10);
	
	Serial.begin(115200);
	Serial.println("EPaper Test...\n");
	
	display.init();
	
	//
	display.setRotation(0);
	display.fillScreen(COLOR_WHITE);
	display.setFont(&FreeMonoBold9pt7b);
	
	display.flush(false);
	Serial.println("clear Screen");
	delay(1000);
	
	//
	display.fillTriangle(80, 20, 20, 150, 150, 150, COLOR_BLACK);
	display.drawFastHLine(10, 240, 160, COLOR_BLACK);
	
	display.flush(true);
	Serial.println("draw triangle");
	
	int Number = 3500;
	while( ++Number <= 3520)
	{
		display.drawFastHLine(10, 240, 160, COLOR_BLACK);
		display.fillRect(10, 240-20, 160, 40, COLOR_WHITE);
		display.setTextColor(COLOR_BLACK, COLOR_WHITE);
		display.setCursor(10, 240);
		display.print("> ");
		display.print(Number);
		
		display.flush(true);
		Serial.print("draw Number "); Serial.println(Number);
	}
	
	//
	display.setRotation(0);
	display.fillScreen(COLOR_WHITE);
	display.setFont(&FreeMonoBold9pt7b);
	display.setTextColor(COLOR_BLACK, COLOR_WHITE);
	display.drawBitmapBM(Bitmap_Paragliding, 0, 10, 176, 178, COLOR_WHITE, bm_invert);
	display.setCursor(0, 216);
	display.print("Flight computer");
	
	display.flush(false);
	Serial.println("draw sleep display!!");
}

void loop()
{
	delay(10000);
}
