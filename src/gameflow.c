
#include "gameflow.h"

// Initialize an empty 10x10 board
void initializeBoard(char board[BOARD_SIZE][BOARD_SIZE]) {
    for (int y = 0; y < BOARD_SIZE; y++) {
        for (int x = 0; x < BOARD_SIZE; x++) {
            board[y][x] = '~'; // '~' represents water
        }
    }
}

// Initialize player with empty boards and ships
void initializePlayer(Player *player) {
    initializeBoard(player->board);
    initializeBoard(player->attackBoard);
    
    // Initialize ships with unique symbols
    player->ships[0] = (Ship){CARRIER, 0, false, 'C'};
    player->ships[1] = (Ship){BATTLESHIP, 0, false, 'B'};
    player->ships[2] = (Ship){CRUISER, 0, false, 'R'};
    player->ships[3] = (Ship){SUBMARINE, 0, false, 'S'};
    player->ships[4] = (Ship){DESTROYER, 0, false, 'D'};
    
    player->shipsRemaining = NUM_SHIPS;
}

// Check if ship placement is valid on 10x10 grid
bool isValidPlacement(char board[BOARD_SIZE][BOARD_SIZE], int x, int y, int length, bool isHorizontal) {
    // Check boundaries
    if (x < 0 || x >= BOARD_SIZE || y < 0 || y >= BOARD_SIZE) return false;
    
    if (isHorizontal) {
        if (x + length > BOARD_SIZE) return false;
        for (int i = x; i < x + length; i++) {
            if (board[y][i] != '~') return false;
        }
    } else {
        if (y + length > BOARD_SIZE) return false;
        for (int i = y; i < y + length; i++) {
            if (board[i][x] != '~') return false;
        }
    }
    return true;
}

// Place a ship on the board
bool placeShip(Player *player, int shipIndex, int x, int y, bool isHorizontal) {
    int length = player->ships[shipIndex].type;
    char symbol = player->ships[shipIndex].symbol;
    
    if (!isValidPlacement(player->board, x, y, length, isHorizontal)) {
        return false;
    }
    
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

// Automatically place all ships
void placeShips(Player *player) {
    srand(time(NULL));
    
    for (int i = 0; i < NUM_SHIPS; i++) {
        bool placed = false;
        int attempts = 0;
        
        while (!placed && attempts < 100) {
            int x = rand() % BOARD_SIZE;
            int y = rand() % BOARD_SIZE;
            bool isHorizontal = rand() % 2 == 0;
            
            placed = placeShip(player, i, x, y, isHorizontal);
            attempts++;
        }
        
        if (!placed) {
            printf("Failed to place ship %d after 100 attempts\n", i);
            exit(1);
        }
    }
}

// Print the 10x10 board with proper formatting
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
            if (!showShips && isalpha(c) && c != 'X' && c != 'O') {
                c = '~'; // Hide ships if not showing
            }
            printf("%2c", c);
        }
        printf("\n");
    }
}

// Print current game state
void printGameState(Player *player, Player *opponent) {
    printf("\n=== YOUR SHIPS ===\n");
    printBoard(player->board, true);
    
    printf("\n=== YOUR ATTACKS ===\n");
    printBoard(player->attackBoard, false);
}

// Execute an attack
bool makeAttack(Player *attacker, Player *defender, int x, int y) {
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
        attacker->attackBoard[y][x] = 'X';
        defender->board[y][x] = 'X';
        
        // Update ship status
        for (int i = 0; i < NUM_SHIPS; i++) {
            if (defender->ships[i].symbol == shipSymbol) {
                defender->ships[i].hits++;
                if (defender->ships[i].hits == defender->ships[i].type) {
                    defender->ships[i].isSunk = true;
                    defender->shipsRemaining--;
                    printf("You sunk the %c ship!\n", shipSymbol);
                }
                break;
            }
        }
    } else {
        attacker->attackBoard[y][x] = 'O';
        defender->board[y][x] = 'O';
    }
    return true;
}

// Check if game is over
bool isGameOver(Player *player) {
    return player->shipsRemaining == 0;
}

// Main game loop
void playGame() {
    Player human, computer;
    
    initializePlayer(&human);
    initializePlayer(&computer);
    
    printf("=== BATTLESHIP GAME ===\n");
    printf("Board size: %dx%d\n", BOARD_SIZE, BOARD_SIZE);
    
    // Place ships
    placeShips(&human);
    placeShips(&computer);
    
    bool humanTurn = true;
    while (!isGameOver(&human) && !isGameOver(&computer)) {
        if (humanTurn) {
            // Human's turn
            printGameState(&human, &computer);
            
            int x, y;
            printf("\nEnter attack coordinates (x y): ");
            while (scanf("%d %d", &x, &y) != 2 || !makeAttack(&human, &computer, x, y)) {
                printf("Invalid coordinates. Try again (x y): ");
                while (getchar() != '\n'); // Clear input buffer
            }
            printf("You attacked (%d, %d)\n", x, y);
        } else {
            // Computer's turn
            int x, y;
            do {
                x = rand() % BOARD_SIZE;
                y = rand() % BOARD_SIZE;
            } while (!makeAttack(&computer, &human, x, y));
            
            printf("Computer attacked (%d, %d)\n", x, y);
        }
        
        humanTurn = !humanTurn;
    }
    
    // Game over
    printf("\n=== FINAL BOARDS ===\n");
    printGameState(&human, &computer);
    printf("\n=== OPPONENT'S SHIPS ===\n");
    printBoard(computer.board, true);
    
    if (isGameOver(&human)) {
        printf("\nGAME OVER - COMPUTER WINS!\n");
    } else {
        printf("\nCONGRATULATIONS - YOU WIN!\n");
    }
}

int main() {
    srand(time(NULL)); // Seed random number generator
    playGame();
    return 0;
}


