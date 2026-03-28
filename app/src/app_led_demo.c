#include "app_led_demo.h"

#include "app_task_config.h"
#include "led.h"
#include "FreeRTOS.h"
#include "log.h"
#include "task.h"

static void app_led_demo_task(void *context)
{
    uint32_t heartbeat_count = 0U;

    (void)context;

    led_init(LED_ID_STATUS_1);
    led_init(LED_ID_STATUS_2);
    log_info("led", "led demo task started");

    while (1) {
        led_on(LED_ID_STATUS_1);
        led_off(LED_ID_STATUS_2);
        vTaskDelay(pdMS_TO_TICKS(250U));

        led_off(LED_ID_STATUS_1);
        led_on(LED_ID_STATUS_2);
        vTaskDelay(pdMS_TO_TICKS(250U));

        ++heartbeat_count;
        if ((heartbeat_count % 2U) == 0U) {
            log_info("led", "heartbeat");
        }
    }
}

void app_led_demo_start(void)
{
    BaseType_t status;

    status = xTaskCreate(app_led_demo_task,
                         "led_demo",
                         APP_TASK_STACK_SIZE_LED_DEMO,
                         NULL,
                         APP_TASK_PRIORITY_LED_DEMO,
                         NULL);

    configASSERT(status == pdPASS);
}
