#ifndef _TFT_DISPLAY_H_
#define _TFT_DISPLAY_H_

void init_spi1_slow(void);
void sdcard_io_hig_speed(void);
void init_lcd_spi(void);
int __io_putchar(int);
void draw_grid(void);
void setup_grid(void);
void enable_ports(void);
void drive_column(int c);
int read_rows(void);
char rows_to_key(int rows);
void handle_key(char key);
void setup_tim7();
void wipe_board();

#endif


