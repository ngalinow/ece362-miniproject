#include <stdint.h>
#include <stdbool.h>

#ifndef _SD_CARD_H_
#define _SD_CARD_H_

// game state variables that will be loaded and saved to the SD
// this is the 10x10 grid of battle ship with each location saved as a number
// there are two players each with their own boards

// row data stores a byte per square
// x - don't care
// xxxx 0000
// LSB bits 0-3
// bit 0: 0 - did not shoot here, 1 - did shoot here
// bit 1: 0 - didn't hit anything, 1 - did hit something
// bit 2: 0 - we don't have a ship there, 1 - we have a ship there
// bit 3: 0 - our ship wasn't hit, 1 - our ship was hit
uint8_t game_data[100];

// SD card commands for easy reference
#define CMD0     (0x40+0)       /* GO_IDLE_STATE */
#define CMD1     (0x40+1)       /* SEND_OP_COND */
#define CMD8     (0x40+8)       /* SEND_IF_COND */
#define CMD9     (0x40+9)       /* SEND_CSD */
#define CMD10    (0x40+10)      /* SEND_CID */
#define CMD12    (0x40+12)      /* STOP_TRANSMISSION */
#define CMD16    (0x40+16)      /* SET_BLOCKLEN */
#define CMD17    (0x40+17)      /* READ_SINGLE_BLOCK */
#define CMD18    (0x40+18)      /* READ_MULTIPLE_BLOCK */
#define CMD23    (0x40+23)      /* SET_BLOCK_COUNT */
#define CMD24    (0x40+24)      /* WRITE_BLOCK */
#define CMD25    (0x40+25)      /* WRITE_MULTIPLE_BLOCK */
#define CMD41    (0x40+41)      /* SEND_OP_COND (ACMD) */
#define CMD55    (0x40+55)      /* APP_CMD */
#define CMD58    (0x40+58)      /* READ_OCR */

uint8_t sent_byte(uint8_t b);
void send_cmd(uint8_t cmd, uint32_t args, uint8_t crc);
uint8_t sd_card_init_sequance(bool isPlayerOne);
uint8_t write_game_data(uint8_t data[100], bool isPlayerOne);
uint8_t read_game_data(uint8_t data[100], bool isPlayerOne);

#endif