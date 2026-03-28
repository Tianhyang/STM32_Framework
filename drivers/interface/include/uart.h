#ifndef UART_H
#define UART_H

#include <stddef.h>
#include <stdint.h>

typedef enum
{
    UART_ID_DEBUG = 0,
    UART_ID_COUNT
} uart_id_t;

void uart_init(uart_id_t id);
void uart_write(uart_id_t id, const uint8_t *data, size_t length);
void uart_write_string(uart_id_t id, const char *text);
void uart_irq_handler(uart_id_t id);
void uart_dma_irq_handler(uart_id_t id);

#endif
