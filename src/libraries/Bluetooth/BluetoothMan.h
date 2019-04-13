// BluetoothMan.h
//

#ifndef __BLUETOOTHMAN_H__
#define __BLUETOOTHMAN_H__

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
	void				update();
	
	int					available();
	int					read();

private:
	void				writeVarioSentence();
	void				writeGPSSentence();
	
private:
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

#endif // __BLUETOOTHMAN_H__
