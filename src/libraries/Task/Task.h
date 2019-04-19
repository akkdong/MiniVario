// Task.h
//

#ifndef __TASK_H__
#define __TASK_H__

#include <Arduino.h>


////////////////////////////////////////////////////////////////////////////////////////////////////////
// class Task

class Task
{
public:
	Task(const char * name, uint32_t stackDepth, UBaseType_t priority);

public:
//	int					begin();
//	void				end();

	bool				create();
	bool				createPinnedToCore(const BaseType_t xCoreID);
	void				destroy();

protected:
	virtual void		TaskProc() = 0;

protected:	
	static void 		_TaskProc(void * param);
	
protected:
	const char *		taskName;
	uint32_t			stackDepth;
	UBaseType_t			taskPriority;
	TaskHandle_t		taskHandle;
};

#endif // __TASK_H__
