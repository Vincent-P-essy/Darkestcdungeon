#ifndef COMBAT_H
#define COMBAT_H

#include "structures.h"

// Structure combat
void start_combat(GameState* state, Enemy* enemy);
void end_combat(GameState* state, int victory);

// Actions de combat
int calculate_damage(int attack, int defense);
void perform_enemy_action(Enemy* enemy, Character* fighters);
Character* select_healing_target(Character* fighters);
int is_all_dead(Character* fighters);
int is_all_stressed(Character* fighters);

// Gestion du combat
void apply_healing(Character* target, int healing);
void apply_damage(Character* target, int damage);
void apply_stress(Character* target, int stress, int stress_resistance);
float get_random_roll();

#endif