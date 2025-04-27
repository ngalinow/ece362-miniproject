#include "stm32f0xx.h"
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include "tty.h"
#include "lcd.h"
#include "tft_display.h"


extern void internal_clock();
extern int sd_card_init_sequance();
extern int wait_for_response(int length);
extern void init_spi2_sd_stm32(char isPlayerOne);
int send_hit(uint8_t coords);
extern void nano_wait(unsigned int n);

char keymap[16] = {
    '1', '2', '3', 'A',
    '4', '5', '6', 'B',
    '7', '8', '9', 'C',
    '*', '0', '#', 'D'
};

uint8_t col = 0;



int main() {

    RCC -> AHBENR |= RCC_AHBENR_GPIOCEN;
    GPIOC -> MODER |= 0x5 << 12;

    internal_clock();
    
    init_spi2_sd_stm32(0);
    
    int response = 0xff;

    while(response == 0xff) {
        response = wait_for_response(100);
    }

    send_hit(1);
    
    if (response == 1) {
        GPIOC -> ODR |= 0x1 << 6;
    } else {
        GPIOC -> ODR |= 0x1 << 7;
    }
    
    return EXIT_SUCCESS;
}


 
