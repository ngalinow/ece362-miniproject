#include "stm32f0xx.h"
#include "pwm.h"
#include "sd_card.h"

extern void nano_wait(unsigned int n);

void clear_uart_buffer() {
    while (USART5->ISR & USART_ISR_RXNE) {
      // Read the data to clear the buffer
      volatile char temp = USART5->RDR;
      (void)temp;  // Just read to clear the buffer
    }
  }

void init_uart() {
    RCC -> AHBENR |= RCC_AHBENR_GPIOAEN;
    RCC -> APB1ENR |= RCC_APB1ENR_USART2EN;
    GPIOA -> MODER |= 0xA << 4;
    GPIOA -> AFR[0] |= 0x11 << 8;

    USART2 -> CR1 &= ~USART_CR1_UE;
    USART2 -> CR1 &= ~(0x1 << 28);
    USART2 -> CR1 &= ~(0x1 << 12);
    USART2 -> CR2 &= ~(0x3 << 12);
    USART2 -> CR1 &= ~(0x1 << 10);
    USART2 -> CR1 &= ~(0x1 << 15);
    USART2 -> BRR = 20000;
    USART2 -> CR1 |= USART_CR1_TE;
    USART2 -> CR1 |= USART_CR1_RE;
    USART2 -> CR1 |= USART_CR1_UE;

    while ((USART2->ISR & (USART_ISR_TEACK | USART_ISR_REACK)) != 
       (USART_ISR_TEACK | USART_ISR_REACK));
}

// takes in game data and the number of ships remaining
// returns 1 if hit, 2 if missed, 3 if all ships are sunk
uint8_t waiting(uint8_t data[100], int ships_left) {
    clear_uart_buffer();
    volatile uint8_t received;
    volatile uint8_t location;
    volatile uint8_t send;
    while(!(USART2->ISR & USART_ISR_RXNE));
    received = USART2->RDR;
    location = data[received - 1];
    if(location & 0x4) {
        data[received - 1] |= 0x08;
        if(ships_left == 1) {
            send = 0x3;
        } else {
            send = 0x1;
        }
    } else {
        send = 0x2;
    }
    while(!(USART2->ISR & USART_ISR_TXE));
    USART2->TDR=send;
    while (!(USART2->ISR & USART_ISR_TC));
    return send;
}

// takes in the game data and coords to the location
// returns 1 if hit, 2 if missed, 3 if sunk all ships
uint8_t send_hit(uint8_t data[100], uint8_t coords) {
    clear_uart_buffer();
    while(!(USART2->ISR & USART_ISR_TXE));
    USART2->TDR=coords;
    while(!(USART2->ISR & USART_ISR_TC));
    volatile uint8_t response;
    while(!(USART2->ISR & USART_ISR_RXNE));
    response = USART2->RDR;
    data[coords - 1] |= (0x1 | ((response & 0x1) << 1));
    return response;
}

void test(int isPlayerOne) {
    RCC->AHBENR |= RCC_AHBENR_GPIOCEN;
    GPIOC->MODER &= ~(0xFF << 12);
    GPIOC->MODER |= 0x55 << 12;
    
    init_uart();

    uint8_t game_data[100];
    
    for(int i = 0; i < 100; i++) {
        game_data[i] = 0;
    }

    if(isPlayerOne) {
        game_data[23] = 0x04;
    } else {
        game_data[19] = 0x04;
    }

    uint8_t response;

    if(isPlayerOne) {
        nano_wait(100000);
        response = send_hit(game_data, 20);
    } else {
        response = waiting(game_data, 3);
    }

    if (response == 0x1) {
        GPIOC -> ODR |= GPIO_ODR_6;
    } else {
        GPIOC -> ODR |= GPIO_ODR_7;
    }

    nano_wait(100000000);

    if(isPlayerOne) {
        response = waiting(game_data, 3);
    } else {
        nano_wait(100000);
        response = send_hit(game_data, 24);
    }

    if(response == 0x1) {
        GPIOC -> ODR |= GPIO_ODR_8;
    } else {
        GPIOC -> ODR |= GPIO_ODR_9;
    }

    return;
}
