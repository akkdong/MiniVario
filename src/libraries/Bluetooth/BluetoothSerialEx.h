// BluetoothSerialEx.h
//

#ifndef __BLUETOOTHEX_H__
#define __BLUETOOTHEX_H__

#define USE_BLUETOOTHSERIAL		0

#if USE_BLUETOOTHSERIAL

#include <Arduino.h>
#include <BluetoothSerial.h>

class BluetoothSerialEx : public BluetoothSerial
{
public:
	//
	size_t		writeEx(uint8_t);
    int 		availableForWrite(void);
};

#else // USE_BLUETOOTHSERIAL

#include <Arduino.h>
#include <Stream.h>
#include <esp_spp_api.h>

class BluetoothSerialEx : public Stream
{
public:
	BluetoothSerialEx(void);
	~BluetoothSerialEx(void);

public:
	bool 		begin(String localName=String());
	int 		available(void);
	int 		peek(void);
	bool 		hasClient(void);
	int 		read(void);
	size_t 		write(uint8_t c);
	size_t 		write(const uint8_t *buffer, size_t size);
	void 		flush();
	void 		end(void);
	
	esp_err_t 	register_callback(esp_spp_cb_t * callback);
	
	//
	size_t		writeEx(uint8_t);
    int 		availableForWrite(void);

private:
	String 		local_name;
};

#endif // USE_BLUETOOTHSERIAL

#endif // __BLUETOOTHEX_H__
