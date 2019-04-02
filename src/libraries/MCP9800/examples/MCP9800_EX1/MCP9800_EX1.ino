/*----------------------------------------------------------------------*
 * Example sketch for Microchip MCP9800/1/2/3 2-Wire High-Accuracy      *
 * Temperature Sensors. Demonstrates reading of ambient temperature     *
 * in Celsius and Fahrenheit, conversion to floating-point,             *
 * changing device options via the Cofiguration register,               *
 * and changing the Limit-Set and Hysteresis registers.                 *
 *                                                                      *
 * Jack Christensen 28Aug2013                                           *
 *                                                                      *
 * Tested with Arduino 1.8.5 and an Arduino Uno.                        *
 *                                                                      *
 * This work is licensed under CC BY-SA 4.0,                            *
 * http://creativecommons.org/licenses/by-sa/4.0/                       *
 *----------------------------------------------------------------------*/ 
 
#include <MCP9800.h>      // http://github.com/JChristensen/MCP9800
//#include <Streaming.h>    // http://arduiniana.org/libraries/streaming/

MCP9800 mySensor;

void setup()
{
  digitalWrite(19, HIGH);
  pinMode(19, OUTPUT);		
	
    Serial.begin(115200);
    delay(1000);
    //Serial << F( "\n" __FILE__ " " __DATE__ " " __TIME__ "\n" );
	Serial.print("\n"); Serial.print(__FILE__); Serial.print(" "); Serial.print(__DATE__); Serial.print(" "); Serial.println(__TIME__);

    mySensor.begin();        // initialize the hardware
    displayRegs();           // print the initial register values
    //Serial << endl;
	Serial.println("");
	

    mySensor.writeConfig(ADC_RES_12BITS);       // max resolution, 0.0625 °C
    mySensor.writeTempC2(HYSTERESIS, 0 * 2);    // freezing
    mySensor.writeTempC2(LIMITSET, 100 * 2);    // boiling
}

void loop()
{
    delay(2000);
    displayRegs();
}

// print the values of all four registers on the serial monitor
void displayRegs(void)
{
    uint8_t config = mySensor.readConfig();
    float C = mySensor.readTempC16(AMBIENT) / 16.0;
    float F = mySensor.readTempF10(AMBIENT) / 10.0;
//  Serial << F("Config=") << (config < 16 ? F("0x0") : F("0x")) << _HEX(config) << F(", Ambient ") << C << F("C ") << F << 'F';
     Serial.print("Config="); Serial.print(config < 16 ? "0x0" : "0x"); Serial.print(config); Serial.print(", Ambient "); Serial.print(C); Serial.print("C "); Serial.print(F); Serial.print("F");
    
    C = mySensor.readTempC16(HYSTERESIS) / 16.0;
    F = mySensor.readTempF10(HYSTERESIS) / 10.0;
//    Serial << F(", Hysteresis ") << C << F("C ") << F << 'F';
	Serial.print(", Hysteresis "); Serial.print(C); Serial.print("C "); Serial.print(F); Serial.print('F');

    C = mySensor.readTempC16(LIMITSET) / 16.0;
    F = mySensor.readTempF10(LIMITSET) / 10.0;
//    Serial << F(", Limit-Set ") << C << F("C ") << F << 'F' << endl;
    Serial.print(", Limit-Set "); Serial.print(C); Serial.print("C "); Serial.print(F); Serial.println('F');
}
