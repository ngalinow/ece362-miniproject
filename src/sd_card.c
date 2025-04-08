#include <stdlib.h>
#include <string.h>
#include "sd_card.h"
#include "stm32f0xx.h"

void nano_wait(unsigned int n) {
    asm(    "        mov r0,%0\n"
            "repeat: sub r0,#83\n"
            "        bgt repeat\n" : : "r"(n) : "r0", "cc");
}

void enable_sd_card() {
    GPIOB -> ODR &= ~GPIO_ODR_2;
}

void disable_sd_card() {
    GPIOB -> ODR |= GPIO_ODR_2;
}

uint8_t send_cmd(uint8_t b)
{
    while((SPI2->SR & SPI_SR_TXE) == 0);
    *((volatile uint8_t*)&(SPI2->DR)) = b;
    int value = 0xff;
    while ((SPI2->SR & SPI_SR_RXNE) != SPI_SR_RXNE);
        value = *(volatile uint8_t *)&(SPI2->DR);
    while((SPI2->SR & SPI_SR_BSY) == SPI_SR_BSY);
    return value;
}

int sd_card_init_sequance() {
    // need to set MOSI and CS to high
    disable_sd_card(); // cs to high
    SPI2 -> CR1 &= ~SPI_CR1_SSI;
    
    nano_wait(1500000);

    // 74 dummy clock cycles
    for(int i = 0; i < 10; i++) {
        send_cmd(0xff);
    }

    enable_sd_card();
    
    // send command 0
    send_cmd(0x40);
    send_cmd(0x00); // argument byte 1
    send_cmd(0x00); // byte 2
    send_cmd(0x00); // byte 3
    send_cmd(0x00); // byte 4
    send_cmd(0x95); // valid CRC for CMD0

    int value = 0xff;
    int count = 0;
    // The card should respond to any command within 8 bytes.
    // We'll wait for 100 to be safe.
    for(; count<100; count++) {
        value = send_cmd(0xff);
        if (value != 0xff) break;
    }

    if (value == 0xff) { return EXIT_FAILURE; }

    // send cmd 8

    // check lower 12b in R7
    // 0x1AA mismatch, error, or no response -> fail

    // if matched then go next

    // send cmd ACMD41
    // loop until R1 response = 0x00
    // break loop if R1 is error, no response, or timeout -> fail

    // send cmd 58
    // CCS bit in OCR, if 1 -> block address
    // CCS bit in OCR, if 0 -> byte address (I think we would prefer this)
    // cmd 16 for block size
    return EXIT_SUCCESS;
}