#include "stm32f0xx.h"
#include "sd_card.h"
#include "uart.h"
#include "gameflow.h"
#include "tft_display.h"
#include "pwm.h"

extern void nano_wait(unsigned int n);
extern void internal_clock();

extern int ships_placed = 0;
int isPlayerOne = 0;
int ship_count = 5;
uint8_t coords = 0xFF;
uint8_t game_state = 1;
int hit = 0;
uint8_t response = 0;
uint8_t col = 0;
uint8_t game_data[100];

void game_flow() {
    internal_clock();
    init_spi2_sd_stm32();
    sd_card_init_sequance();

    init_tim1();
    start_tim1();
    init_spi2_sd_stm32();
    enable_ports();
    setup_grid();
    setup_tim7();

    
    for(int i = 0; i < 100; i++) {
        game_data[i] = 0x00;
    }


    game_state = 1;
    while(1) {
        switch (game_state) {

        // place ships
        case 1:
            hit = 0;
            color_state(1);
            while( (ships_placed < ship_count) && (game_state == 1) );
            if(game_state == 5) {
                isPlayerOne = ((game_data[0] & 0xF0) == 0xF0);
            }
            color_state(2);
            GPIOA -> ODR |= GPIO_ODR_6;
            TIM7->DIER &= ~TIM_DIER_UIE;
            if(game_state == 1) {
                wipe_board();
            }
            while( (GPIOA -> IDR & GPIO_IDR_7) == 0);
            if(isPlayerOne) {
                game_data[0] |= 0xF0;
                game_state = 2;
                response = 0xFF;
                color_state(2);
            } else {
                game_data[0] &= ~0xF0;
                game_state = 3;
                color_state(3);
            }
            GPIOA -> ODR |= GPIO_ODR_6;
            nano_wait(1000000);
            init_uart();
            while( (GPIOA -> IDR & GPIO_IDR_7) == 1);
            break;

        case 2:
            TIM7->DIER |= TIM_DIER_UIE;
            while(response == 0xFF);
            if(response == 3) {
                game_state = 4;
                color_state(2);
            } else {
                hit = response;
                game_state = 3;
                color_state(3);
            }

            game_data[0] &= ~0xF0;

            TIM7->DIER &= ~TIM_DIER_UIE;

            write_game_data(game_data);

            break;

        case 3:
            TIM7->DIER &= TIM_DIER_UIE;
            response = waiting(game_data, ships_placed);
            if(response == 1) {
                ships_placed -= 1;
                game_state = 2;
                color_state(2);
            } else if (response == 3) {
                game_state = 4;
                color_state(4);
            } else {
                game_state = 2;
                color_state(2);
            }

            game_data[0] |= 0xF0;

            TIM7->DIER &= ~TIM_DIER_UIE;

            write_game_data(game_data);

            response = 0xFF;

            TIM7->DIER |= TIM_DIER_UIE;
            break;
        case 4:
            
            while(1);
            break;
        }

    }
}