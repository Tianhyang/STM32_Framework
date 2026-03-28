#ifndef KEY_H
#define KEY_H

typedef enum
{
    KEY_ID_K1 = 0,
    KEY_ID_K2,
    KEY_ID_COUNT
} key_id_t;

typedef enum
{
    KEY_STATE_RELEASED = 0,
    KEY_STATE_PRESSED
} key_state_t;

typedef enum
{
    KEY_MODE_POLLING = 0,
    KEY_MODE_INTERRUPT
} key_mode_t;

typedef enum
{
    KEY_EVENT_NONE = 0,
    KEY_EVENT_PRESSED,
    KEY_EVENT_RELEASED
} key_event_type_t;

typedef struct
{
    key_id_t id;
    key_event_type_t type;
    key_state_t state;
} key_event_t;

void key_init(key_id_t id);
void key_init_mode(key_id_t id, key_mode_t mode);
key_state_t key_read(key_id_t id);
void key_register_irq_callback(void (*callback)(key_id_t id));

#endif
