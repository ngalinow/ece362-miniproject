#include <stdlib.h>
#include <string.h>
#include "sd_card.h"
#include "stm32f0xx.h"

extern void nano_wait(unsigned int n);
int write_game_data(int data[10]);
int read_data(int data[10]);

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
    SPI2 -> CR2 |= SPI_CR2_FRXTH; // lets us know we have our byte ready to read
    
    SPI2 -> CR1 |= SPI_CR1_SPE;
}

void enable_sd_card() {
    GPIOB -> ODR &= ~GPIO_ODR_2;
}

void disable_sd_card() {
    GPIOB -> ODR |= GPIO_ODR_2;
}

void send_data(char b) {
    while((SPI2->SR & SPI_SR_TXE) == 0);
    *((volatile uint8_t*)&(SPI2->DR)) = b;
}

// sends one byte of information and captures what the SD card sends back
uint8_t send_byte(uint8_t b) {
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
    int r = send_byte(crc);
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

    SPI2 -> CR1 &= ~SPI_CR1_SPE;
    SPI2 -> CR1 |= 0x1 << 3;
    SPI2 -> CR1 &= ~(0x3 << 4);
    SPI2 -> CR1 |= SPI_CR1_SPE;

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
    wait_for_response(100);
    wait_for_response(100);
    wait_for_response(100);
    wait_for_response(100);
    disable_sd_card();

    if (r1 != 0x0) { return EXIT_FAILURE; }

    // enable_sd_card();
    // r1 = send_byte(0xFF);
    // send_cmd(CMD16, 512, 0);
    // r1 = wait_for_response(100);
    // disable_sd_card();

    int write_data[10] = {23, 17, 97, 46, 0, 62, 35, 10, 150, 2};
    int r_data[10] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

    r1 = write_game_data(write_data);
    if(r1 != 0x0) { return EXIT_FAILURE; }

    send_byte(0xFF);

    r1 = read_data(r_data);
    if(r1 != 0x0) { return EXIT_FAILURE;}

    SPI2 -> CR1 |= SPI_CR1_SSI;

    for(int i = 0; i < 10; i++) {
        if(r_data[i] != write_data[i]) { return EXIT_FAILURE; }
    }

    return EXIT_SUCCESS;
}

int write_game_data(int data[10]) {

    int r1 = 0x0;

    enable_sd_card();

    r1 = send_byte(0xFF);

    send_cmd(CMD24, 0x00000200, 0xFF);
    r1 = wait_for_response(8);

    if(r1 != 0) { return r1; }
    
    send_byte(0xFE);

    for(int i = 0; i < 10; i++) {
        send_data(data[i]);
    }

    for(int i = 0; i < 502; i++) {
        send_data(0xFF);
    }

    send_byte(0x0);
    send_byte(0x0);

    r1 = wait_for_response(100);

    if((r1 & 0x1F) != 0x05) { return r1; }

    while(send_byte(0xFF) != 0xFF);

    disable_sd_card();

    return 0;
}

int read_data(int data[10]) {

    int r1 = 0x0;

    enable_sd_card();

    send_byte(0xFF);

    send_cmd(CMD17, 0x00000200, 0x0);
    r1 = wait_for_response(100);

    if(r1 != 0x0) { return r1; }

    while(wait_for_response(100) != 0xFE);

    for(int i = 0; i < 10; i++) {
        data[i] = send_byte(0xFF);
    }

    for(int i = 0; i < 502; i++) {
        send_byte(0xFF);
    }

    send_byte(0xFF);
    send_byte(0xFF);

    while(send_byte(0xFF) != 0xFF);

    disable_sd_card();

    return 0;
}