// EPaper.ino
//
// Waveshare E-Paper  2.7, 4.2 Test

#include "EPaperDisplay.h"
#include "Bitmap_Paragliding.h"

#include <Fonts/FreeMonoBold24pt7b.h>
#include <Fonts/FreeMonoBold9pt7b.h>


//
PinSetting ePaperPins[] = 
{
	// CS
	{ SS, PIN_MODE_OUTPUT, PIN_ACTIVE_LOW, PIN_STATE_INACTIVE },
	// DC
	{ 33, PIN_MODE_OUTPUT, PIN_ACTIVE_LOW, PIN_STATE_INACTIVE },
	// RST
	{ 32, PIN_MODE_OUTPUT, PIN_ACTIVE_LOW, PIN_STATE_INACTIVE },
	// BUSY
	{ 39, PIN_MODE_INPUT, PIN_ACTIVE_LOW, PIN_STATE_INACTIVE },
};

EPaper_Waveshare270 driver(ePaperPins);
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
	display.drawBitmapBM(Bitmap_Paragliding, 6, 24, 164, 166, COLOR_WHITE, bm_invert);
	
	display.refresh(false);
	delay(2000);
	
	//
	display.fillScreen(COLOR_WHITE);
	display.fillTriangle(80, 20, 20, 150, 150, 150, COLOR_BLACK);
	display.drawFastHLine(10, 240, 160, COLOR_BLACK);
	
	display.refresh(true);
	delay(1000);
	
	int Number = 3500;
	while( ++Number <= 3520)
	{
		display.drawFastHLine(10, 240, 160, COLOR_BLACK);
		display.fillRect(10, 240-20, 160, 40, COLOR_WHITE);
		display.setTextColor(COLOR_BLACK, COLOR_WHITE);
		display.setCursor(10, 240);
		display.print("> ");
		display.print(Number);
		
		display.refresh(true);
	}
	
	//
	display.setRotation(0);
	display.fillScreen(COLOR_WHITE);
	display.setFont(&FreeMonoBold9pt7b);
	display.setTextColor(COLOR_BLACK, COLOR_WHITE);
	display.drawBitmapBM(Bitmap_Paragliding, 6, 24, 164, 166, COLOR_WHITE, bm_invert);
	display.setCursor(0, 242);
	display.print("Fly high~");
	display.setCursor(0, 256);
	display.print("Fly away!");
	
	display.refresh(false);
	display.sleep();
}

void loop()
{
	delay(10000);
}
