// TaskWatchdog.cpp
//

#include "TaskWatchdog.h"
#include "esp_task_wdt.h"


////////////////////////////////////////////////////////////////////////////////////////
// class TaskWatchdog

void TaskWatchdog::begin(uint32_t timeout)
{
    esp_task_wdt_init(timeout, true);
}

void TaskWatchdog::end()
{
    esp_task_wdt_deinit();
}

esp_err_t TaskWatchdog::add(TaskHandle_t handle)
{
    return esp_task_wdt_add(handle);
}

esp_err_t TaskWatchdog::reset()
{
    return esp_task_wdt_reset();
}

esp_err_t TaskWatchdog::remove(TaskHandle_t handle)
{
    return esp_task_wdt_delete(handle);
}

esp_err_t TaskWatchdog::status(TaskHandle_t handle)
{
    return esp_task_wdt_status(handle);
}
