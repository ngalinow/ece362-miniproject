
// #include "gameflow.h"
// #include <stdio.h>
// #include <stdlib.h>
// #include <time.h>
// #include <stdbool.h>
// #include <ctype.h>
// #include <string.h>
// #include "sd_card.h"

// extern void color_state(uint8_t state);
// // 1 : blue
// // 2 : green
// // 3 : red
// // default : turn off


// #define BOARD_SIZE 10
// #define NUM_SHIPS 2


// typedef enum {
//     STATE_PLACING_SHIPS,
//     STATE_ATTACKING,
//     STATE_GAME_OVER
// } GameState;


// typedef enum {
//     CARRIER = 3,      // Length 3
//     BATTLESHIP = 2,    // Length 2
//     NO_SHIP = 0
// } ShipType;

// typedef struct {
//     ShipType type;
//     uint8_t hits;
//     bool isSunk;
//     char symbol;
// } Ship;

// // Game state variables
// GameState game_state;
// uint8_t current_ship_index = 0;
// Ship ships[NUM_SHIPS] = {
//     {CARRIER, 0, false, 'A'},
//     {BATTLESHIP, 0, false, 'B'}

// };


// // Initialize/load  game data
// uint8_t game_data[100]; 


// // Convert coordinates to array index
// uint8_t coord_to_index(uint8_t x, uint8_t y) {
//     return y * BOARD_SIZE + x;
// }



// // Check if ship placement is valid
// bool is_valid_placement(uint8_t x, uint8_t y, uint8_t length, bool is_horizontal, bool is_player_one) {
//     if (x >= BOARD_SIZE || y >= BOARD_SIZE) return false;

//     // Check if ship fits on the board
//     if (is_horizontal) {
//         if (x + length > BOARD_SIZE) return false;
//         for (uint8_t i = x; i < x + length; i++) {
//             uint8_t idx = y * 10 + i;
//             if (game_data[idx] & 0x04) return false; // Check "our ship" bit
//         }
//     } else {
//         if (y + length > BOARD_SIZE) return false;
//         for (uint8_t i = y; i < y + length; i++) {
//             uint8_t idx = i * 10 + x;
//             if (game_data[idx] & 0x04) return false;
//         }
//     }
//     return true;
// }



// // Place a ship on the player's board
// void place_ship(uint8_t x, uint8_t y, bool is_horizontal) {
//     uint8_t length = ships[current_ship_index].type;
    
//     if (is_horizontal) {
//         for (uint8_t i = x; i < x + length; i++) {
//             uint8_t idx = coord_to_index(i, y);
//             game_data[idx] |= 0x04;  // Set "our ship" bit
//         }
//     } else {
//         for (uint8_t i = y; i < y + length; i++) {
//             uint8_t idx = coord_to_index(x, i);
//             game_data[idx] |= 0x04;
//         }
//     }
    
//     current_ship_index++;
//     if (current_ship_index >= NUM_SHIPS) {
//         game_state = STATE_ATTACKING;
//     }
// }

// uint8_t process_attack(uint8_t x, uint8_t y, bool is_player_one) {
//     uint8_t idx = y * 10 + x;
//     uint8_t *target_data = &game_data[idx];

//     // Check if already attacked
//     if (*target_data & 0x01) return 0; // Already shot here

//     // Mark as attacked (bit 0)
//     *target_data |= 0x01;

//     // Check for hit (bit 2 = ship present)
//     if (*target_data & 0x04) {
//         *target_data |= 0x02; // Set hit bit (bit 1)
//         *target_data |= 0x08; // Set "ship hit" bit (bit 3)

//         // Check if ship is sunk (requires tracking ship segments)
//         if (check_ship_sunk(x, y, is_player_one)) {
//             return 2; // Ship sunk
//         }
//         return 1; // Hit
//     } else {
//         return 0; // Miss
//     }
// }


