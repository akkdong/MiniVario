// Copyright 2018 Evandro Luis Copercini
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at

//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef __BLUETOOTHEX_H__
#define __BLUETOOTHEX_H__

#include "sdkconfig.h"

#if defined(CONFIG_BT_ENABLED) && defined(CONFIG_BLUEDROID_ENABLED)

#include "Arduino.h"
#include "Stream.h"
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

#endif

#endif // __BLUETOOTHEX_H__
