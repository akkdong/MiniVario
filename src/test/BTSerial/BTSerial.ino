#include "BluetoothSerialEx.h"
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"

BluetoothSerialEx SerialBT;
uint32_t tickUpdate;

void bluetoothSPPCallback(esp_spp_cb_event_t event, esp_spp_cb_param_t * param)
{
	switch (event)
	{
	case ESP_SPP_SRV_OPEN_EVT :
		Serial.println("Server connection open");
		break;
		
	case ESP_SPP_START_EVT :
		Serial.println("Server started");
		break;
		
	case ESP_SPP_OPEN_EVT :
		Serial.println("Client connection open");
		break;
		
	case ESP_SPP_CLOSE_EVT :
		Serial.println("Client connection closed");
		break;
		
	case ESP_SPP_CONG_EVT :
		Serial.println("Connection congestion status changed");
		break;
		
	case ESP_SPP_DISCOVERY_COMP_EVT :
		Serial.println("Discovery complete");
		break;
		
	case ESP_SPP_CL_INIT_EVT :
		Serial.println("Client initiated a connection");
		break;
	}	
}
 
void setup() 
{
//	WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); //disable brownout detector

	//
	digitalWrite(19, HIGH);
	pinMode(19, OUTPUT);	
	delay(10);

	//
	Serial.begin(115200);
	Serial2.begin(9600);

	//
	SerialBT.register_callback(bluetoothSPPCallback);
	
	if(!SerialBT.begin("MiniVario"))
	{
		Serial.println("An error occurred initializing Bluetooth");
		while(1);
	}
	
	tickUpdate = millis();
}
 
void loop() 
{
	while(Serial.available()) 
		SerialBT.write(Serial.read());

	while(SerialBT.available()) 
		Serial2.write(SerialBT.read());

	while(Serial2.available()) 
		SerialBT.write(Serial2.read());
	
	if (millis() - tickUpdate > 2000)
	{
		Serial.print("BluetoothSerialEx::hasClient = "); Serial.println(SerialBT.hasClient());
		tickUpdate = millis();
	}

	delay(1);
}