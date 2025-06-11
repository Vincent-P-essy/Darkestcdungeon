#ifndef CHARACTER_H
#define CHARACTER_H

#include "structures.h"

// Déclaration des fonctions
Character* create_character(const char* name, ClassType class_type);
void display_character(const Character* character);
Character* add_character_to_list(Character* list, Character* new_char);
Character* remove_character_from_list(Character* list, Character* char_to_remove);
void free_character_list(Character* list);
const char* get_class_name(ClassType type);

#endif