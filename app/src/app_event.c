#include "app_event.h"

#include "app_key.h"
#include "app_system.h"
#include "app_task_config.h"
#include "FreeRTOS.h"
#include "log.h"
#include "queue.h"
#include "task.h"

#define APP_EVENT_QUEUE_LENGTH 16U

static QueueHandle_t s_app_event_queue = NULL;

static void app_event_task(void *context)
{
    app_event_t event;

    (void)context;

    log_info("event", "event task started");

    while (1) {
        if (xQueueReceive(s_app_event_queue, &event, portMAX_DELAY) != pdTRUE) {
            continue;
        }

        switch (event.type) {
        case APP_EVENT_TYPE_KEY:
            app_key_handle_event(&event.data.key);
            break;
        case APP_EVENT_TYPE_SYSTEM:
            app_system_handle_event(&event.data.system);
            break;
        case APP_EVENT_TYPE_NONE:
        default:
            break;
        }
    }
}

void app_event_init(void)
{
    s_app_event_queue = xQueueCreate(APP_EVENT_QUEUE_LENGTH, sizeof(app_event_t));
    configASSERT(s_app_event_queue != NULL);
}

void app_event_start(void)
{
    BaseType_t status;

    configASSERT(s_app_event_queue != NULL);

    status = xTaskCreate(app_event_task,
                         "app_event",
                         APP_TASK_STACK_SIZE_EVENT,
                         NULL,
                         APP_TASK_PRIORITY_EVENT,
                         NULL);

    configASSERT(status == pdPASS);
}

void app_event_publish_key(const key_event_t *event)
{
    app_event_t app_event;

    if (event == NULL || s_app_event_queue == NULL) {
        return;
    }

    app_event.type = APP_EVENT_TYPE_KEY;
    app_event.data.key = *event;

    configASSERT(xQueueSend(s_app_event_queue, &app_event, portMAX_DELAY) == pdTRUE);
}

void app_event_publish_system(app_system_event_type_t type)
{
    app_event_t app_event;

    if (s_app_event_queue == NULL) {
        return;
    }

    app_event.type = APP_EVENT_TYPE_SYSTEM;
    app_event.data.system.type = type;

    configASSERT(xQueueSend(s_app_event_queue, &app_event, portMAX_DELAY) == pdTRUE);
}
