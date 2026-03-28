#ifndef LED_H
#define LED_H

typedef enum
{
    LED_ID_STATUS_1 = 0,
    LED_ID_STATUS_2,
    LED_ID_COUNT
} led_id_t;

void led_init(led_id_t id);
void led_on(led_id_t id);
void led_off(led_id_t id);
void led_toggle(led_id_t id);

#endif
