void init_spi1_slow(void);
void sdcard_io_hig_speed(void);
void init_lcd_spi(void);
int __io_putchar(int);
void draw_grid(void);
void setup_grid(void);

// NOTES FOR THE TFT DISPLAY:

// drawfillrect 0 0 200 200 0f0f
// 0 0 (x and y coord based on top-left corner near purple wire)
// 200 200 (width and height of the rectangle) (240 x 320 is the TFT MAX)
// 0f0f (color)


// drawline 0 0 200 200 2
// 0 0 (x and y coord start point based on top-left corner near purple wire)
// 200 200 (end point of the line)
// 2 (thickness of the line)


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