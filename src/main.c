#include "stm32f0xx.h"
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include "tty.h"
#include "lcd.h"
#include "sd_card.h"
#include "spi.h"
#include "tft_display.h"

extern void color_state();
extern void internal_clock();
extern void init_spi2_sd_stm32();
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
    init_spi2_sd_stm32();

    int value = test_SD();

    nano_wait(2000000000);

    value = test_stmComm_sendHit();
    return value;
    // enable_ports();
    // setup_grid();
    // setup_tim7();
}


 
