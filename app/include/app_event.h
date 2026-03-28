#ifndef APP_EVENT_H
#define APP_EVENT_H

#include "key.h"

typedef enum
{
    APP_SYSTEM_EVENT_NONE = 0,
    APP_SYSTEM_EVENT_APP_STARTING,
    APP_SYSTEM_EVENT_APP_STARTED,
    APP_SYSTEM_EVENT_HEARTBEAT
} app_system_event_type_t;

typedef struct
{
    app_system_event_type_t type;
} app_system_event_t;

typedef enum
{
    APP_EVENT_TYPE_NONE = 0,
    APP_EVENT_TYPE_KEY,
    APP_EVENT_TYPE_SYSTEM
} app_event_type_t;

typedef struct
{
    app_event_type_t type;
    union
    {
        key_event_t key;
        app_system_event_t system;
    } data;
} app_event_t;

void app_event_init(void);
void app_event_start(void);
void app_event_publish_key(const key_event_t *event);
void app_event_publish_system(app_system_event_type_t type);

#endif
