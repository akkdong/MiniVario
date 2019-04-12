// DeviceContext.cpp
//

#include "DeviceContext.h"


////////////////////////////////////////////////////////////////////////////////////////////////
// class DeviceContext

DeviceContext::DeviceContext()
{
	reset();
}
	
void DeviceContext::reset()
{
	memset(this, 0, sizeof(DeviceContext));
	
	#if 0
	device.batteryPower = 3.7;
	device.statusGPS = 1;
	device.statusBT = 1;
	device.statusSDCard = 1;
	
	varioState.speedGround = 45.46;
	varioState.speedVertActive = 4.56;
	varioState.heading = 270.4;
	varioState.glideRatio = 8.45;
	varioState.altitudeGPS = 1564.6;
	
	varioState.timeCurrent = 15 * 3600 + 24 * 60 + 32;
	varioState.timeStart = 14 * 3600 + 45 * 60 + 2;	
	#endif
}