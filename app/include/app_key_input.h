#ifndef APP_KEY_INPUT_H
#define APP_KEY_INPUT_H

#include "FreeRTOS.h"

void app_key_input_init(void);
void app_key_input_wait(TickType_t timeout_ticks);

#endif
