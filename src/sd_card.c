#include <stdlib.h>
#include <string.h>
#include "sd_card.h"
#include "spi.h"
#include "stm32f0xx.h"
#include <stdbool.h>

extern void nano_wait(unsigned int n);

void enable_sd_card() {
    GPIOB -> ODR &= ~GPIO_ODR_2;
}

void disable_sd_card() {
    GPIOB -> ODR |= GPIO_ODR_2;
}

// sends the data during a data block transfer
void send_data(char b) {
    while((SPI2->SR & SPI_SR_TXE) == 0);
    *((volatile uint8_t*)&(SPI2->DR)) = b;
}

// sends one byte of information and captures what the SD card sends back
uint8_t send_byte_s(uint8_t b) {
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

void send_cmd_s(uint8_t cmd, uint32_t args, uint8_t crc) {
    send_byte_s(cmd);
    send_byte_s(args >> 24);
    send_byte_s(args >> 16);
    send_byte_s(args >> 8);
    send_byte_s(args);
    send_byte_s(crc);
}

// used after a command, waits for the sd card to respond
// handles any timeouts
uint8_t wait_for_response(int length) {
    int response = 0xff;
    for(int count = 0; count < length; count++) {
        response = send_byte_s(0xff);
        if(response != 0xff) { break; }
    }
    return response;
}

// sd card initilization sequence, refer to this http://elm-chan.org/docs/mmc/mmc_e.html
// isPlayerOne tells the program who should initialize the SD card
// returns 1 if successful
// returns 0 if it fails
uint8_t sd_card_init_sequance() {

    SPI2 -> CR1 |= SPI_CR1_SPE;
    disable_sd_card(); // cs to high
    
    nano_wait(1500000000); // wait for card to be fully powered

    // 80 dummy clock cycles
    // intilizes our SD card to SPI
    for(int i = 0; i < 10; i++) {
        send_byte_s(0xff);
    }

    enable_sd_card(); // pull sd cs low
    send_cmd_s(CMD0, 0, 0x95);
    int r1 = wait_for_response(100);
    disable_sd_card();

    if (r1 == 0xff) {return 0;}

    enable_sd_card();
    send_cmd_s(CMD55, 0, 0x1);
    r1 = wait_for_response(100);
    if(r1 > 0x1) {return 0;}
    disable_sd_card();

    enable_sd_card();
    send_cmd_s(CMD8, 0x1aa, 0x87);
    r1 = wait_for_response(100);
    wait_for_response(100);
    wait_for_response(100);
    int r7_1 = wait_for_response(100) & 0x01;
    int r7_2 = wait_for_response(100) & 0xff;
    int r7 = (r7_1 << 8) + r7_2;
    disable_sd_card();
    if(r7 != 0x1AA) {return 0;}

    enable_sd_card();
    while(r1 != 0x0) {  
        send_cmd_s(CMD55, 0x00000000, 0x01);
        r1 = wait_for_response(100);
        send_cmd_s(CMD41, 0x40000000, 0x01);
        r1 = wait_for_response(100);
    }   
    disable_sd_card();

    if(r1 != 0x0) {return 0;}

    enable_sd_card();
    send_cmd_s(CMD58, 0x0, 0x1);
    r1 = wait_for_response(100);
    wait_for_response(100);
    wait_for_response(100);
    wait_for_response(100);
    wait_for_response(100);
    disable_sd_card();

    if (r1 != 0x0) {return 0;}
    return 1;
}

// takes our game data and boolean value of playerOne and writes to SD card
// returns 1 if successful
// returns 0 if failed
uint8_t write_game_data(uint8_t data[100]) {

    uint32_t address = 0x00000200;
    uint8_t r1 = 0x0;

    enable_sd_card();

    r1 = send_byte_s(0xFF);

    send_cmd_s(CMD24, address, 0xFF);
    r1 = wait_for_response(8);

    if(r1 != 0) {return 0;}
    
    send_byte_s(0xFE);

    for(int i = 0; i < 100; i++) {
        send_data(data[i]);
    }

    for(int i = 0; i < 412; i++) {
        send_data(0xFF);
    }

    send_byte_s(0x00);
    send_byte_s(0x00);

    r1 = wait_for_response(100);

    if((r1 & 0x1F) != 0x05) {return 0;}

    while(send_byte_s(0xFF) != 0xFF);

    disable_sd_card();

    while((SPI2->SR & SPI_SR_RXNE) == 1) {
        volatile uint8_t temp = *(volatile uint8_t *)&(SPI2->DR);
        (void)temp;
    }

    return 1;
}

// takes our game data and boolean value of playerOne and reads from SD card
// returns 1 if successful
// returns 0 if failed
uint8_t read_game_data(uint8_t data[100]) {

    uint32_t address = 0x00000200;

    uint8_t r1 = 0x0;

    enable_sd_card();

    send_byte_s(0xFF);

    send_cmd_s(CMD17, address, 0x0);
    r1 = wait_for_response(100);

    if(r1 != 0x0) {return 0;}

    while(wait_for_response(100) != 0xFE);

    for(int i = 0; i < 100; i++) {
        data[i] = send_byte_s(0xFF);
    }

    for(int i = 0; i < 412; i++) {
        send_byte_s(0xFF);
    }

    send_byte_s(0xFF);
    send_byte_s(0xFF);

    while(send_byte_s(0xFF) != 0xFF);

    disable_sd_card();

    while((SPI2->SR & SPI_SR_RXNE) == 1) {
        volatile uint8_t temp = *(volatile uint8_t *)&(SPI2->DR);
        (void)temp;
    }

    return 1;
}

// testing function for SD card
// red light on PC6 means success
// yellow light on PC7 means data is different
// lights on PC8 means sd initilization failed
// lights on PC9 means read or write failed
void test_SD() {
    RCC -> AHBENR |= RCC_AHBENR_GPIOCEN;
    GPIOC -> MODER |= 0x55 << 12;

    uint8_t original_data[100];

    for(int i = 0; i < 100; i++) {
        original_data[i] = i;
    }

    for(int i = 0; i < 100; i++) {
        game_data[i] = i;
    }

    uint8_t read_data[100];
    uint8_t response = 0;

    response = sd_card_init_sequance();

    if(response != 1) {
        GPIOC -> ODR |= 0x1 << 8;
        return;
    }

    response = write_game_data(game_data);

    if(response != 1) {
        GPIOC -> ODR |= 0x1 << 9;
        return;
    }

    response = read_game_data(read_data);

    if(response != 1) {
        GPIOC -> ODR |= 0x1 << 9;
        return;
    }

    response = 1;

    for(int i = 0; i < 100; i++) {
        if(original_data[i] != read_data[i]) {
            response = 0;
            break;
        }
    }

    if(response != 1) {
        GPIOC -> ODR |= 0x1 << 7;
    } else {
        GPIOC -> ODR |= 0x1 << 6;
    }

    return;
}