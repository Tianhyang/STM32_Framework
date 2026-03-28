#include "app.h"

#include "app_service.h"
#include "app_task.h"

void app_init(void)
{
    app_service_init();
}

void app_start(void)
{
    app_task_start();
}
