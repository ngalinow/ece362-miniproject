#include "stm32f0xx.h"
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include "tty.h"
#include "lcd.h"
#include "tft_display.h"
#include "spi.h"


extern void init_spi2_sd_stm32();
extern void internal_clock();
extern void nano_wait(unsigned int n);

char keymap[16] = {
    '1', '2', '3', 'A',
    '4', '5', '6', 'B',
    '7', '8', '9', 'C',
    '*', '0', '#', 'D'
};

uint8_t col = 0;

int main() {
    internal_clock();
    RCC -> AHBENR |= RCC_AHBENR_GPIOCEN;
    GPIOC -> MODER |= 0x5 << 12;
    GPIOC -> ODR &= ~(0x3 << 6); 
    init_spi2_sd_stm32();
    uint8_t response = 0;
    response = send_hit(1);
    if(response == 0xff) {
        GPIOC -> ODR |= 0x1 << 6;
    } else {
        GPIOC -> ODR |= 0x1 << 7;
    }
    return EXIT_SUCCESS;
}


 
