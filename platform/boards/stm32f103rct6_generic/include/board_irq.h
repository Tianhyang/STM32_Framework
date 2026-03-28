#ifndef BOARD_IRQ_H
#define BOARD_IRQ_H

void board_irq_handle_nmi(void);
void board_irq_handle_hardfault(void);
void board_irq_handle_memmanage(void);
void board_irq_handle_busfault(void);
void board_irq_handle_usagefault(void);
void board_irq_handle_debugmon(void);
void board_irq_handle_systick(void);
void board_irq_handle_usart1(void);
void board_irq_handle_dma1_channel4(void);
void board_irq_handle_exti0(void);
void board_irq_handle_exti15_10(void);

#endif
