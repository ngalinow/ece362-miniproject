
#include "gameflow.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>
#include <ctype.h>
#include <string.h>








// Initialize player with empty boards and ships
 void initializePlayer(Player *player) {
    initializeBoard(player->board);
    initializeBoard(player->attackBoard);
    
    // Initialize ships with unique symbols and correct lengths
    player->ships[0] = (Ship){CARRIER, 0, false, 'A'};    // Aircraft Carrier (3)
    player->ships[1] = (Ship){BATTLESHIP, 0, false, 'B'}; // Battleship (2)
    
    player->shipsRemaining = NUM_SHIPS;
}

// Check if ship placement is valid
bool isValidPlacement(char board[BOARD_SIZE][BOARD_SIZE], int x, int y, int length, bool isHorizontal) {
    // Check if starting position is valid
    if (x < 0 || x >= BOARD_SIZE || y < 0 || y >= BOARD_SIZE) {
        return false;
    }
    
    // Check if ship fits on the board
    if (isHorizontal) {
        if (x + length > BOARD_SIZE) return false;
        // Check for overlapping ships
        for (int i = x; i < x + length; i++) {
            if (board[y][i] != '~') return false;
            // Check adjacent cells (optional for game rules)
        }
    } else {
        if (y + length > BOARD_SIZE) return false;
        // Check for overlapping ships
        for (int i = y; i < y + length; i++) {
            if (board[i][x] != '~') return false;
            // Check adjacent cells (optional for game rules)
        }
    }
    return true;
}

// Place a ship on the board
bool placeShip(Player *player, int shipIndex, int x, int y, bool isHorizontal) {
    if (shipIndex < 0 || shipIndex >= NUM_SHIPS) return false;
    
    int length = player->ships[shipIndex].type;
    char symbol = player->ships[shipIndex].symbol;
    
    if (!isValidPlacement(player->board, x, y, length, isHorizontal)) {
        return false;
    }
    
    // Place the ship
    if (isHorizontal) {
        for (int i = x; i < x + length; i++) {
            player->board[y][i] = symbol;
        }
    } else {
        for (int i = y; i < y + length; i++) {
            player->board[i][x] = symbol;
        }
    }
    
    return true;
}


// Print the board with proper formatting
void printBoard(char board[BOARD_SIZE][BOARD_SIZE], bool showShips) {
    // Print column headers
    printf("   ");
    for (int x = 0; x < BOARD_SIZE; x++) {
        printf("%2d", x);
    }
    printf("\n");
    
    // Print each row
    for (int y = 0; y < BOARD_SIZE; y++) {
        printf("%2d ", y);
        for (int x = 0; x < BOARD_SIZE; x++) {
            char c = board[y][x];
            if (!showShips && c != '~' && c != 'X' && c != 'O') {
                c = '~'; // Hide ships if not showing
            }
            printf("%2c", c);
        }
        printf("\n");
    }
}

// // Print current game state
// void printGameState(Player *player, Player *opponent) {
//     printf("\n=== YOUR FLEET === (Ships remaining: %d)\n", player->shipsRemaining);
//     printBoard(player->board, true);
    
//     printf("\n=== YOUR ATTACKS ===\n");
//     printBoard(player->attackBoard, false);
    
//     // Print sunk ships information
//     printf("\nSunk ships: ");
//     bool anySunk = false;
//     for (int i = 0; i < NUM_SHIPS; i++) {
//         if (player->ships[i].isSunk) {
//             printf("%c ", player->ships[i].symbol);
//             anySunk = true;
//         }
//     }
//     if (!anySunk) printf("None");
//     printf("\n");
// }


// Execute an attack
bool makeAttack(Player *attacker, Player *defender, int x, int y) {
    // Validate coordinates
    if (x < 0 || x >= BOARD_SIZE || y < 0 || y >= BOARD_SIZE) {
        return false;
    }
    
    // Check if already attacked this position
    if (attacker->attackBoard[y][x] != '~') {
        return false;
    }
    
    // Check if hit
    if (defender->board[y][x] != '~') {
        char shipSymbol = defender->board[y][x];
        attacker->attackBoard[y][x] = 'X'; // Mark hit
        defender->board[y][x] = 'X';       // Mark hit on defender's board
        
        // Update ship status
        for (int i = 0; i < NUM_SHIPS; i++) {
            if (defender->ships[i].symbol == shipSymbol) {
                defender->ships[i].hits++;
                if (defender->ships[i].hits == defender->ships[i].type) {
                    defender->ships[i].isSunk = true;
                    defender->shipsRemaining--;
                    printf("\a"); // Beep when ship is sunk
                    printf("You sunk the %s ship!\n", 
                        shipSymbol == 'A' ? "Aircraft Carrier" :
                        shipSymbol == 'B' ? "Battleship" 
                      );
                }
                break;
            }
        }
    } else {
        attacker->attackBoard[y][x] = 'O'; // Mark miss
        defender->board[y][x] = 'O';       // Mark miss on defender's board
    }
    return true;
}

// Check if playergame is over
bool isGameOver(Player *player) {
    return ->shipsRemaining == 0;
}

// Clear input buffer
void clearInputBuffer() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

// Main game loop
void playGame() {
    Player human, player2;
    
    // Initialize random seed
    srand(time(NULL));
    
    // Initialize players
    initializePlayer(&human);
    initializePlayer(&player2);
    
    // printf("=== BATTLESHIP GAME ===\n");
    // printf("Board size: %dx%d\n", BOARD_SIZE, BOARD_SIZE);
    // printf("Ships: A(5), B(4), C(3), S(3), D(2)\n\n");
    
    // // Place ships
    // printf("Placing your ships randomly...\n");
    // placeShips(&human);
    // printf("Placing player2's ships randomly...\n");
    // placeShips(&player2);
    
    bool humanTurn = true;
    while (!isGameOver(&human) && !isGameOver(&player2)) {
        if (humanTurn) {
            // Human's turn
            printGameState(&human, &player2);
            
            int x, y;
            printf("\nEnter attack coordinates (x y): ");
            while (scanf("%d %d", &x, &y) != 2 || !makeAttack(&human, &player2, x, y)) {
                printf("Invalid coordinates or already attacked. Try again (x y): ");
                clearInputBuffer();
            }
            printf("You attacked (%d, %d)\n", x, y);
        } else {
            // player2's turn (simple random AI)
            int x, y;
            do {
                x = rand() % BOARD_SIZE;
                y = rand() % BOARD_SIZE;
            } while (!makeAttack(&player2, &human, x, y));
            
            // printf("player2 attacked (%d, %d)\n", x, y);
        }
        humanTurn = !humanTurn;
    }
    
}
