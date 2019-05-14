// BluetoothMan.h
//

#ifndef __BLUETOOTHMAN_H__
#define __BLUETOOTHMAN_H__

#include "FS.h"
#include "SD_MMC.h"


#if 0
#define BTMAN_BLOCK_NONE		(0)
#define BTMAN_BLOCK_NMEA		(1<<0)
#define BTMAN_BLOCK_SENSOR		(1<<1)
#define BTMAN_BLOCK_MASK		(BTMAN_BLOCK_NMEA|BTMAN_BLOCK_SENSOR)
#endif

class NmeaParserEx;
class VarioSentence;
class BluetoothSerialEx;


/////////////////////////////////////////////////////////////////////////////
// class BluetoothMan

class BluetoothMan
{
public:
	BluetoothMan(BluetoothSerialEx & serial, NmeaParserEx & nmea, VarioSentence & vario);	
	
public:
	void				begin();
	void				end();

	void				update();
	
	int					available();
	int					read();

	virtual void		startLogging(const char * file) {}
	virtual void		stopLogging() {}

protected:
	virtual void		writeVarioSentence();
	virtual void		writeGPSSentence();
	
protected:
	//
	uint8_t				lockState;
	#if 0
	uint8_t				blockTransfer;	// block transfer of nmea or sensor
	#endif
	
	//
	BluetoothSerialEx &	serialBluetooth;

	NmeaParserEx &		nmeaParser;
	VarioSentence &		varioSentence;
};


/////////////////////////////////////////////////////////////////////////////
// class BluetoothManEx

class BluetoothManEx : public BluetoothMan
{
public:
	BluetoothManEx(BluetoothSerialEx & serial, NmeaParserEx & nmea, VarioSentence & vario);	

	virtual void		startLogging(time_t date);
	virtual void		stopLogging();

protected:
	virtual void		writeVarioSentence();
	virtual void		writeGPSSentence();

protected:
	File				fileLog;
};


#endif // __BLUETOOTHMAN_H__
