#include "stm32f0xx.h"
#include "spi.h"
#include <stdlib.h>
#include <stdio.h>

#define HIT 1
#define MISS 2
#define NO_RESPONSE 0

extern void nano_wait(unsigned int n);
extern void disable_sd_card();

void SPI2_IRQHandler() {

    volatile uint32_t temp;

    if ((GPIOA -> IDR & GPIO_IDR_7) == 0) {
        temp = SPI2 -> SR;
        SPI2 -> CR1 &= ~SPI_CR1_MSTR;
        SPI2 -> CR1 |= SPI_CR1_SPE;
    } else {
        temp = SPI2 -> SR;
        SPI2 -> CR1 |= SPI_CR1_SPE;
    }
    (void)temp;
    return;
}

void enable_send() {
    GPIOA -> ODR &= ~(0x1 << 6);
    GPIOB -> ODR &= ~(0x1 << 1);
}

void disable_send() {
    GPIOA -> ODR |= 0x1 << 6;
    GPIOB -> ODR |= 0x1 << 1;
}

void enable_slaveMode() {
    SPI2 -> CR1 &= ~SPI_CR1_SPE;
    SPI2 -> CR1 &= ~SPI_CR1_MSTR;
    SPI2 -> CR1 |= SPI_CR1_SPE;
}

void disable_slaveMode() {
    while((GPIOA -> IDR & GPIO_IDR_7) == 0);
    SPI2 -> CR1 &= ~SPI_CR1_SPE;
    SPI2 -> CR1 |= SPI_CR1_MSTR;
    SPI2 -> CR1 |= SPI_CR1_SPE;
}

uint8_t reverse_bits(uint8_t value) {
    uint8_t new_value = 0;
    for(int i = 0; i < 8; i++) {
        new_value |= ((value >> i) & 1) << (7-i);
    }
    return new_value;
}

uint8_t send_byte_c(uint8_t b) {
    while((SPI2->SR & SPI_SR_TXE) == 0);
    *((volatile uint8_t*)&(SPI2->DR)) = b;
    int value = 0x0;
    while((SPI2->SR & SPI_SR_RXNE) != SPI_SR_RXNE);
    value = *(volatile uint8_t *)&(SPI2->DR);
    while((SPI2->SR & SPI_SR_BSY) == SPI_SR_BSY);
    return value;
}

void init_spi2_sd_stm32() {
    RCC -> APB1ENR |= RCC_APB1ENR_SPI2EN;
    RCC -> AHBENR |= RCC_AHBENR_GPIOBEN;
    RCC -> AHBENR |= RCC_AHBENR_GPIOAEN;
    // PB12 nss
    // PB13 sck
    // PB14 miso
    // PB15 mosi
    GPIOB -> MODER |= 0xAA << 24; // sets pins 12-15 as alternate function
    GPIOB -> MODER |= 0x5 << 2; // PB2 and PB3 set to output for CS
    GPIOA -> MODER |= 0x1 << 12;
    GPIOB -> ODR |= 0x3 << 1;
    GPIOA -> ODR |= 0x1 << 6;
    while( (GPIOA -> IDR & GPIO_IDR_7) == 0);
    SPI2 -> CR1 &= ~SPI_CR1_SPE;
    SPI2 -> CR2 |= 0x1 << 5;    
    NVIC_EnableIRQ(SPI2_IRQn);
    SPI2 -> CR1 |= SPI_CR1_MSTR; // master mode configuration
    SPI2 -> CR1 &= ~SPI_CR1_CPOL; 
    SPI2 -> CR1 &= ~SPI_CR1_CPHA;
    SPI2 -> CR1 |= 0x7 << 3; // baud rate at slowest (sclk speed set to 187.5 KHz) 
    SPI2 -> CR2 |= 0x7 << 8; // sets data to 8 bits (1 byte per transaction)
    SPI2 -> CR2 |= SPI_CR2_FRXTH; // lets us know we have our byte ready to read (one byte)
    SPI2 -> CR1 |= SPI_CR1_SPE;
}

