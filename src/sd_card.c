#include <stdlib.h>
#include <string.h>
#include "sd_card.h"
#include "stm32f0xx.h"

void enable_sd_card() {
    GPIOB -> ODR &= ~(0x1 << 2);
}

void disable_sd_card() {
    GPIOB -> ODR |= ~(0x1 << 2);
}

int sd_card_init_sequance() {
    // need to set MOSI and CS to high
    disable_sd_card(); // cs to high
    
    // nano_wait(1000000);

    // 74 dummy clock cycles
    for(int i = 0; i < 10; i++) {
        send_cmd(0xff);
    }
    
    // send command 0
    send_cmd(CMD0);

    // check R1 response
    // error or no response -> fail
    if(r1_response != 0x01) {
        return EXIT_FAILURE;
    }

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

void send_cmd(unsigned int cmd) {
    while((SPI2 -> SR & SPI_SR_TXE) == 0);
    SPI2 -> DR = cmd;
}

uint8_t r1_response() {
   while((SPI2 -> SR & SPI_SR_RXNE) == 0);
   uint8_t data = SPI2 -> DR;
   return data; 
}
