#ifndef BOARD_H
#define BOARD_H

#include <stdint.h>

void board_init(void);
uint32_t board_get_sysclk_hz(void);
void board_fault_trap(void);

#endif
