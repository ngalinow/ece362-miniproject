#include "stm32f0xx.h"
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <math.h>
#include "tty.h"
#include "lcd.h"
#include "tft_display.h"
#include "sd_card.h"
#include "gameflow.h"
#include "uart.h"

int rowx = -1;          // variable used to calculate the row (first number pressed on keypad)
int coly = -1;          // variable used to calculate the col (second number pressed on keypad)
int keypad_counter = 0; // counts how many keys have been pressed (resets back to 0 after two keys are pressed)
           // 1 = X's are enabled (hit), 0 = O's are enabled (miss)
extern int ships_placed;

char keypad_map[4][4] = {
    {'1', '2', '3', 'A'},
    {'4', '5', '6', 'B'},
    {'7', '8', '9', 'C'},
    {'*', '0', '#', 'D'}};

extern void internal_clock();
extern void nano_wait(unsigned int n);
extern char keymap;
extern uint8_t col;
char *keymap_arr = &keymap;

void init_spi1_slow(void) {
  RCC->APB2ENR |= RCC_APB2ENR_SPI1EN;
  RCC->AHBENR |= RCC_AHBENR_GPIOBEN;
  GPIOB->MODER &= ~((0b11 << (3 * 2)) | (0b11 << (4 * 2)) | (0b11 << (5 * 2)));
  GPIOB->MODER |= ((0b10 << (3 * 2)) | (0b10 << (4 * 2)) | (0b10 << (5 * 2)));
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
  GPIOB->MODER |= GPIO_MODER_MODER8_0 | GPIO_MODER_MODER11_0 | GPIO_MODER_MODER10_0;
  init_spi1_slow();
  sdcard_io_high_speed();
}

int __io_putchar(int c) {
  if (c == '\n') {
    while (!(USART5->ISR & USART_ISR_TXE));
    USART5->TDR = '\r';
  }

  while (!(USART5->ISR & USART_ISR_TXE));
  USART5->TDR = c;
  return c;
}

void wipe_board() {
  LCD_DrawFillRectangle(0, 0, 240, 320, 0xFFFF);
  draw_grid();
}

void draw_grid() {
  LCD_Setup();
  LCD_DrawFillRectangle(0, 0, 240, 320, 0xFFFF);

  for (int x = 24; x < 240; x += 24) {
    LCD_DrawLine(x, 0, x, 320, 0x0000);
  }

  for (int y = 32; y < 320; y += 32) {
    LCD_DrawLine(0, y, 240, y, 0x0000);
  }
}

// void load_shots(uint8_t game_data) {
//   for(int looper = 0; looper < 100; looper++) {
//     int shoot = game_data[(int*)looper] & 1;        // LSB checks if shot
//     int hit1 = (game_data[(int*)looper] >> 1) & 1;  // 2nd LSB checks if hit

//     int coly1 = looper % 10;          // column
//     int rowx1 = floor(looper / 10);   // row

//     if (shoot == 1) {
//       if (hit1 == 1) {
//         LCD_DrawLine(240 -(rowx1 * 24), 32 * coly1, 216 - (rowx1 * 24), 32 * (coly1 + 1), 0xF800);
//         LCD_DrawLine(216 -(rowx1 * 24), 32 * coly1, 240 - (rowx1 * 24), 32 * (coly1 + 1), 0xF800);
//       } else if(hit1 == 0) {
//         LCD_Circle(228 - (rowx1 * 24), 32 * coly1 + 16, 10, 1, 0X7D7C);
//       }
//     }
//   }
// }

void setup_grid() {
  internal_clock();
  setbuf(stdin, 0);
  setbuf(stdout, 0);
  setbuf(stderr, 0);
  init_lcd_spi();
  draw_grid();
  // load_shots(game_data);
}

