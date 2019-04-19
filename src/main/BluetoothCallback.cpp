// BluetoothCallback.cpp
//

#include "BluetoothCallback.h"
#include "DeviceContext.h"

/////////////////////////////////////////////////////////////////////////////////////////
//

static DeviceContext & context = __DeviceContext;


/////////////////////////////////////////////////////////////////////////////////////////
//

void bluetoothSPPCallback(esp_spp_cb_event_t event, esp_spp_cb_param_t * param)
{
	switch (event)
	{
	case ESP_SPP_START_EVT :
		Serial.println("Server started");
		context.device.statusBT = 1;
		break;

	case ESP_SPP_SRV_OPEN_EVT :
		Serial.println("Server connection open");
		if (context.device.statusBT)
			context.device.statusBT = 2;
		break;
		
	case ESP_SPP_OPEN_EVT :
		Serial.println("Client connection open");
		break;

	case ESP_SPP_CLOSE_EVT :
		Serial.println("Client connection closed");
		if (context.device.statusBT)
			context.device.statusBT = 1;
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
