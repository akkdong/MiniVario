// GPS.ino
//


void setup()
{
	digitalWrite(19, HIGH);
	pinMode(19, OUTPUT);	
	
	Serial.begin(115200);
	Serial.println("GPS Test...");
	
	Serial2.begin(9600);
	delay(100);
	
	
}

void loop()
{
	while (Serial2.available())
	{
		int ch = Serial2.read();
		Serial.write(ch);
	}
}