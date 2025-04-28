#include <stdint.h>

#ifndef __SPI_H__
#define __SPI_H__

uint8_t sent_byte_c(uint8_t b);
uint8_t send_hit(uint8_t coords);
uint8_t waiting(uint8_t ship_locations[100]);

#endif