// function that will be used to send a hit command to the other player
// returns 1 if hit, 2 if missed, 0 if something went wrong
uint8_t send_hit(uint8_t coords) {
    disable_sd_card();
    uint8_t response = 0xff;
    nano_wait(100000);
    enable_send();
    response = send_byte_c(coords);
    nano_wait(100000);
    if(response == 0xAA) {
        response = send_byte_c(0xFF);
    } else {
        return 0;
    }
    if( !(response & 0x1) && !(response & 0x2) ) {
        response = 0;
    }
    disable_send();
    return response;
}

// wait function when the controller is waiting for the other player's move
// returns 1 if ship was hit, 2 if it was missed, 0 if error
uint8_t waiting(uint8_t game_data[100]) {
    disable_sd_card();
    enable_slaveMode();
    uint8_t response = 0xaa;
    uint8_t return_value = 0;
    uint8_t location = 0;
    *((volatile uint8_t*)&(SPI2->DR)) = response;
    while((SPI2->SR & SPI_SR_RXNE) == 0);
    response = *(volatile uint8_t *)&(SPI2->DR);
    location = game_data[response - 1];
    if(location & 0x4) {
        response = 0x1;
    } else {
        response = 0x2;
    }
    *((volatile uint8_t*)&(SPI2->DR)) = response;
    while((SPI2->SR & SPI_SR_BSY) == SPI_SR_BSY);
    return_value = response;
    while((SPI2->SR & SPI_SR_RXNE) == 0);
    response = *(volatile uint8_t *)&(SPI2->DR);
    while((SPI2->SR & SPI_SR_BSY) == SPI_SR_BSY);
    disable_slaveMode();
    if(response != 0xff) {
        return_value = 0;
    }
    return return_value;
}

int test_stmComm_sendHit() {
    RCC -> AHBENR |= RCC_AHBENR_GPIOCEN;
    GPIOC -> MODER |= 0x55 << 12;

    uint8_t response = 0;
    response = send_hit((uint8_t) 32);

    if(response == 1) {
        GPIOC -> ODR |= 0x1 << 6;
    } else {
        GPIOC -> ODR |= 0x1 << 8;
    }

    nano_wait(100000);

    response = 0;
    response = send_hit((uint8_t) 28);

    if(response == 2) {
        GPIOC -> ODR |= 0x1 << 7;
    } else {
        GPIOC -> ODR |= 0x1 << 9;
    }

    return EXIT_SUCCESS;
}

int test_stmComm_waiting() {
    RCC -> AHBENR |= RCC_AHBENR_GPIOCEN;
    GPIOC -> MODER |= 0x55 << 12;

    uint8_t game_data[100];
    uint8_t response = 0;

    for(int i = 0; i < 100; i++) {
        game_data[i] = 0x0;
    }

    game_data[31] = 0x04;

    response = waiting(game_data);

    if(response == 1) {
        GPIOC -> ODR |= 0x1 << 6;
    } else {
        GPIOC -> ODR |= 0x1 << 8;
    }

    response = 0;
    response = waiting(game_data);

    if(response == 2) {
        GPIOC -> ODR |= 0x1 << 7;
    } else {
        GPIOC -> ODR |= 0x1 << 9;
    }

    return EXIT_SUCCESS;
}

void full_test(bool isPlayerOne) {
    RCC -> AHBENR |= RCC_AHBENR_GPIOCEN;
    GPIOC -> MODER |= 0x55 << 12;

    uint8_t game_data[100];
    uint8_t response = 0;

    for(int i = 0; i < 100; i++) {
        game_data[i] = 0x0;
    }

    game_data[31] = 0x04;

    if(isPlayerOne) {
        nano_wait(100000000);
        response = sd_card_init_sequance();
    } else {
        response = sd_card_init_sequance();
        nano_wait(100000000);
    }

    if(response != 1) {
        GPIOC -> ODR |= 0x1 << 9;
        return;
    }

    if(isPlayerOne) {
        nano_wait(10000000);
        response = send_hit(32);
    } else {
        response = waiting(game_data);
    }

    if (response != 1) {
        GPIOC -> ODR |= 0x1 << 8;
        return;
    }

    if(isPlayerOne) {
        nano_wait(10000000);
    } else {
        response = write_game_data(game_data);
    }

    if (response != 1) {
        GPIOC -> ODR |= 0x1 << 7;
        return;
    }

    GPIOC -> ODR |= 0x1 << 6;

    return;
}