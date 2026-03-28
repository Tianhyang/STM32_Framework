#include "bsp_uart.h"

#include <string.h>

#include "board_config.h"
#include "FreeRTOS.h"
#include "stm32f1xx_hal.h"
#include "task.h"

#define BSP_UART_TX_RING_BUFFER_SIZE 1024U
#define BSP_UART_TX_DMA_CHUNK_SIZE   128U

typedef struct
{
    USART_TypeDef *instance;
    GPIO_TypeDef *tx_port;
    uint16_t tx_pin;
    GPIO_TypeDef *rx_port;
    uint16_t rx_pin;
    uint32_t baudrate;
} bsp_uart_hw_t;

typedef struct
{
    UART_HandleTypeDef uart_handle;
    DMA_HandleTypeDef tx_dma_handle;
    uint8_t tx_ring[BSP_UART_TX_RING_BUFFER_SIZE];
    uint8_t tx_dma_buffer[BSP_UART_TX_DMA_CHUNK_SIZE];
    uint16_t tx_head;
    uint16_t tx_tail;
    uint16_t tx_count;
    uint8_t tx_dma_active;
} bsp_uart_runtime_t;

static bsp_uart_runtime_t s_uart_runtime[UART_ID_COUNT];

static const bsp_uart_hw_t s_uart_hw[UART_ID_COUNT] = {
    [UART_ID_DEBUG] = {
        .instance = USART1,
        .tx_port = GPIOA,
        .tx_pin = GPIO_PIN_9,
        .rx_port = GPIOA,
        .rx_pin = GPIO_PIN_10,
        .baudrate = BOARD_UART_DEBUG_BAUDRATE,
    },
};

static int bsp_uart_is_valid(uart_id_t id)
{
    return id < UART_ID_COUNT;
}

static uart_id_t bsp_uart_find_id_from_handle(UART_HandleTypeDef *huart)
{
    if (huart == NULL) {
        return UART_ID_COUNT;
    }

    for (uart_id_t id = UART_ID_DEBUG; id < UART_ID_COUNT; ++id) {
        if (&s_uart_runtime[id].uart_handle == huart) {
            return id;
        }
    }

    return UART_ID_COUNT;
}

static UBaseType_t bsp_uart_lock(void)
{
    if (__get_IPSR() != 0U) {
        return taskENTER_CRITICAL_FROM_ISR();
    }

    taskENTER_CRITICAL();
    return 0U;
}

static void bsp_uart_unlock(UBaseType_t state)
{
    if (__get_IPSR() != 0U) {
        taskEXIT_CRITICAL_FROM_ISR(state);
    } else {
        taskEXIT_CRITICAL();
    }
}

static size_t bsp_uart_copy_to_ring(bsp_uart_runtime_t *runtime, const uint8_t *data, size_t length)
{
    size_t free_space;
    size_t copy_length;
    size_t first_chunk;

    free_space = BSP_UART_TX_RING_BUFFER_SIZE - runtime->tx_count;
    copy_length = (length < free_space) ? length : free_space;
    if (copy_length == 0U) {
        return 0U;
    }

    first_chunk = copy_length;
    if ((size_t)(BSP_UART_TX_RING_BUFFER_SIZE - runtime->tx_head) < first_chunk) {
        first_chunk = (size_t)(BSP_UART_TX_RING_BUFFER_SIZE - runtime->tx_head);
    }

    memcpy(&runtime->tx_ring[runtime->tx_head], data, first_chunk);
    memcpy(&runtime->tx_ring[0], data + first_chunk, copy_length - first_chunk);

    runtime->tx_head = (uint16_t)((runtime->tx_head + copy_length) % BSP_UART_TX_RING_BUFFER_SIZE);
    runtime->tx_count = (uint16_t)(runtime->tx_count + copy_length);

    return copy_length;
}

static uint16_t bsp_uart_fill_dma_buffer(bsp_uart_runtime_t *runtime)
{
    uint16_t transfer_length;
    uint16_t first_chunk;

    if ((runtime->tx_dma_active != 0U) || (runtime->tx_count == 0U)) {
        return 0U;
    }

    transfer_length = runtime->tx_count;
    if (transfer_length > BSP_UART_TX_DMA_CHUNK_SIZE) {
        transfer_length = BSP_UART_TX_DMA_CHUNK_SIZE;
    }

    first_chunk = transfer_length;
    if ((uint16_t)(BSP_UART_TX_RING_BUFFER_SIZE - runtime->tx_tail) < first_chunk) {
        first_chunk = (uint16_t)(BSP_UART_TX_RING_BUFFER_SIZE - runtime->tx_tail);
    }

    memcpy(runtime->tx_dma_buffer, &runtime->tx_ring[runtime->tx_tail], first_chunk);
    memcpy(&runtime->tx_dma_buffer[first_chunk], &runtime->tx_ring[0], (size_t)(transfer_length - first_chunk));

    runtime->tx_tail = (uint16_t)((runtime->tx_tail + transfer_length) % BSP_UART_TX_RING_BUFFER_SIZE);
    runtime->tx_count = (uint16_t)(runtime->tx_count - transfer_length);
    runtime->tx_dma_active = 1U;

    return transfer_length;
}

static void bsp_uart_start_tx_locked(uart_id_t id)
{
    bsp_uart_runtime_t *runtime;
    uint16_t transfer_length;

    runtime = &s_uart_runtime[id];
    transfer_length = bsp_uart_fill_dma_buffer(runtime);
    if (transfer_length == 0U) {
        return;
    }

    configASSERT(HAL_UART_Transmit_DMA(&runtime->uart_handle,
                                       runtime->tx_dma_buffer,
                                       transfer_length) == HAL_OK);
}

