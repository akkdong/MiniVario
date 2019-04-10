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
	
	timeCurrent = 15 * 3600 + 24 * 60 + 32;
	timeFly = 23 * 60 + 59;
	
	batteryPower = 3.7;
	stateGPS = 1;
	stateBT = 1;
	stateStorage = 1;
	
	vario.speedGround = 45.46;
	vario.speedVertActive = 4.56;
	vario.heading = 270.4;
	vario.glideRatio = 8.45;
	vario.altitudeGPS = 1564.6;
	
	
}