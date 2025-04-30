#include <stdint.h>
#ifndef __PWM_H__
#define __PWM_H__

void init_tim1();
void start_tim1();
void stop_tim1();
void color_state(uint8_t state);

#endif