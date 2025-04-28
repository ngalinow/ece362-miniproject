
#include "gameflow.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>
#include <ctype.h>
#include <string.h>


void initializePlayer(Player *player) 
{
    initializeBoard(player->board);
    initializeBoard(player->attackBoard);
    
    // Initialize ships with unique symbols and correct lengths
    player->ships[0] = (Ship){CARRIER, 0, false, 'A'};    // Aircraft Carrier (3)
    player->ships[1] = (Ship){BATTLESHIP, 0, false, 'B'}; // Battleship (2)
    
    player->shipsRemaining = NUM_SHIPS;
}

// Check if ship placement is valid
bool isValidPlacement(char board[BOARD_SIZE][BOARD_SIZE], uint8_t x, uint8_t y, uint8_t length, bool isHorizontal) 
{
    // Check if starting position is valid
    if (x >= BOARD_SIZE || y >= BOARD_SIZE) {
        return false;
    }
    
    // Check if ship fits on the board
    if (isHorizontal) {
        if (x + length > BOARD_SIZE) return false;
        // Check for overlapping ships
        for (uint8_t i = x; i < x + length; i++) {
            if (board[y][i] != '~') return false;
            // Check adjacent cells (optional for game rules)
        }
    } else {
        if (y + length > BOARD_SIZE) return false;
        // Check for overlapping ships
        for (uint8_t i = y; i < y + length; i++) {
            if (board[i][x] != '~') return false;
            // Check adjacent cells (optional for game rules)
        }
    }
    return true;
}

// Place a ship on the board
bool placeShip(Player *player, uint8_t shipIndex, uint8_t x, uint8_t y, bool isHorizontal) 
{
    if (shipIndex >= NUM_SHIPS) return false;
    
    uint8_t length = player->ships[shipIndex].type;
    char symbol = player->ships[shipIndex].symbol;
    
    if (!isValidPlacement(player->board, x, y, length, isHorizontal)) {
        return false;
    }
    
    // Place the ship
    if (isHorizontal) {
        for (uint8_t i = x; i < x + length; i++) {
            player->board[y][i] = symbol;
        }
    } else {
        for (uint8_t i = y; i < y + length; i++) {
            player->board[i][x] = symbol;
        }
    }
    
    return true;
}



// Execute an attack
bool makeAttack(Player *attacker, Player *defender, uint8_t x, uint8_t y) 
{
    // Validate coordinates
    if (x >= BOARD_SIZE || y >= BOARD_SIZE) {
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
        for (uint8_t i = 0; i < NUM_SHIPS; i++) {
            if (defender->ships[i].symbol == shipSymbol) {
                defender->ships[i].hits++;
                if (defender->ships[i].hits == defender->ships[i].type) {
                    defender->ships[i].isSunk = true;
                    defender->shipsRemaining--;
                    printf("\a"); // Beep when ship is sunk
                    printf("You sunk the %s ship!\n", 
                        shipSymbol == 'A' ? "Aircraft Carrier" :
                        shipSymbol == 'B' ? "Battleship" : "Unknown Ship"
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






// Main game loop (modified to use uint8_t for coordinates)
void playGame() 
{
    Player human, player2;
    
    // Initialize random seed
    srand(time(NULL));
    
    // Initialize players
    initializePlayer(&human);
    initializePlayer(&player2);
    
    bool humanTurn = true;
    while (!isGameOver(&human) && !isGameOver(&player2)) {
        if (humanTurn) {
            // Human's turn
            printGameState(&human, &player2);
            
            int x, y;
            printf("\nEnter attack coordinates (x y): ");
            while (scanf("%d %d", &x, &y) != 2 || !makeAttack(&human, &player2, (uint8_t)x, (uint8_t)y)) {
                printf("Invalid coordinates or already attacked. Try again (x y): ");
                clearInputBuffer();
            }
            printf("You attacked (%d, %d)\n", x, y);
        } else {
            // player2's turn (simple random AI)
            uint8_t x, y;
            do {
                x = rand() % BOARD_SIZE;
                y = rand() % BOARD_SIZE;
            } while (!makeAttack(&player2, &human, x, y));
        }
        humanTurn = !humanTurn;
    }
}


