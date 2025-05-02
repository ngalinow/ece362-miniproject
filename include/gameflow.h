
#ifndef GAMEFLOW_H
#define GAMEFLOW_H

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>
#include <ctype.h>

void game_flow();

extern uint8_t game_state;
extern int ship_count; 
extern uint8_t coords;
extern int hit;
extern uint8_t response; 

#endif 



