// #include <stdbool.h>
// #include <stdint.h>
// // start code for game. Using some gameflow functions. start game, save game state
// // add functions to update tft dispay,
// // Add functions to have the waiting in between turns
// // Add function that checks the 

// #include <stdbool.h>
// #include <stdint.h>

// // --- Constants ---
// #define BOARD_SIZE 10
// #define NUM_SHIPS 2

// // --- Game Data Structure ---
// typedef struct {
//     uint8_t game_data[100];  
//     uint8_t ships_remaining;
//     bool is_player_one_turn;
//     bool game_initialized;
// } GameState;

// // --- SD Card Functions --- // will include them, here for testing
// uint8_t sd_card_init_sequence(bool isPlayerOne);
// uint8_t write_game_data(uint8_t data[100], bool isPlayerOne);
// uint8_t read_game_data(uint8_t data[100], bool isPlayerOne);

// //gamestate
// GameState current_game;

// //helper function to convert
// uint8_t coord_to_index(uint8_t x, uint8_t y) {
//     return y * BOARD_SIZE + x;
// }

// // core game funtions:
// bool is_valid_placement(uint8_t x, uint8_t y, uint8_t length, bool is_horizontal) {
//     if (x >= BOARD_SIZE || y >= BOARD_SIZE) return false;
    
//     if (is_horizontal) {
//         if (x + length > BOARD_SIZE) return false;
//         for (uint8_t i = x; i < x + length; i++) {
//             uint8_t idx = coord_to_index(i, y);
//             if (current_game.game_data[idx] & 0x04) return false;  // Check ship present bit
//         }
//     } else {
//         if (y + length > BOARD_SIZE) return false;
//         for (uint8_t i = y; i < y + length; i++) {
//             uint8_t idx = coord_to_index(x, i);
//             if (current_game.game_data[idx] & 0x04) return false;
//         }
//     }
//     return true;
// }

// void place_ship(uint8_t x, uint8_t y, bool is_horizontal, uint8_t length) {
//     if (is_horizontal) {
//         for (uint8_t i = x; i < x + length; i++) {
//             uint8_t idx = coord_to_index(i, y);
//             current_game.game_data[idx] |= 0x04;  // Set ship present bit
//         }
//     } else {
//         for (uint8_t i = y; i < y + length; i++) {
//             uint8_t idx = coord_to_index(x, i);
//             current_game.game_data[idx] |= 0x04;
//         }
//     }
// }

// uint8_t process_attack(uint8_t x, uint8_t y, bool is_attacker_player_one) {
//     uint8_t idx = coord_to_index(x, y);
    
//     // Check if already attacked
//     if (current_game.game_data[idx] & 0x01) return 0;  // Already shot here
    
//     // Mark as attacked
//     current_game.game_data[idx] |= 0x01;
    
//     // Check for hit
//     if (current_game.game_data[idx] & 0x04) {
//         current_game.game_data[idx] |= 0x02;  // Set hit bit
//         current_game.game_data[idx] |= 0x08;  // Set ship hit bit
//         return 1;  // Hit
//     }
//     return 0;  // Miss
// }

// // --- Game Flow Functions ---
// void initialize_new_game(bool is_player_one) {
//     // Clear game data
//     for (int i = 0; i < 100; i++) {
//         current_game.game_data[i] = 0;
//     }
    
//     // Example ship placement (in real game, get from player input)
//     if (is_player_one) {
//         place_ship(2, 3, true, 5);  // Carrier (length 5)
//         place_ship(7, 5, false, 4); // Battleship (length 4)
//     } else {
//         place_ship(1, 1, true, 5);
//         place_ship(6, 4, false, 4);
//     }
    
//     current_game.ships_remaining = NUM_SHIPS;
//     current_game.is_player_one_turn = true;
//     current_game.game_initialized = true;
    
//     // Save initial state
//     write_game_data(current_game.game_data, is_player_one);
// }

// // --- Main Game Loop ---
// int main() {
//     // Initialize hardware and SD card
//     bool is_player_one = determine_player_role();  // Implement based on your hardware
//     if (sd_card_init_sequence(is_player_one) != 1) {
//         return 1;  // SD card init failed
//     }
    
//     // Game initialization
//     if (is_player_one) {
//         initialize_new_game(true);
//     } else {
//         // Wait for player 1's board
//         while (read_game_data(current_game.game_data, true) != 1) {
//             delay_ms(1000);  // Poll SD card
//         }
//         initialize_new_game(false);
//     }
    
//     // Main game loop
//     while (1) {
//         if ((current_game.is_player_one_turn && is_player_one) || 
//             (!current_game.is_player_one_turn && !is_player_one)) {
            
//             // Player's turn
//             uint8_t x, y;
//             get_player_input(&x, &y);  // Get coordinates from keypad
            
//             uint8_t attack_result = process_attack(x, y, is_player_one);
//             if (attack_result != 0) {
//                 // Update display
//                 if (attack_result == 1) printf("Hit!\n");
                
//                 // Save game state
//                 write_game_data(current_game.game_data, is_player_one);
                
//                 // Switch turns
//                 current_game.is_player_one_turn = !current_game.is_player_one_turn;
//             }
//         } else {
//             // Opponent's turn - check SD card for updates
//             uint8_t temp_data[100];
//             if (read_game_data(temp_data, !is_player_one) == 1) {
//                 // Check for changes
//                 for (int i = 0; i < 100; i++) {
//                     if ((temp_data[i] & 0x01) && !(current_game.game_data[i] & 0x01)) {
//                         // Update our board
//                         current_game.game_data[i] = temp_data[i];
//                         update_display(i % 10, i / 10, current_game.game_data[i]);
                        
//                         // Switch turns
//                         current_game.is_player_one_turn = is_player_one;
//                         break;
//                     }
//                 }
//             }
//             delay_ms(500);  // Polling delay, will have the waiting function 
//         }
        
//         // Check for game over -> Will have a variable called ship_segments that decreases every hit
//         if (current_game.ships_remaining == 0) {
//             printf("Game over! %s wins!\n", is_player_one ? "Player 2" : "Player 1");
//             break;
//         }
//     }
    
//     return 0;
// }