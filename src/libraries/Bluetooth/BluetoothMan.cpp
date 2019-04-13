// BluetoothMan.cpp
//

#include "BluetoothSerialEx.h"
#include "NmeaParserEx.h"
#include "VarioSentence.h"
#include "BluetoothMan.h"

/*
#define CLEAR_STATE()		lockState = 0;
#define SET_STATE(bit)		lockState |= (1 << bit)
#define UNSET_STATE(bit)	lockState &= ~(1 << bit)

#define IS_SET(bit)			(lockState & (1 << bit))


#define LOCKED_BY_VARIO		1
#define LOCKED_BY_GPS			2
*/

enum BTMAN_LOCK_STATE
{
	BTMAN_UNLOCKED = 0,
	BTMAN_LOCKED_BY_VARIO,
	BTMAN_LOCKED_BY_GPS,
	BTMAN_LOCKED_BY_SENSOR,
	BTMAN_LOCKED_BY_RESPONSE,
};


/////////////////////////////////////////////////////////////////////////////
// class BluetoothMan

BluetoothMan::BluetoothMan(BluetoothSerialEx & serial, NmeaParserEx & nmea, VarioSentence & vario) 
	: serialBluetooth(serial)
	, nmeaParser(nmea)
	, varioSentence(vario)
	, lockState(BTMAN_UNLOCKED)/*, blockTransfer(BTMAN_BLOCK_NONE)*/
{
}


void BluetoothMan::update()
{
	//if (! serialBluetooth.isConnected())
	//	return;

	if (lockState == BTMAN_UNLOCKED)
	{
		if (varioSentence.available() /*&& ! (blockTransfer & BTMAN_BLOCK_NMEA)*/)
		{
			lockState = BTMAN_LOCKED_BY_VARIO;			
			writeVarioSentence();
		}
		else if (nmeaParser.available() /*&& ! (blockTransfer & BTMAN_BLOCK_NMEA)*/)
		{
			lockState = BTMAN_LOCKED_BY_GPS;
			writeGPSSentence();
		}
	}
	else
	{
		if (lockState == BTMAN_LOCKED_BY_VARIO)
		{
			// send Vario NMEA setence
			writeVarioSentence();
		}
		else if (lockState == BTMAN_LOCKED_BY_GPS)
		{
			// send GPS NMEA sentence
			writeGPSSentence();
		}
	}
}

void BluetoothMan::writeGPSSentence()
{
	while (serialBluetooth.availableForWrite())
	{
		//
		int c = nmeaParser.read();
		if (c < 0)
			break;
		
		//
		serialBluetooth.writeEx(c);
		
		if (c == '\n') // last setence character : every sentence end with '\r\n'
		{			
			lockState = BTMAN_UNLOCKED;
			break;
		}
	}
}

void BluetoothMan::writeVarioSentence()
{
	while (serialBluetooth.availableForWrite())
	{
		//
		int c = varioSentence.read();
		if (c < 0)
			break;
		
		//
		serialBluetooth.writeEx(c);
		
		if (c == '\n') // last setence character : every sentence end with '\r\n'
		{
			lockState = BTMAN_UNLOCKED;
			break;
		}
	}
}

int BluetoothMan::available()
{
	return serialBluetooth.available();
}

int BluetoothMan::read()
{
	return serialBluetooth.read();
}
