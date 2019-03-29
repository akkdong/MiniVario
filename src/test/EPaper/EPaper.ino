// EPaper.ino
//

// Waveshare E-Paper  2.7, 4.2 Test

//#include <Fonts/FreeMonoBold9pt7b.h>

#include "EPaperDisplay.h"
#include <Fonts/FreeMonoBold24pt7b.h>


PinSetting ePaperPins[] = 
{
	// CS
	{ SS, PIN_MODE_OUTPUT, PIN_ACTIVE_LOW },
	// DC
	{ 17, PIN_MODE_OUTPUT, PIN_ACTIVE_LOW },
	// RST
	{ 16, PIN_MODE_OUTPUT, PIN_ACTIVE_LOW },
	// BUSY
	{ 4, PIN_MODE_INPUT, PIN_ACTIVE_LOW },
};

EPaper_Waveshare420 driver(ePaperPins);
EPaperDisplay display(driver);


void setup()
{
	Serial.begin(115200);
	Serial.println("EPaper Test...\n");
	
	display.init();
	
	display.setRotation(0);
	display.fillScreen(COLOR_WHITE);
	display.setFont(&FreeMonoBold24pt7b);
	display.setTextColor(COLOR_BLACK, COLOR_WHITE);
	display.setCursor(10, 100);
	display.print("Hello~");
	
	display.flush(false);
	Serial.println("flush Hello~");
	delay(1000);
	
	display.fillTriangle(200, 100, 300, 160, 100, 160, COLOR_BLACK);
	display.flush(true);
	Serial.println("flush Triangle");
	
	int Number = 3500;
	while( Number <= 3510)
	{
		//display.fillScreen(1);
		display.drawFastHLine(10, 260, 300, COLOR_BLACK);
		display.fillRect(10, 260-30, 360, 40, COLOR_WHITE);
		display.setCursor(10, 260);
		display.print("> ");
		display.print(Number++);
		display.flush(true);
		Serial.println("flush Number");
	}
	
	display.fillScreen(COLOR_WHITE);
	display.setFont(&FreeMonoBold24pt7b);
	display.setTextColor(COLOR_BLACK, COLOR_WHITE);
	display.setCursor(10, 100);
	display.print("Bye~");
	display.flush(false);
	Serial.println("flush Bye!!");
	
	display.sleep();
	Serial.println("go Sleep");
}

void loop()
{
	delay(10000);
}
