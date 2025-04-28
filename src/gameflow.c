
#include "gameflow.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>
#include <ctype.h>
#include <string.h>
#include "sd_card.h"

extern void color_state(uint8_t state);
// 1 : blue
// 2 : green
// 3 : red
// default : turn off


#define BOARD_SIZE 10
#define NUM_SHIPS 2


typedef enum {
    STATE_PLACING_SHIPS,
    STATE_ATTACKING,
    STATE_GAME_OVER
} GameState;


typedef enum {
    CARRIER = 3,      // Length 3
    BATTLESHIP = 2,    // Length 2
    NO_SHIP = 0
} ShipType;

typedef struct {
    ShipType type;
    uint8_t hits;
    bool isSunk;
    char symbol;
} Ship;

// Game state variables
GameState game_state;
uint8_t current_ship_index = 0;
Ship ships[NUM_SHIPS] = {
    {CARRIER, 0, false, 'A'},
    {BATTLESHIP, 0, false, 'B'}

};


// Initialize/load  game data
uint8_t game_data[100]; 


// Convert coordinates to array index
uint8_t coord_to_index(uint8_t x, uint8_t y) {
    return y * BOARD_SIZE + x;
}



// Check if ship placement is valid
bool is_valid_placement(uint8_t x, uint8_t y, uint8_t length, bool is_horizontal) {
    if (x >= BOARD_SIZE || y >= BOARD_SIZE) return false;
    
    if (is_horizontal) {
        if (x + length > BOARD_SIZE) return false;
        for (uint8_t i = x; i < x + length; i++) {
            uint8_t idx = coord_to_index(i, y);
            if (game_data[idx] & 0x04) return false;  // Check if ship already exists
        }
    } else {
        if (y + length > BOARD_SIZE) return false;
        for (uint8_t i = y; i < y + length; i++) {
            uint8_t idx = coord_to_index(x, i);
            if (game_data[idx] & 0x04) return false;
        }
    }
    return true;
}



// Place a ship on the player's board
void place_ship(uint8_t x, uint8_t y, bool is_horizontal) {
    uint8_t length = ships[current_ship_index].type;
    
    if (is_horizontal) {
        for (uint8_t i = x; i < x + length; i++) {
            uint8_t idx = coord_to_index(i, y);
            game_data[idx] |= 0x04;  // Set "our ship" bit
        }
    } else {
        for (uint8_t i = y; i < y + length; i++) {
            uint8_t idx = coord_to_index(x, i);
            game_data[idx] |= 0x04;
        }
    }
    
    current_ship_index++;
    if (current_ship_index >= NUM_SHIPS) {
        game_state = STATE_ATTACKING;
    }
}

void process_attack(uint8_t x, uint8_t y) {
    uint8_t idx = coord_to_index(x, y);
    
    // Mark that we shot here
    game_data[idx] |= 0x01;
    
    // Check if we hit something
    bool hit = rand() % 4 == 0;  // 25% chance for demo
    
    if (hit) {
        game_data[idx] |= 0x02;  // Set hit bit
        // In real game, check if sunk here
    }
    
    // Check if game over (simplified)
    static uint8_t hits = 0;
    if (hit) hits++;
    if (hits >= 17) {  // Total ship squares (5+4+3+3+2)
        game_state = STATE_GAME_OVER;
    }
}


// void handle_keypress(uint8_t key) {
//     // Only handle digits 0-9
//     if (key > 9) return;
    
//     // Store input in buffer
//     input_buffer[input_index++] = key;
    
//     // Process based on game state
//     switch (game_state) {
//         case STATE_PLACING_SHIPS:
//             if (input_index == 3) {  // Expecting X, Y, Orientation (0=H, 1=V)
//                 uint8_t x = input_buffer[0];
//                 uint8_t y = input_buffer[1];
//                 bool is_horizontal = (input_buffer[2] == 0);
                
//                 if (is_valid_placement(x, y, ships[current_ship_index].type, is_horizontal)) {
//                     place_ship(x, y, is_horizontal);
//                 }
//                 input_index = 0;
//             }
//             break;
            
//         case STATE_ATTACKING:
//             if (input_index == 2) {  // Expecting X, Y
//                 uint8_t x = input_buffer[0];
//                 uint8_t y = input_buffer[1];
                
//                 uint8_t idx = coord_to_index(x, y);
//                 if (!(game_data[idx] & 0x01)) {  // Check if already shot here
//                     process_attack(x, y);
//                 }
//                 input_index = 0;
//             }
//             break;
            
//         case STATE_GAME_OVER:
//             // Reset game if any key pressed
//             initialize_game_data();
//             input_index = 0;
//             break;
//     }
// }




