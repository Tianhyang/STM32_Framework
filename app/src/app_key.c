#include "app_key.h"

#include <stddef.h>

#include "log.h"

static const char *app_key_name(key_id_t id)
{
    switch (id) {
    case KEY_ID_K1:
        return "K1";
    case KEY_ID_K2:
        return "K2";
    default:
        return "UNKNOWN";
    }
}

void app_key_handle_event(const key_event_t *event)
{
    const char *action;

    if (event == NULL || event->type == KEY_EVENT_NONE) {
        return;
    }

    action = (event->type == KEY_EVENT_PRESSED) ? "pressed" : "released";
    log_printf("key", "%s %s", app_key_name(event->id), action);
}
