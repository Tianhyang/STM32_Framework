#include "app_service.h"

#include "app_event.h"
#include "app_timer.h"
#include "log.h"

void app_service_init(void)
{
    log_init();
    app_event_init();
    app_timer_init();
    log_info("boot", "services initialized");
}

void app_service_start(void)
{
    app_timer_start();
    log_info("boot", "services started");
}
