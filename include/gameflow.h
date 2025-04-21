
#ifndef GAMEFLOW_H
#define GAMEFLOW_H

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>
#include <ctype.h>

#define BOARD_SIZE 10
#define NUM_SHIPS 5

// Ship types with their lengths
typedef enum {
    CARRIER = 5,
    BATTLESHIP = 4,
    CRUISER = 3,
    SUBMARINE = 3,
    DESTROYER = 2
} ShipType;

// Ship structure
typedef struct {
    ShipType type;
    int hits;
    bool isSunk;
    char symbol;
} Ship;

// Player structure
typedef struct {
    char board[BOARD_SIZE][BOARD_SIZE];      // Own ships
    char attackBoard[BOARD_SIZE][BOARD_SIZE]; // Tracking opponent
    Ship ships[NUM_SHIPS];
    int shipsRemaining;
} Player;

// Function prototypes
void initializeBoard(char board[BOARD_SIZE][BOARD_SIZE]);
void initializePlayer(Player *player);
void placeShips(Player *player);
bool placeShip(Player *player, int shipIndex, int x, int y, bool isHorizontal);
void printBoard(char board[BOARD_SIZE][BOARD_SIZE], bool showShips);
void printGameState(Player *player, Player *opponent);
bool isValidPlacement(char board[BOARD_SIZE][BOARD_SIZE], int x, int y, int length, bool isHorizontal);
bool makeAttack(Player *attacker, Player *defender, int x, int y);
bool isGameOver(Player *player);
void playGame();

#endif 

