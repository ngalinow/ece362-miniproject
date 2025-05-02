#include "stm32f0xx.h"
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include "tty.h"
#include "lcd.h"
#include "sd_card.h"
#include "tft_display.h"
#include "pwm.h"
#include "gameflow.h"

extern void internal_clock();
extern void test(int isPlayerOne);
extern void init_spi2_sd_stm32();
extern void nano_wait(unsigned int n);
extern void disable_sd_card();

char keymap[16] = {
    '1', '2', '3', 'A',
    '4', '5', '6', 'B',
    '7', '8', '9', 'C',
    '*', '0', '#', 'D'
};

int main() {
    game_flow();
    return EXIT_SUCCESS;
}


 
