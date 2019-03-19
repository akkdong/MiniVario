#include "BluetoothSerial.h"
 
BluetoothSerial SerialBT;
 
void setup() {
  Serial.begin(115200);
 
  if(!SerialBT.begin("Abrakatabra")){
    Serial.println("An error occurred initializing Bluetooth");
  }
}
 
void loop() {
 
  while(SerialBT.available()) {
    Serial.write(SerialBT.read());
  }
  
  while(Serial.available()) {
	  SerialBT.write(Serial.read());
  }
 
  delay(50);
}