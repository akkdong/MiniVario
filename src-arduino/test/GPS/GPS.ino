// GPS.ino
//

void setup()
{
	//
	digitalWrite(19, HIGH);
	pinMode(19, OUTPUT);	
	
	//
	Serial.begin(115200);
	Serial.println("GPS Test...");
	
	//
	Serial2.begin(9600);
	delay(100);	
}

void loop()
{
	while(Serial.available()) 
		Serial2.write(Serial.read());

	while (Serial2.available())
		Serial.write(Serial2.read());
	
	delay(1);
}