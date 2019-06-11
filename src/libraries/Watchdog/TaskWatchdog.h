// TaskWatchdog.h
//

#ifndef __TASKWATCHDOG_H__
#define __TASKWATCHDOG_H__

#include <Arduino.h>


////////////////////////////////////////////////////////////////////////////////////////
// class TaskWatchdog

class TaskWatchdog
{
public:
    static void         begin(uint32_t timeout); // timeout in seconds
    static void         end();

    static esp_err_t    add(TaskHandle_t handle); // handle for task, NULL means current task
    static esp_err_t    reset();
    static esp_err_t    remove(TaskHandle_t handle);
    static esp_err_t    status(TaskHandle_t handle);
};

#endif // __TASKWATCHDOG_H__
