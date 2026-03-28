#include "log.h"

#include <stdio.h>

#include "FreeRTOS.h"
#include "semphr.h"
#include "task.h"
#include "uart.h"

#define LOG_BUFFER_SIZE 128

static SemaphoreHandle_t s_log_mutex = NULL;

static void log_write_prefix(const char *tag)
{
    char prefix[32];
    BaseType_t scheduler_state;
    int length;

    scheduler_state = xTaskGetSchedulerState();
    if (scheduler_state == taskSCHEDULER_NOT_STARTED) {
        length = snprintf(prefix, sizeof(prefix), "[%s] ", tag);
    } else {
        length = snprintf(prefix,
                          sizeof(prefix),
                          "[%lu][%s] ",
                          (unsigned long)xTaskGetTickCount(),
                          tag);
    }

    if (length > 0) {
        size_t write_length = (length < (int)sizeof(prefix)) ? (size_t)length : (sizeof(prefix) - 1U);
        uart_write(UART_ID_DEBUG, (const uint8_t *)prefix, write_length);
    }
}

void log_init(void)
{
    uart_init(UART_ID_DEBUG);
    s_log_mutex = xSemaphoreCreateMutex();
    configASSERT(s_log_mutex != NULL);
}

void log_vprintf(const char *tag, const char *format, va_list args)
{
    char buffer[LOG_BUFFER_SIZE];
    int length;
    BaseType_t scheduler_state;

    if (tag == NULL || format == NULL) {
        return;
    }

    scheduler_state = xTaskGetSchedulerState();
    if ((scheduler_state != taskSCHEDULER_NOT_STARTED) && (s_log_mutex != NULL)) {
        configASSERT(xSemaphoreTake(s_log_mutex, portMAX_DELAY) == pdTRUE);
    }

    log_write_prefix(tag);
    length = vsnprintf(buffer, sizeof(buffer), format, args);
    if (length > 0) {
        size_t write_length = (length < (int)sizeof(buffer)) ? (size_t)length : (sizeof(buffer) - 1U);
        uart_write(UART_ID_DEBUG, (const uint8_t *)buffer, write_length);
    }

    uart_write_string(UART_ID_DEBUG, "\r\n");

    if ((scheduler_state != taskSCHEDULER_NOT_STARTED) && (s_log_mutex != NULL)) {
        configASSERT(xSemaphoreGive(s_log_mutex) == pdTRUE);
    }
}

void log_printf(const char *tag, const char *format, ...)
{
    va_list args;

    if (tag == NULL || format == NULL) {
        return;
    }

    va_start(args, format);
    log_vprintf(tag, format, args);
    va_end(args);
}

void log_info(const char *tag, const char *message)
{
    log_printf(tag, "%s", message);
}
