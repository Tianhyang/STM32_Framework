#include "board_irq.h"

#include "FreeRTOS.h"
#include "board.h"
#include "board_config.h"
#include "stm32f1xx_hal.h"
#include "task.h"
#include "uart.h"

extern void xPortSysTickHandler(void);

void board_irq_handle_nmi(void)
{
    board_fault_trap();
}

void board_irq_handle_hardfault(void)
{
    board_fault_trap();
}

void board_irq_handle_memmanage(void)
{
    board_fault_trap();
}

void board_irq_handle_busfault(void)
{
    board_fault_trap();
}

void board_irq_handle_usagefault(void)
{
    board_fault_trap();
}

void board_irq_handle_debugmon(void)
{
}

void board_irq_handle_systick(void)
{
    HAL_IncTick();

    if (xTaskGetSchedulerState() != taskSCHEDULER_NOT_STARTED) {
        xPortSysTickHandler();
    }
}

void board_irq_handle_usart1(void)
{
    uart_irq_handler(UART_ID_DEBUG);
}

void board_irq_handle_dma1_channel4(void)
{
    uart_dma_irq_handler(UART_ID_DEBUG);
}

void board_irq_handle_exti0(void)
{
    HAL_GPIO_EXTI_IRQHandler(BOARD_KEY_K1_PIN);
}

void board_irq_handle_exti15_10(void)
{
    HAL_GPIO_EXTI_IRQHandler(BOARD_KEY_K2_PIN);
}
