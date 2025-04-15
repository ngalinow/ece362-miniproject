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

// NOTES FOR THE TFT DISPLAY:

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

// Bottom left corner is (0,0)
// Bottom right corner is (0, 320)
// Top left corner is (240, 0)
// Top right corner is (240, 320)

//     C0  C1  C2  C3  C4  C5  C6  C7  C8  C9  
// A0
// A1
// A2
// A3
// A4
// A5
// A6
// A7
// A8
// A9

// MAX_ROW = 240
// MAX_COL = 320
// ROW_FACTOR = 24
// COL_FACTOR = 32

// Guess == A0C0
// key == {A0C0}
// LCD_DrawLine(240, 0, 216, 32, 0xF800); --> LCD_Drawline(x, (COL_FACTOR * key[3]), x - ROW_FACTOR, (COL_FACTOR * key[3]) + COL_FACTOR)
// LCD_DrawLine(216, 0, 240, 32, 0xF800);
