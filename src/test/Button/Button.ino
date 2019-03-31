// Button.ino
//

void setup()
{
	pinMode(35, INPUT);
	pinMode(34, INPUT);
	pinMode(0, INPUT);
	
	Serial.begin(115200);
	
	delay(100);
}

void loop()
{
	Serial.print("Key UP: "); Serial.println(digitalRead(34));
	Serial.print("Key DN: "); Serial.println(digitalRead(35));
	Serial.print("Key RE: "); Serial.println(digitalRead(0));
	delay(500);
}
