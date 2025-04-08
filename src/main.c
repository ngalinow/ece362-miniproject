#include <stdlib.h>
#include "stm32f0xx.h"

extern void internal_clock();
extern int sd_card_init_sequance();

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
    // SPI2 -> CR2 |= SPI_CR2_NSSP; // NSS pulse management, allows our spi to generate the nss
    SPI2 -> CR1 |= SPI_CR1_SSM; // enables software slave management (SSI bit determines NSS)
    SPI2 -> CR1 |= SPI_CR1_SSI; // controls the nss for our controller
    SPI2 -> CR2 |= SPI_CR2_FRXTH; // lets us know we have our byte ready to read

    // will use DMA for transfers
    // SPI2 -> CR2 |= SPI_CR2_TXDMAEN;
    // SPI2 -> CR2 |= SPI_CR2_RXDMAEN;
    
    SPI2 -> CR1 |= SPI_CR1_SPE;
}

// // write DMA channel setup
// void init_sd_dma_TX() {
//     RCC -> AHBENR |= RCC_AHBENR_DMAEN; // enables our clock
//     DMA1_Channel5 -> CCR &= ~(0x1); // turn off the channel for now
//     DMA1_Channel5 -> CMAR = (uint32_t) &data; // link our DMA transfer to data
//     DMA1_Channel5 -> CPAR = (uint32_t) &(SPI2 -> DR); // linked our output location
//     DMA1_Channel5 -> CCR |= 0x1 << 4; // sets the direction to memory->peripheral
//     DMA1_Channel5 -> CCR |= 0x1 << 7; // sets the incrementation for the memory
//     // DMA1_Channel5 -> CCR |= 0x1 << 5; // enables circular mode
// }

// // read DMA channel setup
// void init_sd_dma_RX() {
//     RCC -> AHBENR |= RCC_AHBENR_DMAEN;
//     DMA1_Channel4 -> CCR &= ~(0x1);
// }

int main() {
    internal_clock();
    init_spi_sd();

    if(sd_card_init_sequance() == 1) {
        return EXIT_SUCCESS;
    } else {
        return EXIT_FAILURE;
    }
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

 // NOTES FOR THE TFT DISPLAY:

 // drawfillrect 0 0 200 200 0f0f
// 0 0 (x and y coord based on top-left corner near purple wire)
// 200 200 (width and height of the rectangle) (240 x 320 is the TFT MAX)
// 0f0f (color)


// drawline 0 0 200 200 2
// 0 0 (x and y coord start point based on top-left corner near purple wire)
// 200 200 (end point of the line)
// 2 (thickness of the line)


// CODE FOR THE GRID
// drawfillrect 0 0 240 320 0f0f
// drawline 24 0 24 320 2
// drawline 48 0 48 320 2
// drawline 72 0 72 320 2
// drawline 96 0 96 320 2
// drawline 120 0 120 320 2
// drawline 144 0 144 320 2
// drawline 168 0 168 320 2
// drawline 192 0 192 320 2
// drawline 216 0 216 320 2
// drawline 0 32 240 32 2
// drawline 0 64 240 64 2
// drawline 0 96 240 96 2
// drawline 0 128 240 128 2
// drawline 0 160 240 160 2
// drawline 0 192 240 192 2
// drawline 0 224 240 224 2
// drawline 0 256 240 256 2
// drawline 0 288 240 288 2

 