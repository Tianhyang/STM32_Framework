#include "app_key_demo.h"

#include "app_event.h"
#include "app_key_input.h"
#include "app_task_config.h"
#include "key.h"
#include "log.h"
#include "task.h"

#define APP_KEY_SCAN_PERIOD_MS     500U
#define APP_KEY_DEBOUNCE_TIME_MS   20U

static void app_key_demo_task(void *context)
{
    key_state_t stable_state[KEY_ID_COUNT];

    (void)context;
    app_key_input_init();

    for (key_id_t id = KEY_ID_K1; id < KEY_ID_COUNT; ++id) {
        stable_state[id] = key_read(id);
    }

    log_info("key", "key demo task started");

    while (1) {
        for (key_id_t id = KEY_ID_K1; id < KEY_ID_COUNT; ++id) {
            key_state_t current_state = key_read(id);

            if (current_state != stable_state[id]) {
                vTaskDelay(pdMS_TO_TICKS(APP_KEY_DEBOUNCE_TIME_MS));
                current_state = key_read(id);

                if (current_state == stable_state[id]) {
                    continue;
                }

                stable_state[id] = current_state;

                key_event_t event;
                event.id = id;
                event.state = current_state;
                event.type = (current_state == KEY_STATE_PRESSED) ? KEY_EVENT_PRESSED : KEY_EVENT_RELEASED;
                app_event_publish_key(&event);
            }
        }

        app_key_input_wait(pdMS_TO_TICKS(APP_KEY_SCAN_PERIOD_MS));
    }
}

void app_key_demo_start(void)
{
    BaseType_t status;

    status = xTaskCreate(app_key_demo_task,
                         "key_demo",
                         APP_TASK_STACK_SIZE_KEY_DEMO,
                         NULL,
                         APP_TASK_PRIORITY_KEY_DEMO,
                         NULL);

    configASSERT(status == pdPASS);
}
