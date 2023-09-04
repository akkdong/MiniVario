// EEPROMDriver.h
//

#ifndef __EEPROMDRIVER_H__
#define __EEPROMDRIVER_H__

#include <Arduino.h>
#include <Wire.h>


///////////////////////////////////////////////////////////////////////////////////////////////
// class EEPROMDriver

class EEPROMDriver
{
public:
	EEPROMDriver(TwoWire & wire);
	
public:
	void				writeByte(unsigned char devAddr, unsigned short memAddr, unsigned char data);
	void				writeBuffer(unsigned char devAddr, unsigned short memAddr, unsigned char * data, short dataLen);
	
	unsigned char		readByte(unsigned char devAddr, unsigned short memAddr);
	void				readBuffer(unsigned char devAddr, unsigned short memAddr, unsigned char * buf, short bufLen);
	
protected:
	int8_t			waitComplete(unsigned char devAddr, uint16_t timeout);

private:
	TwoWire &			Wire;
};


#endif // __EEPROMDRIVER_H__