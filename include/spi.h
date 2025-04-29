#include <stdint.h>
#include <stdbool.h>

#ifndef __SPI_H__
#define __SPI_H__

uint8_t waiting(uint8_t ship_locations[100]);
uint8_t send_hit(uint8_t coords);
int test_stmComm_sendHit();
int test_stmComm_waiting();
void full_test(bool isPlayerOne);

#endif