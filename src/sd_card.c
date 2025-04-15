#include <stdlib.h>
#include <string.h>
#include "sd_card.h"
#include "stm32f0xx.h"

extern void nano_wait(unsigned int n);

void enable_sd_card() {
    GPIOB -> ODR &= ~GPIO_ODR_2;
}

void disable_sd_card() {
    GPIOB -> ODR |= GPIO_ODR_2;
}

// sends one byte of information and captures what the SD card sends back
uint8_t send_byte(uint8_t b)
{
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

void send_cmd(uint8_t cmd, uint32_t args, uint8_t crc) {
    send_byte(cmd);
    send_byte(args >> 24);
    send_byte(args >> 16);
    send_byte(args >> 8);
    send_byte(args);
    send_byte(crc);
}

// used after a command, waits for the sd card to respond
// handles any timeouts
int wait_for_response(int length) {
    int response = 0xff;
    for(int count = 0; count < length; count++) {
        response = send_byte(0xff);
        if(response != 0xff) { break; }
    }
    return response;
}

// sd card initilization sequence, refer to this http://elm-chan.org/docs/mmc/mmc_e.html
int sd_card_init_sequance() {

    SPI2 -> CR1 &= ~SPI_CR1_SSI;

    disable_sd_card(); // cs to high
    
    nano_wait(1500000); // wait for card to be fully powered

    // 80 dummy clock cycles
    // intilizes our SD card to SPI
    for(int i = 0; i < 10; i++) {
        send_byte(0xff);
    }

    // SPI2 -> CR1 &= ~SPI_CR1_SPE;
    // SPI2 -> CR1 |= 0x1 << 3;
    // SPI2 -> CR1 &= ~(0x3 << 4);
    // SPI2 -> CR1 |= SPI_CR1_SPE;

    enable_sd_card(); // pull sd cs low
    send_cmd(CMD0, 0, 0x95);
    int r1 = wait_for_response(100);
    disable_sd_card();

    if (r1 == 0xff) { return EXIT_FAILURE; }

    enable_sd_card();
    send_cmd(CMD55, 0, 0x1);
    r1 = wait_for_response(100);
    if(r1 > 0x1) { return EXIT_FAILURE; }
    disable_sd_card();

    enable_sd_card();
    send_cmd(CMD8, 0x1aa, 0x87);
    r1 = wait_for_response(100);
    wait_for_response(100);
    wait_for_response(100);
    int r7_1 = wait_for_response(100) & 0x01;
    int r7_2 = wait_for_response(100) & 0xff;
    int r7 = (r7_1 << 8) + r7_2;
    disable_sd_card();
    if(r7 != 0x1AA) { return EXIT_FAILURE; }
    
    while(r1 != 0x0) {
        enable_sd_card();
        send_cmd(CMD55, 0x40000000, 0x01);
        r1 = wait_for_response(100);
        send_cmd(CMD41, 0x40000000, 0x01);
        r1 = wait_for_response(100);
        disable_sd_card();
    }   

    if(r1 != 0x0) { return EXIT_FAILURE; }

    enable_sd_card();
    send_cmd(CMD58, 0x0, 0x1);
    r1 = wait_for_response(100);
    int ccs = wait_for_response(100) && 0x1 << 6;
    disable_sd_card();

    if (r1 != 0x0) { return EXIT_FAILURE; }

    SPI2 -> CR1 |= SPI_CR1_SSI;

    return EXIT_SUCCESS;
}