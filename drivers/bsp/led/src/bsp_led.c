#include "bsp_led.h"

#include "board_config.h"
#include "stm32f1xx_hal.h"

typedef struct
{
    GPIO_TypeDef *port;
    uint16_t pin;
} bsp_led_hw_t;

static const bsp_led_hw_t s_led_hw[LED_ID_COUNT] = {
    [LED_ID_STATUS_1] = {BOARD_LED_STATUS_1_GPIO_PORT, BOARD_LED_STATUS_1_GPIO_PIN},
    [LED_ID_STATUS_2] = {BOARD_LED_STATUS_2_GPIO_PORT, BOARD_LED_STATUS_2_GPIO_PIN},
};

static int bsp_led_is_valid(led_id_t id)
{
    return id < LED_ID_COUNT;
}

static void bsp_led_gpio_clock_enable(GPIO_TypeDef *port)
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

void led_init(led_id_t id)
{
    GPIO_InitTypeDef gpio_init = {0};

    if (!bsp_led_is_valid(id)) {
        return;
    }

    bsp_led_gpio_clock_enable(s_led_hw[id].port);

    gpio_init.Pin = s_led_hw[id].pin;
    gpio_init.Mode = GPIO_MODE_OUTPUT_PP;
    gpio_init.Pull = GPIO_NOPULL;
    gpio_init.Speed = GPIO_SPEED_FREQ_LOW;

    HAL_GPIO_Init(s_led_hw[id].port, &gpio_init);
    led_off(id);
}

void led_on(led_id_t id)
{
    if (!bsp_led_is_valid(id)) {
        return;
    }

    HAL_GPIO_WritePin(s_led_hw[id].port, s_led_hw[id].pin, GPIO_PIN_RESET);
}

void led_off(led_id_t id)
{
    if (!bsp_led_is_valid(id)) {
        return;
    }

    HAL_GPIO_WritePin(s_led_hw[id].port, s_led_hw[id].pin, GPIO_PIN_SET);
}

void led_toggle(led_id_t id)
{
    if (!bsp_led_is_valid(id)) {
        return;
    }

    HAL_GPIO_TogglePin(s_led_hw[id].port, s_led_hw[id].pin);
}
