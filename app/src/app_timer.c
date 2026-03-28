#include "app_timer.h"

#include "app_event.h"
#include "FreeRTOS.h"
#include "timers.h"

#define APP_TIMER_HEARTBEAT_PERIOD_MS 5000U

static TimerHandle_t s_heartbeat_timer = NULL;

static void app_timer_heartbeat_callback(TimerHandle_t timer)
{
    (void)timer;

    app_event_publish_system(APP_SYSTEM_EVENT_HEARTBEAT);
}

void app_timer_init(void)
{
    s_heartbeat_timer = xTimerCreate("sys_hb",
                                     pdMS_TO_TICKS(APP_TIMER_HEARTBEAT_PERIOD_MS),
                                     pdTRUE,
                                     NULL,
                                     app_timer_heartbeat_callback);

    configASSERT(s_heartbeat_timer != NULL);
}

void app_timer_start(void)
{
    configASSERT(s_heartbeat_timer != NULL);
    configASSERT(xTimerStart(s_heartbeat_timer, 0U) == pdPASS);
}
