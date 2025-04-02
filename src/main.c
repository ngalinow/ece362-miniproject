#include <stdlib.h>
#include "stm32f0xx.h"

extern void internal_clock();

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
    internal_clock();

    int x = 4;
    int y = 5;

    int z = x + y;

    return EXIT_SUCCESS;
}

void init_spi1_slow(void) {    
    RCC->APB2ENR |= RCC_APB2ENR_SPI1EN;
    RCC->AHBENR |= RCC_AHBENR_GPIOBEN;
   
    GPIOB->MODER &= ~((0b11 << (3 * 2)) | (0b11 << (4 * 2)) | (0b11 << (5 * 2)));
    GPIOB->MODER |= ((0b10 << (3 * 2)) | (0b10 << (4 * 2)) | (0b10 << (5 * 2)));;
    GPIOB->AFR[0] &= ~(GPIO_AFRL_AFSEL3 | GPIO_AFRL_AFSEL4 | GPIO_AFRL_AFSEL5);
   
    SPI1->CR1 &= ~SPI_CR1_BR;
    SPI1->CR1 |= SPI_CR1_BR_2 | SPI_CR1_BR_1 | SPI_CR1_BR_0;
    SPI1->CR1 |= SPI_CR1_MSTR;
    SPI1->CR2 |= SPI_CR2_DS_0 | SPI_CR2_DS_1 | SPI_CR2_DS_2;
    SPI1->CR1 |= SPI_CR1_SSM | SPI_CR1_SSI; 
    SPI1->CR2 |= SPI_CR2_FRXTH;
    SPI1->CR1 |= SPI_CR1_SPE;
 }
 
 
 void sdcard_io_high_speed() {
    SPI1->CR1 &= ~SPI_CR1_SPE;
    SPI1->CR1 &= ~SPI_CR1_BR;
    SPI1->CR1 |= (SPI_CR1_BR_0);
    SPI1->CR1 |= SPI_CR1_SPE;
 }
 
 
 void init_lcd_spi() {
    RCC->AHBENR |= RCC_AHBENR_GPIOBEN;
    GPIOB -> MODER |= GPIO_MODER_MODER8_0 | GPIO_MODER_MODER11_0 | GPIO_MODER_MODER14_0;
    init_spi1_slow();
    sdcard_io_high_speed();
 }
 