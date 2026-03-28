#include "bsp_key.h"

#include "board_config.h"
#include "stm32f1xx_hal.h"

typedef struct
{
    GPIO_TypeDef *port;
    uint16_t pin;
} bsp_key_hw_t;

static const bsp_key_hw_t s_key_hw[KEY_ID_COUNT] = {
    [KEY_ID_K1] = {BOARD_KEY_K1_PORT, BOARD_KEY_K1_PIN},
    [KEY_ID_K2] = {BOARD_KEY_K2_PORT, BOARD_KEY_K2_PIN},
};

static void (*s_key_irq_callback)(key_id_t id) = NULL;

static int bsp_key_is_valid(key_id_t id)
{
    return id < KEY_ID_COUNT;
}

static void bsp_key_gpio_clock_enable(GPIO_TypeDef *port)
{
    if (port == GPIOA) {
        __HAL_RCC_GPIOA_CLK_ENABLE();
    } else if (port == GPIOB) {
        __HAL_RCC_GPIOB_CLK_ENABLE();
    } else if (port == GPIOC) {
        __HAL_RCC_GPIOC_CLK_ENABLE();
    } else if (port == GPIOD) {
        __HAL_RCC_GPIOD_CLK_ENABLE();
    } else if (port == GPIOE) {
        __HAL_RCC_GPIOE_CLK_ENABLE();
    }
}

void key_init_mode(key_id_t id, key_mode_t mode)
{
    GPIO_InitTypeDef gpio_init = {0};

    if (!bsp_key_is_valid(id)) {
        return;
    }

    __HAL_RCC_AFIO_CLK_ENABLE();

    bsp_key_gpio_clock_enable(s_key_hw[id].port);

    gpio_init.Pin = s_key_hw[id].pin;
    gpio_init.Mode = (mode == KEY_MODE_INTERRUPT) ? GPIO_MODE_IT_RISING_FALLING : GPIO_MODE_INPUT;
    gpio_init.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(s_key_hw[id].port, &gpio_init);

    if (mode == KEY_MODE_INTERRUPT) {
        if (id == KEY_ID_K1) {
            HAL_NVIC_SetPriority(BOARD_KEY_K1_IRQn, BOARD_IRQ_PRIORITY_KEY_EXTI, 0U);
            HAL_NVIC_EnableIRQ(BOARD_KEY_K1_IRQn);
        } else if (id == KEY_ID_K2) {
            HAL_NVIC_SetPriority(BOARD_KEY_K2_IRQn, BOARD_IRQ_PRIORITY_KEY_EXTI, 0U);
            HAL_NVIC_EnableIRQ(BOARD_KEY_K2_IRQn);
        }
    }
}

void key_init(key_id_t id)
{
    key_init_mode(id, KEY_MODE_POLLING);
}

key_state_t key_read(key_id_t id)
{
    GPIO_PinState state;

    if (!bsp_key_is_valid(id)) {
        return KEY_STATE_RELEASED;
    }

    state = HAL_GPIO_ReadPin(s_key_hw[id].port, s_key_hw[id].pin);

    return (state == GPIO_PIN_SET) ? KEY_STATE_PRESSED : KEY_STATE_RELEASED;
}

void key_register_irq_callback(void (*callback)(key_id_t id))
{
    s_key_irq_callback = callback;
}

void HAL_GPIO_EXTI_Callback(uint16_t gpio_pin)
{
    if (s_key_irq_callback == NULL) {
        return;
    }

    for (key_id_t id = KEY_ID_K1; id < KEY_ID_COUNT; ++id) {
        if (s_key_hw[id].pin == gpio_pin) {
            s_key_irq_callback(id);
            break;
        }
    }
}
