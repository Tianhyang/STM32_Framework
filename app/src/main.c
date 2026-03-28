#include "app.h"
#include "board.h"
#include "FreeRTOS.h"
#include "task.h"

int main(void)
{
    board_init();
    app_init();
    app_start();
    vTaskStartScheduler();

    while (1) {
    }
}
