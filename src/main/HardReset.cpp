// HardReset.cpp
//

#include "HardReset.h"

#include <esp_int_wdt.h>
#include <esp_task_wdt.h>



///////////////////////////////////////////////////////////////////////////////////////////////////////
//

void HardReset() 
{
	esp_task_wdt_init(1,true);
	esp_task_wdt_add(NULL);
	
	while(true);
}