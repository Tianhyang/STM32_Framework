#include "board_irq.h"

void NMI_Handler(void)
{
    board_irq_handle_nmi();
}

void HardFault_Handler(void)
{
    board_irq_handle_hardfault();
}

void MemManage_Handler(void)
{
    board_irq_handle_memmanage();
}

void BusFault_Handler(void)
{
    board_irq_handle_busfault();
}

void UsageFault_Handler(void)
{
    board_irq_handle_usagefault();
}

void DebugMon_Handler(void)
{
    board_irq_handle_debugmon();
}

void SysTick_Handler(void)
{
    board_irq_handle_systick();
}

void USART1_IRQHandler(void)
{
    board_irq_handle_usart1();
}

void DMA1_Channel4_IRQHandler(void)
{
    board_irq_handle_dma1_channel4();
}

void EXTI0_IRQHandler(void)
{
    board_irq_handle_exti0();
}

void EXTI15_10_IRQHandler(void)
{
    board_irq_handle_exti15_10();
}
