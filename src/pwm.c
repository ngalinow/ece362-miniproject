#include "stm32f091xc.h"
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
void init_tim1() {
    RCC -> AHBENR |= RCC_AHBENR_GPIOAEN;
    RCC -> APB2ENR |= RCC_APB2ENR_TIM1EN;
    GPIOA -> MODER |= 0x2A << 16;
    GPIOA -> AFR[1] |= 0x222;
    TIM1 -> BDTR |= TIM_BDTR_MOE;
    TIM1 -> PSC = 0;
    TIM1 -> ARR = 2400 - 1;
    TIM1 -> CCMR1 |= 0x6 << 4;
    TIM1 -> CCMR1 |= 0x6 << 12;
    TIM1 -> CCMR2 |= 0x6 << 4;
    TIM1 -> CCER |= 0x111;
    TIM1 -> CCR1 = 2400;
    TIM1 -> CCR2 = 2400;
    TIM1 -> CCR3 = 2400;

}

void start_tim1() {
    TIM1 -> CR1 |= TIM_CR1_CEN;
}

void stop_tim1() {
    TIM1 -> CR1 &= ~TIM_CR1_CEN;
}

// 1 - game isn't started
// 2 - player's turn
// 3 - other player's turn
// 4 - spi is waiting to initialize
void color_state(uint8_t state) {
    switch (state) {
        // game is not running
        case 1:
            TIM1 -> CCR1 = 2400;
            TIM1 -> CCR2 = 2400;
            TIM1 -> CCR3 = 1800;
            break;
        case 2:
        // player's turn, send hit when cmd is sent
            TIM1 -> CCR1 = 2400;
            TIM1 -> CCR2 = 1800;
            TIM1 -> CCR3 = 2400;
            break;
        case 3:
        // other player's turn, waiting for hit command
            TIM1 -> CCR1 = 1800;
            TIM1 -> CCR2 = 2400;
            TIM1 -> CCR3 = 2400;
            break;
        case 4:
            TIM1 -> CCR1 = 1800;
            TIM1 -> CCR2 = 2400;
            TIM1 -> CCR3 = 1800;
            break;
        default:
        // turns off led
            TIM1 -> CCR1 = 2400;
            TIM1 -> CCR2 = 2400;
            TIM1 -> CCR3 = 2400;
            break;
    }
}