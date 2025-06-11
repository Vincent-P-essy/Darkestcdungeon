#ifndef SAVE_LOAD_H
#define SAVE_LOAD_H

#include "structures.h"
#include "character.h"
#include "accessory.h"

int save_game(const char* filename, GameState* state);
GameState* load_game(const char* filename);
void free_game_state(GameState* state);

#endif