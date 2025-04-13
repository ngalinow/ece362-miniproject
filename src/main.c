#include "stm32f0xx.h"
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include "tty.h"
#include "lcd.h"
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

void init_spi_sd() {
    RCC -> APB1ENR |= RCC_APB1ENR_SPI2EN;
    RCC -> AHBENR |= RCC_AHBENR_GPIOBEN;

    // 12 NSS
    // 13 sck
    // 14 miso
    // 15 mosi

    GPIOB -> MODER |= 0xAA << 24; // sets pins 12-15 as alternate function
    
    SPI2 -> CR1 &= ~SPI_CR1_SPE;

    SPI2 -> CR1 |= 0x7 << 3; // baud rate at slowest
    SPI2 -> CR2 |= 0x7 << 8; // sets data to 8 bits

    SPI2 -> CR1 |= SPI_CR1_MSTR;
    SPI2 -> CR2 |= SPI_CR2_SSOE;
    SPI2 -> CR2 |= SPI_CR2_NSSP;

    // will use DMA for transfers
    // SPI2 -> CR2 |= SPI_CR2_TXDMAEN;
    // SPI2 -> CR2 |= SPI_CR2_RXDMAEN;
    
    SPI2 -> CR1 |= SPI_CR1_SPE;
}

// write DMA channel setup
void init_sd_dma_TX() {
    RCC -> AHBENR |= RCC_AHBENR_DMAEN; // enables our clock
    DMA1_Channel5 -> CCR &= ~(0x1); // turn off the channel for now

    DMA1_Channel5 -> CPAR = (uint32_t) &(SPI2 -> DR); // linked our output location
    DMA1_Channel5 -> CCR |= 0x1 << 4; // sets the direction to memory->peripheral
    DMA1_Channel5 -> CCR |= 0x1 << 7; // sets the incrementation for the memory
    // DMA1_Channel5 -> CCR |= 0x1 << 5; // enables circular mode
}

// read DMA channel setup
void init_sd_dma_RX() {
    RCC -> AHBENR |= RCC_AHBENR_DMAEN;
    DMA1_Channel4 -> CCR &= ~(0x1);
}

int main() {
    enable_ports();
    setup_grid();
    setup_tim7();

}