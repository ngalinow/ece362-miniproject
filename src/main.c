#include "stm32f0xx.h"
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include "tty.h"
#include "lcd.h"
#include "tft_display.h"
#include "spi.h"

extern void internal_clock();
extern int test_stmComm_waiting();
extern int test_stmComm_sendHit();
extern void nano_wait(unsigned int n);

char keymap[16] = {
    '1', '2', '3', 'A',
    '4', '5', '6', 'B',
    '7', '8', '9', 'C',
    '*', '0', '#', 'D'
};

uint8_t col = 0;
uint8_t ship_locations[100];

int main() {
    internal_clock();
}


 
