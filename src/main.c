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

char keymap[16] = {
    '1', '2', '3', 'A',
    '4', '5', '6', 'B',
    '7', '8', '9', 'C',
    '*', '0', '#', 'D'
};

uint8_t col = 0;

void init_spi_sd() {
    RCC -> APB1ENR |= RCC_APB1ENR_SPI2EN;
    RCC -> AHBENR |= RCC_AHBENR_GPIOBEN;

    // 12 NSS
    // 13 sck
    // 14 miso
    // 15 mosi

    GPIOB -> MODER |= 0xAA << 24; // sets pins 12-15 as alternate function
    GPIOB -> MODER |= 0x1 << 4; // PB2 set to output for turning the SD card on / off
    
    SPI2 -> CR1 &= ~SPI_CR1_SPE;

    SPI2 -> CR1 |= 0x7 << 3; // baud rate at slowest (sclk speed set to 187.5 KHz) 
    SPI2 -> CR2 |= 0x7 << 8; // sets data to 8 bits (1 byte per transaction)

    SPI2 -> CR1 |= SPI_CR1_MSTR; // master mode configuration
    SPI2 -> CR2 |= SPI_CR2_SSOE; // SS output enabled, multimaster not allowed
    SPI2 -> CR1 |= SPI_CR1_SSM; // enables software slave management (SSI bit determines NSS)
    SPI2 -> CR1 |= SPI_CR1_SSI; // controls the nss for our controller
    SPI2 -> CR2 |= SPI_CR2_FRXTH; // lets us know we have our byte ready to read
    
    SPI2 -> CR1 |= SPI_CR1_SPE;
}

int main() {
    internal_clock();
    init_spi_sd();

    // enable_ports();
    // setup_grid();
    // setup_tim7();

    if(sd_card_init_sequance() == 1) {
        return EXIT_SUCCESS;
    } else {
        return EXIT_FAILURE;
    }
}


 
