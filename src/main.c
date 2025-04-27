#include "stm32f0xx.h"
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include "tty.h"
#include "lcd.h"
#include "tft_display.h"


extern void internal_clock();
extern int sd_card_init_sequance();
extern void nano_wait(unsigned int n);
extern int sd_card_init_sequance();

char keymap[16] = {
    '1', '2', '3', 'A',
    '4', '5', '6', 'B',
    '7', '8', '9', 'C',
    '*', '0', '#', 'D'
};

uint8_t col = 0;


int main() {
    internal_clock();
    init_spi_sd();

    RCC -> AHBENR |= RCC_AHBENR_GPIOCEN;

    GPIOC -> MODER |= 0x5 << 12;

    // enable_ports();
    // setup_grid();
    // setup_tim7();

    if(sd_card_init_sequance() == EXIT_SUCCESS) {
        GPIOC -> ODR |= 0x1 << 6;
        return EXIT_SUCCESS;
    } else {
        GPIOC -> ODR |= 0x1 << 7;
        return EXIT_FAILURE;
    }
}


 
