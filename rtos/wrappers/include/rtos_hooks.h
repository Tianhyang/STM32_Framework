#ifndef RTOS_HOOKS_H
#define RTOS_HOOKS_H

#include "FreeRTOS.h"
#include "task.h"

void vApplicationMallocFailedHook(void);
void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName);

#endif