void HAL_UART_MspInit(UART_HandleTypeDef *huart)
{
    GPIO_InitTypeDef gpio_init = {0};
    DMA_HandleTypeDef *tx_dma_handle = NULL;

    if (huart->Instance != BOARD_UART_DEBUG_INSTANCE) {
        return;
    }

    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_USART1_CLK_ENABLE();
    __HAL_RCC_DMA1_CLK_ENABLE();

    gpio_init.Pin = BOARD_UART_DEBUG_TX_PIN;
    gpio_init.Mode = GPIO_MODE_AF_PP;
    gpio_init.Pull = GPIO_PULLUP;
    gpio_init.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(BOARD_UART_DEBUG_TX_PORT, &gpio_init);

    gpio_init.Pin = BOARD_UART_DEBUG_RX_PIN;
    gpio_init.Mode = GPIO_MODE_INPUT;
    gpio_init.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(BOARD_UART_DEBUG_RX_PORT, &gpio_init);

    tx_dma_handle = &s_uart_runtime[UART_ID_DEBUG].tx_dma_handle;
    tx_dma_handle->Instance = BOARD_UART_DEBUG_DMA_TX_CHANNEL;
    tx_dma_handle->Init.Direction = DMA_MEMORY_TO_PERIPH;
    tx_dma_handle->Init.PeriphInc = DMA_PINC_DISABLE;
    tx_dma_handle->Init.MemInc = DMA_MINC_ENABLE;
    tx_dma_handle->Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    tx_dma_handle->Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    tx_dma_handle->Init.Mode = DMA_NORMAL;
    tx_dma_handle->Init.Priority = DMA_PRIORITY_LOW;

    configASSERT(HAL_DMA_Init(tx_dma_handle) == HAL_OK);
    __HAL_LINKDMA(huart, hdmatx, *tx_dma_handle);

    HAL_NVIC_SetPriority(BOARD_UART_DEBUG_DMA_TX_IRQn, BOARD_IRQ_PRIORITY_UART, 0U);
    HAL_NVIC_EnableIRQ(BOARD_UART_DEBUG_DMA_TX_IRQn);
    HAL_NVIC_SetPriority(USART1_IRQn, BOARD_IRQ_PRIORITY_UART, 0U);
    HAL_NVIC_EnableIRQ(USART1_IRQn);
}

void uart_init(uart_id_t id)
{
    UART_HandleTypeDef *handle;
    bsp_uart_runtime_t *runtime;

    if (!bsp_uart_is_valid(id)) {
        return;
    }

    runtime = &s_uart_runtime[id];
    memset(runtime, 0, sizeof(*runtime));

    handle = &runtime->uart_handle;
    handle->Instance = s_uart_hw[id].instance;
    handle->Init.BaudRate = s_uart_hw[id].baudrate;
    handle->Init.WordLength = UART_WORDLENGTH_8B;
    handle->Init.StopBits = UART_STOPBITS_1;
    handle->Init.Parity = UART_PARITY_NONE;
    handle->Init.Mode = UART_MODE_TX_RX;
    handle->Init.HwFlowCtl = UART_HWCONTROL_NONE;
    handle->Init.OverSampling = UART_OVERSAMPLING_16;

    configASSERT(HAL_UART_Init(handle) == HAL_OK);
}

void uart_write(uart_id_t id, const uint8_t *data, size_t length)
{
    bsp_uart_runtime_t *runtime;
    size_t total_written = 0U;

    if (!bsp_uart_is_valid(id) || data == NULL || length == 0U) {
        return;
    }

    runtime = &s_uart_runtime[id];

    while (total_written < length) {
        size_t written;
        UBaseType_t lock_state;

        lock_state = bsp_uart_lock();
        written = bsp_uart_copy_to_ring(runtime, &data[total_written], length - total_written);
        bsp_uart_start_tx_locked(id);
        bsp_uart_unlock(lock_state);

        total_written += written;
        if (total_written >= length) {
            break;
        }

        if (xTaskGetSchedulerState() == taskSCHEDULER_NOT_STARTED) {
            continue;
        }

        taskYIELD();
    }
}

void uart_write_string(uart_id_t id, const char *text)
{
    const char *cursor = text;

    if (cursor == NULL) {
        return;
    }

    while (*cursor != '\0') {
        ++cursor;
    }

    uart_write(id, (const uint8_t *)text, (size_t)(cursor - text));
}

void uart_irq_handler(uart_id_t id)
{
    if (!bsp_uart_is_valid(id)) {
        return;
    }

    HAL_UART_IRQHandler(&s_uart_runtime[id].uart_handle);
}

void uart_dma_irq_handler(uart_id_t id)
{
    bsp_uart_runtime_t *runtime;

    if (!bsp_uart_is_valid(id)) {
        return;
    }

    runtime = &s_uart_runtime[id];
    HAL_DMA_IRQHandler(&runtime->tx_dma_handle);
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
    uart_id_t id;
    UBaseType_t lock_state;

    id = bsp_uart_find_id_from_handle(huart);
    if (!bsp_uart_is_valid(id)) {
        return;
    }

    lock_state = bsp_uart_lock();
    s_uart_runtime[id].tx_dma_active = 0U;
    bsp_uart_start_tx_locked(id);
    bsp_uart_unlock(lock_state);
}

void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
    uart_id_t id;
    UBaseType_t lock_state;

    id = bsp_uart_find_id_from_handle(huart);
    if (!bsp_uart_is_valid(id)) {
        return;
    }

    lock_state = bsp_uart_lock();
    s_uart_runtime[id].tx_dma_active = 0U;
    bsp_uart_start_tx_locked(id);
    bsp_uart_unlock(lock_state);
}
