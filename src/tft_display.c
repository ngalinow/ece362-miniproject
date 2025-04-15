#include "stm32f0xx.h"
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include "tty.h"
#include "lcd.h"
#include "tft_display.h"

#define HISTORY_SIZE 4
extern void internal_clock();
extern void nano_wait(unsigned int n);
char key_buffer[4] = {0};
extern char keymap;
extern uint8_t col;
extern char disp[9];
char* keymap_arr = &keymap;
char coords[HISTORY_SIZE];
int history_index = 0;
int history_count = 0;

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

int __io_putchar(int c) {
    if (c == '\n') {
        while (!(USART5->ISR & USART_ISR_TXE));
        USART5->TDR = '\r';
    }

    while(!(USART5->ISR & USART_ISR_TXE));
    USART5->TDR = c;
    return c;
}

void draw_grid() {
    LCD_Setup();
    LCD_DrawFillRectangle(0, 0, 240, 320, 0xFFFF);

    for(int x = 24; x < 240; x += 24){
        LCD_DrawLine(x, 0, x, 320, 0x0000);
    }

    for(int y = 32; y < 320; y += 32){
        LCD_DrawLine(0, y, 240, y, 0x0000);
    }
}

void setup_grid() {
    internal_clock();
    setbuf(stdin,0);
    setbuf(stdout,0);
    setbuf(stderr,0);
    init_lcd_spi();
    draw_grid();
}

void enable_ports() {
    RCC->AHBENR |= RCC_AHBENR_GPIOCEN;
    GPIOC->MODER &= ~0xFF00;
    GPIOC->MODER |=  0x5500;
    GPIOC->MODER &= ~0xFF;
    GPIOC->PUPDR &= ~0xFF;
    GPIOC->PUPDR |=  0xAA;
  }

  void drive_column(int c) {
    c &= 0x3;
    GPIOC->BSRR = (0xF << 4) << 16;
    GPIOC->BSRR = (1 << (c + 4));
  }

  int read_rows() {
    return GPIOC->IDR & 0xF;
  }

  char rows_to_key(int rows) {
    int row = -1;
    for(int i = 0; i < 4; i++) {
        if(rows & (1 << i)) {
            row = 3 - i;
            break;
        }
    }

    if(row == -1) return '\0';

    int col_index = col & 0x3;
    int adjusted_col = 3 - col_index;
    int offset = row * 4 + adjusted_col;
    return keymap_arr[offset];
  }

  void handle_key(char key) {
    // int row_picked;
    // int col_picked;

    switch (key) {
      case '0':
        LCD_DrawLine(240, 0, 216, 32, 0xF800);
        LCD_DrawLine(216, 0, 240, 32, 0xF800);
        break;
      case '1':
        LCD_DrawLine(216, 32, 192, 64, 0xF800);
        LCD_DrawLine(192, 32, 216, 64, 0xF800);
        break;
      case '2':
        LCD_DrawLine(192, 64, 168, 96, 0xF800);
        LCD_DrawLine(168, 64, 192, 96, 0xF800);
        break;
      case '3':
        LCD_DrawLine(168, 96, 144, 128, 0xF800);
        LCD_DrawLine(144, 96, 168, 128, 0xF800);
        break;
      case '4':
        LCD_DrawLine(144, 128, 120, 160, 0xF800);
        LCD_DrawLine(120, 128, 144, 160, 0xF800);
        break;
      case '5':
        LCD_DrawLine(120, 160, 96, 192, 0xF800);
        LCD_DrawLine(96, 160, 120, 192, 0xF800);
        break;
      case '6':
        LCD_DrawLine(96, 192, 72, 224, 0xF800);
        LCD_DrawLine(72, 192, 96, 224, 0xF800);
        break;
      case '7':
        LCD_DrawLine(72, 224, 48, 256, 0xF800);
        LCD_DrawLine(48, 224, 72, 256, 0xF800);
        break;
      case '8':
        LCD_DrawLine(48, 256, 24, 288, 0xF800);
        LCD_DrawLine(24, 256, 48, 288, 0xF800);
        break;
      case '9':
        LCD_DrawLine(24, 288, 0, 320, 0xF800);
        LCD_DrawLine(0, 288, 24, 320, 0xF800);
        break;
    }

    // Row_picked = key_buffer[1] - 48;
    // col_picked = key_buffer[3] - 48; 

  
    // LCD_DrawLine(240-(24*Row_picked), (0+32*col_picked), (240-24*(Row_picked+1)), 32*(col_picked+1), 0xF800);
    // LCD_DrawLine(240-(24*(Row_picked+1)), (0+32*(col_picked+1)), (240-24*(Row_picked)), 32*(col_picked+1), 0xF800);
  
    // if (key == 'A') {
    //   LCD_DrawFillRectangle(0, 0, 240, 320, 0xFFFF);
    // } else if (key == 'B') {
    //   LCD_DrawFillRectangle(0, 0, 240, 320, 0x0000);
    // } else if (key == 'C') {
    //   LCD_DrawFillRectangle(0, 0, 240, 320, 0x001F);
    // } else if (key == 'D') {
    //   LCD_DrawFillRectangle(0, 0, 240, 320, 0xFFE0);
    // } else if (key == '1') {
    //   LCD_DrawFillRectangle(0, 0, 240, 320, 0x07FF);
    // } else if (key == '2') {
    //   LCD_DrawFillRectangle(0, 0, 240, 320, 0xF800);
    // } else if (key == '3') {
    //   LCD_DrawFillRectangle(0, 0, 240, 320, 0xF81F);
    // } else if (key == '4') {
    //   LCD_DrawFillRectangle(0, 0, 240, 320, 0x07E0);
    // } else if (key == '5') {
    //   LCD_DrawFillRectangle(0, 0, 240, 320, 0x7FFF);
    // } else if (key == '6') {
    //   LCD_DrawFillRectangle(0, 0, 240, 320, 0xBC40);
    // } else if (key == '7') {
    //   LCD_DrawFillRectangle(0, 0, 240, 320, 0xFC07);
    // } else if (key == '8') {
    //   LCD_DrawFillRectangle(0, 0, 240, 320, 0x8430);
    // } else if (key == '9') {
    //   LCD_DrawFillRectangle(0, 0, 240, 320, 0x01CF);
    // } else if (key == '0') {
    //   LCD_DrawFillRectangle(0, 0, 240, 320, 0x7D7C);
    // } else if (key == '*') {
    //   LCD_DrawFillRectangle(0, 0, 240, 320, 0x5458);
    // } else if (key == '#') {
    //   LCD_DrawFillRectangle(0, 0, 240, 320, 0x841F);
    // }
}

  void TIM7_IRQHandler(void) {
    TIM7->SR &= ~TIM_SR_UIF;
    int rows = read_rows();
    if (rows != 0) {
      char key = rows_to_key(rows);
      handle_key(key);
    }

    col = (col + 1) & 0x7;
    drive_column(col);
  }

  void setup_tim7() {
    RCC->APB1ENR |= RCC_APB1ENR_TIM7EN;
    TIM7->PSC = 47;
    TIM7->ARR = 99;
    TIM7->DIER |= TIM_DIER_UIE;
    NVIC_EnableIRQ(TIM7_IRQn);
    TIM7->CR1 |= TIM_CR1_CEN;
  }

