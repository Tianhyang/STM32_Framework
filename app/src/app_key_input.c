#include "app_key_input.h"

#include "FreeRTOS.h"
#include "key.h"
#include "task.h"

#define APP_KEY_INPUT_MODE KEY_MODE_INTERRUPT

static TaskHandle_t s_app_key_input_task_handle = NULL;

static void app_key_input_irq_callback(key_id_t id)
{
    BaseType_t higher_priority_task_woken = pdFALSE;

    (void)id;

    if (s_app_key_input_task_handle == NULL) {
        return;
    }

    vTaskNotifyGiveFromISR(s_app_key_input_task_handle, &higher_priority_task_woken);
    portYIELD_FROM_ISR(higher_priority_task_woken);
}

void app_key_input_init(void)
{
    s_app_key_input_task_handle = xTaskGetCurrentTaskHandle();

    if (APP_KEY_INPUT_MODE == KEY_MODE_INTERRUPT) {
        key_register_irq_callback(app_key_input_irq_callback);
    } else {
        key_register_irq_callback(NULL);
    }

    for (key_id_t id = KEY_ID_K1; id < KEY_ID_COUNT; ++id) {
        key_init_mode(id, APP_KEY_INPUT_MODE);
    }
}

void app_key_input_wait(TickType_t timeout_ticks)
{
    if (APP_KEY_INPUT_MODE == KEY_MODE_INTERRUPT) {
        (void)ulTaskNotifyTake(pdTRUE, timeout_ticks);
    } else {
        vTaskDelay(timeout_ticks);
    }
}
