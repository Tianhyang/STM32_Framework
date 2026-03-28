#include "app_system.h"

#include <stddef.h>

#include "log.h"

void app_system_handle_event(const app_system_event_t *event)
{
    if (event == NULL) {
        return;
    }

    switch (event->type) {
    case APP_SYSTEM_EVENT_APP_STARTING:
        log_info("system", "application starting");
        break;
    case APP_SYSTEM_EVENT_APP_STARTED:
        log_info("system", "application started");
        break;
    case APP_SYSTEM_EVENT_HEARTBEAT:
        log_info("system", "heartbeat");
        break;
    default:
        break;
    }
}
