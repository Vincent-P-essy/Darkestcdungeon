#ifndef DUNGEON_H
#define DUNGEON_H

#include <stddef.h>
#include "structures.h"

extern const Enemy DUNGEON_ENEMIES[10];
GameState *create_new_game(void);
int character_max_hp(const Character *character);
int character_attack(const Character *character);
int character_healing(const Character *character);
int list_size(const Character *list);
int select_fighters(GameState *state, Character **team, int count);
int equip_accessory(GameState *state, Character *character, Accessory *accessory, int slot);
int unequip_accessory(GameState *state, Character *character, int slot);
int send_to_rest(GameState *state, Character *character, int tavern);
int recall_character(GameState *state, Character *character);
int buy_accessory(GameState *state, Accessory *accessory);
int perform_player_action(GameState *state, Enemy *enemy, Character *actor,
                          char action, Character *target, char *message, size_t size);
void reset_defense(Character *fighters);
void remove_dead_fighters(GameState *state);
int has_surviving_heroes(const GameState *state);

#endif
