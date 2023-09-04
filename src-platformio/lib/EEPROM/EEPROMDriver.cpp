// EEPROMDriver.cpp
//

#include "EEPROMDriver.h"

#define MAX_WRITE_BLOCK_SIZE	30
#define MAX_READ_BLOCK_SIZE		32

#define TIMEOUT_WAIT					500


///////////////////////////////////////////////////////////////////////////////////////////////
// class EEPROMDriver

EEPROMDriver::EEPROMDriver(TwoWire & wire) : Wire(wire)
{
}

int8_t EEPROMDriver::waitComplete(unsigned char devAddr, uint16_t timeout)
{
	uint32_t tick = millis();

	while (1)
	{
		Wire.beginTransmission(devAddr);
		if (Wire.endTransmission(true) ==0)
			break;
		
		if ((millis() - tick) > timeout)
			return -1;
	}
	
	return 0;
	
	//do
	//{
	//	Wire.beginTransmission(devAddr);
	//} while (Wire.endTransmission(true) != 0);
}

void EEPROMDriver::writeByte(unsigned char devAddr, unsigned short memAddr, unsigned char data)
{
	int rdata = data;
	
	Wire.beginTransmission(devAddr);
	Wire.write((int)((memAddr >> 8) & 0x00FF)); // MSB
	Wire.write((int)(memAddr & 0x00FF)); // LSB
	Wire.write(rdata);
	Wire.endTransmission();
	
	// wait write completion
	waitComplete(devAddr, TIMEOUT_WAIT);
}

// WARNING: address is a page address, 6-bit end will wrap around
// also, data can be maximum of about 30 bytes, because the Wire library has a buffer of 32 bytes
//
// rename writePage to writeBuffer
//   writeBuffer can write buffer greater than Wire library buffer
//   but must be not exceed page size
void EEPROMDriver::writeBuffer(unsigned char devAddr, unsigned short memAddr, unsigned char * buf, short bufLen)
{
	short remainLen = bufLen;
	short bufStart = 0, writeLen;
	unsigned short writeTo = memAddr;
	
	while (remainLen > 0)
	{
		Wire.beginTransmission(devAddr);
		Wire.write((int)((writeTo >> 8) & 0x00FF)); // MSB
		Wire.write((int)(writeTo & 0x00FF)); // LSB
		
		//Serial.print("W: ");
		//Serial.print(writeTo, HEX);
		//Serial.print(" ");
		
		for (writeLen = 0; writeLen < MAX_WRITE_BLOCK_SIZE && writeLen < remainLen; writeLen++)
		{
			//Serial.print(buf[bufStart+writeLen], HEX);
			//Serial.print(" ");

			Wire.write(buf[bufStart+writeLen]);
		}
		//Serial.println("");
		
		Wire.endTransmission();
		
		//
		bufStart += writeLen;
		writeTo += writeLen;		
		remainLen -= writeLen;
	
		// wait write completion
		waitComplete(devAddr, TIMEOUT_WAIT);
	}
}

unsigned char EEPROMDriver::readByte(unsigned char devAddr, unsigned short memAddr)
{
    unsigned char rdata = 0xFF;
	
    Wire.beginTransmission(devAddr);
    Wire.write((int)(memAddr >> 8)); // MSB
    Wire.write((int)(memAddr & 0xFF)); // LSB
    Wire.endTransmission();	
	
    Wire.requestFrom(devAddr, (uint8_t)1);
    if (Wire.available()) 
		rdata = Wire.read();
	
    return rdata;	
}

void EEPROMDriver::readBuffer(unsigned char devAddr, unsigned short memAddr, unsigned char * buf, short bufLen)
{
 	short remainLen = bufLen;
	short bufStart = 0, readLen;
	unsigned short readFrom = memAddr;
	
	while (remainLen > 0)
	{
		Wire.beginTransmission(devAddr);
		Wire.write((int)(readFrom >> 8)); // MSB
		Wire.write((int)(readFrom & 0xFF)); // LSB
		Wire.endTransmission();
		
		//Serial.print("R: ");
		//Serial.print(readFrom, HEX);
		//Serial.print(" ");	
		
		readLen = remainLen < MAX_READ_BLOCK_SIZE ? remainLen : MAX_READ_BLOCK_SIZE;
		
		Wire.requestFrom(devAddr, (uint8_t)readLen);
		//delay(1);
		for (short c = 0; c < readLen; c++ )
		{
			/*if (Wire.available())*/ buf[bufStart+c] = Wire.read();

			//Serial.print(buf[bufStart+c], HEX);
			//Serial.print(" ");
		}
		//Serial.println("");
		
		bufStart += readLen;
		readFrom += readLen;	
		remainLen -= readLen;
		
		//
		waitComplete(devAddr, TIMEOUT_WAIT);
	}
}