void enable_ports() {
  RCC->AHBENR |= RCC_AHBENR_GPIOCEN;
  GPIOC->MODER &= ~0xFFF00;
  GPIOC->MODER |= 0x15500;
  GPIOC->MODER &= ~0xFF;
  GPIOC->PUPDR &= ~0xFF;
  GPIOC->PUPDR |= 0xAA;
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
  for (int i = 0; i < 4; i++) {
    if (rows & (1 << i)) {
      row = 3 - i;
      break;
    }
  }

  if (row == -1) {
    return '\0';  
  }

  int col_index = col & 0x3;
  int adjusted_col = 3 - col_index;
  int offset = row * 4 + adjusted_col;
  return keymap_arr[offset];
}

void handle_key(char key) {
  switch (key) {
  case '0':
    if (rowx > -1) {
      coly = 0;
      int coords = rowx + (coly * 10) + 1;
      if(game_state != 1) {
        response = send_hit(game_data, coords);
        hit = response & 0x1;
      } else {
        hit = 0;
        ships_placed += 1;
        game_data[coords-1] |= 0x04;
      }
      if (hit > 0) {
        LCD_DrawLine(240 - (rowx * 24), 32 * coly, 216 - (rowx * 24), 32 * (coly + 1), 0xF800);
        LCD_DrawLine(216 - (rowx * 24), 32 * coly, 240 - (rowx * 24), 32 * (coly + 1), 0xF800);
      }
      else if (hit == 0) {
        LCD_Circle(228 - (rowx * 24), 32 * coly + 16, 10, 1, 0X7D7C);
      }
    }
    rowx = 0;
    nano_wait(1000000000);
    keypad_counter++;
    GPIOC->ODR |= GPIO_ODR_8;
    break;
  case '1':
    if (rowx > -1) {
      coly = 1;
      int coords = rowx + (coly * 10) + 1;
      if(game_state != 1) {
        response = send_hit(game_data, coords);
        hit = response & 0x1;
      } else {
        hit = 0;
        ships_placed += 1;
        game_data[coords-1] |= 0x04;
      }
      if (hit > 0) {
        LCD_DrawLine(240 - (rowx * 24), 32 * coly, 216 - (rowx * 24), 32 * (coly + 1), 0xF800);
        LCD_DrawLine(216 - (rowx * 24), 32 * coly, 240 - (rowx * 24), 32 * (coly + 1), 0xF800);
      }
      else if (hit == 0) {
        LCD_Circle(228 - (rowx * 24), 32 * coly + 16, 10, 1, 0X7D7C);
      }
    }
    rowx = 1;
    nano_wait(1000000000);
    keypad_counter++;
    GPIOC->ODR |= GPIO_ODR_8;
    break;
  case '2':
    if (rowx > -1) {
      coly = 2;
      int coords = rowx + (coly * 10) + 1;
      if(game_state != 1) {
        response = send_hit(game_data, coords);
        hit = response & 0x1;
      } else {
        hit = 0;
        ships_placed += 1;
        game_data[coords-1] |= 0x04;
      }
      if (hit > 0) {
        LCD_DrawLine(240 - (rowx * 24), 32 * coly, 216 - (rowx * 24), 32 * (coly + 1), 0xF800);
        LCD_DrawLine(216 - (rowx * 24), 32 * coly, 240 - (rowx * 24), 32 * (coly + 1), 0xF800);
      }
      else if (hit == 0) {
        LCD_Circle(228 - (rowx * 24), 32 * coly + 16, 10, 1, 0X7D7C);
      }
    }
    rowx = 2;
    nano_wait(1000000000);
    keypad_counter++;
    GPIOC->ODR |= GPIO_ODR_8;
    break;
  case '3':
    if (rowx > -1) {
      coly = 3;
      int coords = rowx + (coly * 10) + 1;
      if(game_state != 1) {
        response = send_hit(game_data, coords);
        hit = response & 0x1;
      } else {
        hit = 0;
        ships_placed += 1;
        game_data[coords-1] |= 0x04;
      }
      if (hit > 0) {
        LCD_DrawLine(240 - (rowx * 24), 32 * coly, 216 - (rowx * 24), 32 * (coly + 1), 0xF800);
        LCD_DrawLine(216 - (rowx * 24), 32 * coly, 240 - (rowx * 24), 32 * (coly + 1), 0xF800);
      }
      else if (hit == 0) {
        LCD_Circle(228 - (rowx * 24), 32 * coly + 16, 10, 1, 0X7D7C);
      }
    }
    rowx = 3;
    nano_wait(1000000000);
    keypad_counter++;
    GPIOC->ODR |= GPIO_ODR_8;
    break;
  case '4':
    if (rowx > -1) {
      coly = 4;
      int coords = rowx + (coly * 10) + 1;
      if(game_state != 1) {
        response = send_hit(game_data, coords);
        hit = response & 0x1;
      } else {
        hit = 0;
        ships_placed += 1;
        game_data[coords-1] |= 0x04;
      }
      if (hit > 0) {
        LCD_DrawLine(240 - (rowx * 24), 32 * coly, 216 - (rowx * 24), 32 * (coly + 1), 0xF800);
        LCD_DrawLine(216 - (rowx * 24), 32 * coly, 240 - (rowx * 24), 32 * (coly + 1), 0xF800);
      }
      else if (hit == 0) {
        LCD_Circle(228 - (rowx * 24), 32 * coly + 16, 10, 1, 0X7D7C);
      }
    }
    rowx = 4;
    nano_wait(1000000000);
    keypad_counter++;
    GPIOC->ODR |= GPIO_ODR_8;
    break;
  case '5':
    if (rowx > -1) {
      coly = 5;
      int coords = rowx + (coly * 10) + 1;
      if(game_state != 1) {
        response = send_hit(game_data, coords);
        hit = response & 0x1;
      } else {
        hit = 0;
        ships_placed += 1;
        game_data[coords-1] |= 0x04;
      }
      if (hit > 0) {
        LCD_DrawLine(240 - (rowx * 24), 32 * coly, 216 - (rowx * 24), 32 * (coly + 1), 0xF800);
        LCD_DrawLine(216 - (rowx * 24), 32 * coly, 240 - (rowx * 24), 32 * (coly + 1), 0xF800);
      }
      else if (hit == 0) {
        LCD_Circle(228 - (rowx * 24), 32 * coly + 16, 10, 1, 0X7D7C);
      }
    }
    rowx = 5;
    nano_wait(1000000000);
    keypad_counter++;
    GPIOC->ODR |= GPIO_ODR_8;
    break;
  case '6':
    if (rowx > -1) {
      coly = 6;
      int coords = rowx + (coly * 10) + 1;
      if(game_state != 1) {
        response = send_hit(game_data, coords);
        hit = response & 0x1;
      } else {
        hit = 0;
        ships_placed += 1;
        game_data[coords-1] |= 0x04;
      }
      if (hit > 0) {
        LCD_DrawLine(240 - (rowx * 24), 32 * coly, 216 - (rowx * 24), 32 * (coly + 1), 0xF800);
        LCD_DrawLine(216 - (rowx * 24), 32 * coly, 240 - (rowx * 24), 32 * (coly + 1), 0xF800);
      }
      else if (hit == 0) {
        LCD_Circle(228 - (rowx * 24), 32 * coly + 16, 10, 1, 0X7D7C);
      }
    }
    rowx = 6;
    nano_wait(1000000000);
    keypad_counter++;
    GPIOC->ODR |= GPIO_ODR_8;
    break;
  case '7':
    if (rowx > -1) {
      coly = 7;
      int coords = rowx + (coly * 10) + 1;
      if(game_state != 1) {
        response = send_hit(game_data, coords);
        hit = response & 0x1;
      } else {
        hit = 0;
        ships_placed += 1;
        game_data[coords-1] |= 0x04;
      }
      if (hit > 0) {
        LCD_DrawLine(240 - (rowx * 24), 32 * coly, 216 - (rowx * 24), 32 * (coly + 1), 0xF800);
        LCD_DrawLine(216 - (rowx * 24), 32 * coly, 240 - (rowx * 24), 32 * (coly + 1), 0xF800);
      }
      else if (hit == 0) {
        LCD_Circle(228 - (rowx * 24), 32 * coly + 16, 10, 1, 0X7D7C);
      }
    }
    rowx = 7;
    nano_wait(1000000000);
    keypad_counter++;
    GPIOC->ODR |= GPIO_ODR_8;
    break;
  case '8':
    if (rowx > -1) {
      coly = 8;
      int coords = rowx + (coly * 10) + 1;
      if(game_state != 1) {
        response = send_hit(game_data, coords);
        hit = response & 0x1;
      } else {
        hit = 0;
        ships_placed += 1;
        game_data[coords-1] |= 0x04;
      }
      if (hit > 0) {
        LCD_DrawLine(240 - (rowx * 24), 32 * coly, 216 - (rowx * 24), 32 * (coly + 1), 0xF800);
        LCD_DrawLine(216 - (rowx * 24), 32 * coly, 240 - (rowx * 24), 32 * (coly + 1), 0xF800);
      }
      else if (hit == 0) {
        LCD_Circle(228 - (rowx * 24), 32 * coly + 16, 10, 1, 0X7D7C);
      }
    }
    rowx = 8;
    nano_wait(1000000000);
    keypad_counter++;
    GPIOC->ODR |= GPIO_ODR_8;
    break;
  case '9':
    if (rowx > -1) {
      coly = 9;
      int coords = rowx + (coly * 10) + 1;
      if(game_state != 1) {
        response = send_hit(game_data, coords);
        hit = response & 0x1;
      } else {
        hit = 0;
        ships_placed += 1;
        game_data[coords-1] |= 0x04;
      }
      if (hit > 0) {
        LCD_DrawLine(240 - (rowx * 24), 32 * coly, 216 - (rowx * 24), 32 * (coly + 1), 0xF800);
        LCD_DrawLine(216 - (rowx * 24), 32 * coly, 240 - (rowx * 24), 32 * (coly + 1), 0xF800);
      }
      else if (hit == 0) {
        LCD_Circle(228 - (rowx * 24), 32 * coly + 16, 10, 1, 0X7D7C);
      }
    }
    rowx = 9;
    nano_wait(1000000000);
    keypad_counter++;
    GPIOC->ODR |= GPIO_ODR_8;
    break;
  case '#':
    // Where i = rows and j = columns
    if(game_state == 1) {
      TIM7 -> CR1 &= ~TIM_CR1_CEN;
      ships_placed = 0;
      read_game_data(game_data);
      for(int i = 0; i < 10; i++) {
        for(int j = 0; j < 10; j++) {
          int shoot = ((game_data[i + j] & 0x1) == 0x1);        // LSB checks if shot
          int hit1 = ((game_data[i + j]) & 0x2 == 0x2);  // 2nd LSB checks if hit

          int coly1 = j;          // column
          int rowx1 = i;          // row

          if (shoot == 1) {
            if (hit1 == 1) {
              LCD_DrawLine(240 - (rowx1 * 24), 32 * coly1, 216 - (rowx1 * 24), 32 * (coly1 + 1), 0xF800);
              LCD_DrawLine(216 - (rowx1 * 24), 32 * coly1, 240 - (rowx1 * 24), 32 * (coly1 + 1), 0xF800);
            } else if(hit1 == 0) {
              LCD_Circle(228 - (rowx1 * 24), 32 * coly1 + 16, 10, 1, 0X7D7C);
            }
          }

          if( (game_data[i+j] & 0xC) == 0x4) {
            ships_placed += 1;
          }
        }
      }
      game_state = 5;
    }
    break;
}

  if (keypad_counter > 1) {
    rowx = -1;
    coly = -1;
    keypad_counter = 0;
    GPIOC->ODR &= ~GPIO_ODR_8;
  }
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
