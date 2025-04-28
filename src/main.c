#include "stm32f0xx.h"
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include "tty.h"
#include "lcd.h"
#include "sd_card.h"
#include "tft_display.h"


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
    RCC -> AHBENR |= RCC_AHBENR_GPIOCEN;
    GPIOC -> MODER |= 0x55 << 12;

    uint8_t original_data[100];

    for(int i = 0; i < 100; i++) {
        original_data[i] = i;
    }

    for(int i = 0; i < 100; i++) {
        game_data[i] = i;
    }

    uint8_t read_data[100];

    bool isPlayerOne = true;
    uint8_t response = 0;

    response = sd_card_init_sequance(isPlayerOne);

    if(response != 1) {
        GPIOC -> ODR |= 0x1 << 8;
        return EXIT_FAILURE;
    }

    response = write_game_data(game_data, isPlayerOne);

    if(response != 1) {
        GPIOC -> ODR |= 0x1 << 9;
        return EXIT_FAILURE;
    }

    response = read_game_data(read_data, isPlayerOne);

    if(response != 1) {
        GPIOC -> ODR |= 0x1 << 9;
        return EXIT_FAILURE;
    }

    response = 1;

    for(int i = 0; i < 100; i++) {
        if(original_data[i] != read_data[i]) {
            response = 0;
            break;
        }
    }

    if(response != 1) {
        GPIOC -> ODR |= 0x1 << 7;
        return EXIT_FAILURE;
    } else {
        GPIOC -> ODR |= 0x1 << 6;
        return EXIT_SUCCESS;
    }

}


 
