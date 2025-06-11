#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "accessory.h"

// Créer un nouvel accessoire
Accessory* create_accessory(const char* name, int attbonus, int defbonus, int HPbonus, int restbonus, int strred) {
    Accessory* new_acc = malloc(sizeof(Accessory));
    if (!new_acc) return NULL;
    
    strncpy(new_acc->name, name, 49);
    new_acc->name[49] = '\0';
    new_acc->attbonus = attbonus;
    new_acc->defbonus = defbonus;
    new_acc->HPbonus = HPbonus;
    new_acc->restbonus = restbonus;
    new_acc->strred = strred;
    new_acc->next = NULL;
    
    return new_acc;
}

// Afficher un accessoire
void display_accessory(const Accessory* accessory) {
    printf("%-20s +%d att +%d def +%d HP +%d rest -%d str\n",
           accessory->name,
           accessory->attbonus,
           accessory->defbonus, 
           accessory->HPbonus,
           accessory->restbonus,
           accessory->strred);
}

// Ajouter un accessoire à une liste
Accessory* add_accessory_to_list(Accessory* list, Accessory* new_acc) {
    if (!new_acc) return list;
    new_acc->next = list;
    return new_acc;
}

Accessory* remove_accessory_from_list(Accessory* list, Accessory* acc_to_remove) {
    if (!list || !acc_to_remove) return list;
    
    if (list == acc_to_remove) {
        Accessory* new_head = list->next;
        acc_to_remove->next = NULL;
        return new_head;
    }
    
    Accessory* current = list;
    while (current->next && current->next != acc_to_remove) {
        current = current->next;
    }
    
    if (current->next) {
        current->next = acc_to_remove->next;
        acc_to_remove->next = NULL;
    }
    
    return list;
}

// Libérer une liste d'accessoires
void free_accessory_list(Accessory* list) {
    while (list) {
        Accessory* temp = list;
        list = list->next;
        free(temp);
    }
}