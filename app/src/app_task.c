#include "app_task.h"

#include "app_event.h"
#include "app_key_demo.h"
#include "app_led_demo.h"
#include "app_service.h"
#include "log.h"

void app_task_start(void)
{
    log_info("boot", "starting application tasks");
    app_event_start();
    app_event_publish_system(APP_SYSTEM_EVENT_APP_STARTING);
    app_service_start();
    app_led_demo_start();
    app_key_demo_start();
    app_event_publish_system(APP_SYSTEM_EVENT_APP_STARTED);
}
