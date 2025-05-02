#include "stm32f0xx.h"
#include <stdlib.h>
#include <stdio.h>
#include "pwm.h"
#include "sd_card.h"

#define HIT 1
#define MISS 2
#define NO_RESPONSE 0

void init_spi2_sd_stm32() {
    RCC -> APB1ENR |= RCC_APB1ENR_SPI2EN;
    RCC -> AHBENR |= RCC_AHBENR_GPIOBEN;
    RCC -> AHBENR |= RCC_AHBENR_GPIOAEN;
    // PB12 nss
    // PB13 sck
    // PB14 miso
    // PB15 mosi
    GPIOB -> MODER |= 0xAA << 24; // sets pins 12-15 as alternate function
    GPIOB -> MODER |= 0x1 << 4; // PB2 set to output for CS
    GPIOA -> MODER |= 0x1 << 12;
    GPIOA -> MODER &= ~(0x3 << 14);
    GPIOB -> ODR |= 0x3 << 1;
    GPIOA -> ODR |= 0x1 << 6;
    color_state(4);
    while( (GPIOA -> IDR & GPIO_IDR_7) == 0);
    SPI2 -> CR1 &= ~SPI_CR1_SPE;
    SPI2 -> CR1 |= SPI_CR1_MSTR; // master mode configuration
    SPI2 -> CR1 &= ~SPI_CR1_CPOL; 
    SPI2 -> CR1 &= ~SPI_CR1_CPHA;
    SPI2 -> CR1 |= 0x7 << 3; // baud rate at slowest (sclk speed set to 187.5 KHz) 
    SPI2 -> CR2 |= 0x7 << 8; // sets data to 8 bits (1 byte per transaction)
    SPI2 -> CR2 |= SPI_CR2_FRXTH; // lets us know we have our byte ready to read (one byte)
    SPI2 -> CR1 |= SPI_CR1_SPE;
    color_state(1);
}

// function that will be used to send a hit command to the other player
// returns 1 if hit, 2 if missed, 0 if something went wrong
uint8_t send_hit(uint8_t coords, uint8_t data[100]) {
    SPI2 -> CR1 |= SPI_CR1_SPE;
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
    nano_wait(1000000);
    data[coords - 1] |= (0x1 | ((response & 0x1) << 1));
    while( (GPIOA -> IDR & GPIO_IDR_7) == 1);
    write_game_data(data);
    return response;
}

// wait function when the controller is waiting for the other player's move
// returns 1 if ship was hit, 2 if it was missed, 0 if error
uint8_t waiting(uint8_t data[100]) {
    disable_sd_card();
    enable_slaveMode();
    volatile uint32_t temp = SPI2 -> SR;
    (void)temp;
    SPI2 -> CR1 |= SPI_CR1_SPE;
    uint8_t response = 0xaa;
    uint8_t return_value = 0;
    uint8_t location = 0;
    color_state(3);
    *((volatile uint8_t*)&(SPI2->DR)) = response;
    while((SPI2->SR & SPI_SR_RXNE) == 0);
    response = *(volatile uint8_t *)&(SPI2->DR);
    location = data[response - 1];
    if(location & 0x4) {
        response = 0x1;
        data[response - 1] |= 0x08;
    } else {
        response = 0x2;
    }
    color_state(2);
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
    GPIOA -> ODR |= GPIO_ODR_6;
    write_game_data(data);
    GPIOA -> ODR &= ~GPIO_ODR_6;
    color_state(1);
    return return_value;
}