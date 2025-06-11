#ifndef ACCESSORY_H 
#define ACCESSORY_H

#include "structures.h"

Accessory* create_accessory(const char* name, int attbonus, int defbonus, int HPbonus, int restbonus, int strred);
void display_accessory(const Accessory* accessory);
Accessory* add_accessory_to_list(Accessory* list, Accessory* new_acc);
Accessory* remove_accessory_from_list(Accessory* list, Accessory* acc_to_remove);
void free_accessory_list(Accessory* list);

#endif