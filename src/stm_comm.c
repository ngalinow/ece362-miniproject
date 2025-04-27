#include "stm32f0xx.h"
#include "spi.h"
#include <stdlib.h>
#include <stdio.h>

#define HIT 1
#define MISS 2
#define NO_RESPONSE 0

extern void nano_wait(unsigned int n);

void enable_send() {
    GPIOB -> ODR &= ~(0x1 << 3);
}

void disable_send() {
    GPIOB -> ODR |= 0x1 << 3;
}

uint8_t send_byte_c(uint8_t b) {
    while((SPI2->SR & SPI_SR_TXE) == 0);
    *((volatile uint8_t*)&(SPI2->DR)) = b;
    int value = 0xff;
    int count = 0;
    while((SPI2->SR & SPI_SR_RXNE) != SPI_SR_RXNE && count < 100) {
        count++;
        nano_wait(100);
    }
        value = *(volatile uint8_t *)&(SPI2->DR);
    while((SPI2->SR & SPI_SR_BSY) == SPI_SR_BSY);
    return value;
}

void init_spi2_sd_stm32() {
    
    RCC -> APB1ENR |= RCC_APB1ENR_SPI2EN;
    RCC -> AHBENR |= RCC_AHBENR_GPIOBEN;

    // PB12 nss
    // PB13 sck
    // PB14 miso
    // PB15 mosi

    GPIOB -> MODER |= 0xAA << 24; // sets pins 12-15 as alternate function

    SPI2 -> CR1 &= ~SPI_CR1_SPE;

    GPIOB -> MODER |= 0x5 << 4; // PB2 and PB3 set to output for CS
    SPI2 -> CR1 |= SPI_CR1_MSTR; // master mode configuration
    SPI2 -> CR1 |= SPI_CR1_SSM; // enables software slave management (SSI bit determines NSS)
    SPI2 -> CR1 |= SPI_CR1_SSI; // controls the nss for our controller

    SPI2 -> CR1 |= 0x7 << 3; // baud rate at slowest (sclk speed set to 187.5 KHz) 
    SPI2 -> CR2 |= 0x7 << 8; // sets data to 8 bits (1 byte per transaction)
    SPI2 -> CR2 |= SPI_CR2_FRXTH; // lets us know we have our byte ready to read

    SPI2 -> CR1 |= SPI_CR1_SPE;
}

// function that will be used to send a hit command to the other player
// returns 1 if hit, 2 if missed, 0 if something went wrong
uint8_t send_hit(uint8_t coords) {

    uint8_t response = 0xff;

    enable_send();
    response = send_byte_c(coords);
    disable_send();

    return response;
}

uint8_t waiting() {

    uint8_t response = 0x0;
    while((SPI2->SR & SPI_SR_RXNE) != 1);
    response = *(volatile uint8_t *)&(SPI2->DR);
    while((SPI2->SR & SPI_SR_TXE) == 0);
    *((volatile uint8_t*)&(SPI2->DR)) = response;
    while((SPI2->SR & SPI_SR_BSY) == SPI_SR_BSY);
    return response;
}