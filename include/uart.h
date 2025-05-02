#include <stdint.h>

#ifndef __UART_H__
#define __UART_H__

uint8_t send_hit(uint8_t data[100], uint8_t coords);
uint8_t waiting(uint8_t data[100], int ships_left);
void init_uart();

#endif