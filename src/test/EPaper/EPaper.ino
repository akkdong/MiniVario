// EPaper.ino
//

// Waveshare E-Paper  2.7, 4.2 Test


#include "EPaperDisplay.h"

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
	
	display.setRotation(0);
	display.fillScreen(COLOR_WHITE);
	display.setFont(&FreeMonoBold9pt7b);
	display.setTextColor(COLOR_BLACK, COLOR_WHITE);
	display.setCursor(10, 10);
	display.print("Hello~");
	
	display.flush(false);
	Serial.println("flush Hello~");
	delay(1000);
	
	display.fillTriangle(80, 20, 20, 150, 150, 150, COLOR_BLACK);
	display.flush(true);
	Serial.println("flush Triangle");
	
	int Number = 3500;
	while( Number <= 3510)
	{
		//display.fillScreen(1);
		display.drawFastHLine(10, 180, 160, COLOR_BLACK);
		display.fillRect(10, 180-20, 160, 40, COLOR_WHITE);
		display.setCursor(10, 180);
		display.print("> ");
		display.print(Number++);
		display.flush(true);
		Serial.println("flush Number");
	}
	
	display.fillScreen(COLOR_WHITE);
	display.setFont(&FreeMonoBold9pt7b);
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
