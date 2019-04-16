// Task.cpp
//

#include "Task.h"

#if CONFIG_FREERTOS_UNICORE
#define ARDUINO_RUNNING_CORE 0
#else
#define ARDUINO_RUNNING_CORE 1
#endif


////////////////////////////////////////////////////////////////////////////////////////////////////////
// class Task

Task::Task(const char * name, uint32_t depth, UBaseType_t priority) : taskName(name), stackDepth(depth), taskPriority(priority), taskHandle(NULL)
{
}

int Task::begin()
{
	if (taskHandle != NULL)
		return -1;
	
	#if 1
	xTaskCreatePinnedToCore(
		_TaskProc,  
		taskName,     // A name just for humans
		stackDepth,   // This stack size can be checked & adjusted by reading the Stack Highwater
		this,  // Parameter
		taskPriority,    // Priority, with 3 (configMAX_PRIORITIES - 1) being the highest, and 0 being the lowest.
		&taskHandle ,  
		ARDUINO_RUNNING_CORE);
	#else
	xTaskCreate(
		_TaskProc,  
		taskName,     // A name just for humans
		stackDepth,   // This stack size can be checked & adjusted by reading the Stack Highwater
		this,  // Parameter
		taskPriority,    // Priority, with 3 (configMAX_PRIORITIES - 1) being the highest, and 0 being the lowest.
		&taskHandle);
	#endif

	return 0;
}

void Task::end()
{
	vTaskDelete(taskHandle);
}

void Task::_TaskProc(void * param)
{
	((Task *)param)->TaskProc();
	
	vTaskDelete(NULL);
}
