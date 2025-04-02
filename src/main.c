#include <stdlib.h>
#include "stm32f0xx.h"

extern void internal_clock();

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
 