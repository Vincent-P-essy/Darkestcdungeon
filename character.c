#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "character.h"

// Classes prédéfinies avec leurs statistiques
const Class CLASS_STATS[] = {
    {CLASS_FURIE, 13, 0, 20, 0},            // Furie
    {CLASS_VESTALE, 3, 0, 20, 10},          // Vestale 
    {CLASS_CHASSEUR_DE_PRIMES, 7, 3, 25, 3}, // Chasseur de primes
    {CLASS_MAITRE_CHIEN, 10, 6, 17, 5}      // Maître chien
};

// Conversion de ClassType en string pour l'affichage
const char* get_class_name(ClassType type) {
    switch(type) {
        case CLASS_FURIE: return "Furie";
        case CLASS_VESTALE: return "Vestale";
        case CLASS_CHASSEUR_DE_PRIMES: return "Chasseur";
        case CLASS_MAITRE_CHIEN: return "M. Chien";
        default: return "Inconnue";
    }
}

// Créer un nouveau personnage
Character* create_character(const char* name, ClassType class_type) {
    Character* new_char = malloc(sizeof(Character));
    if (!new_char) return NULL;
    
    strncpy(new_char->name, name, 49);
    new_char->name[49] = '\0';
    new_char->class = CLASS_STATS[class_type];
    new_char->HP = new_char->class.HPmax;
    new_char->stress = 0;
    new_char->acc1 = NULL;
    new_char->acc2 = NULL;
    new_char->nbcomb = 0;
    new_char->is_defending = 0;
    new_char->next = NULL;
    
    return new_char;
}

// Afficher un personnage
void display_character(const Character* character) {
    // Calculer les bonus totaux des accessoires
    int att_bonus = 0, def_bonus = 0, hp_bonus = 0, rest_bonus = 0;
    
    if (character->acc1) {
        att_bonus += character->acc1->attbonus;
        def_bonus += character->acc1->defbonus;
        hp_bonus += character->acc1->HPbonus;
        rest_bonus += character->acc1->restbonus;
    }
    if (character->acc2) {
        att_bonus += character->acc2->attbonus;
        def_bonus += character->acc2->defbonus;
        hp_bonus += character->acc2->HPbonus;
        rest_bonus += character->acc2->restbonus;
    }

    printf("%-15s %-10s %3d(+%d) %3d(+%d) %3d/%-3d %3d(+%d) %3d %3d\n",
           character->name,
           get_class_name(character->class.type),
           character->class.att, att_bonus,
           character->class.def, def_bonus,
           character->HP, character->class.HPmax + hp_bonus,
           character->class.rest, rest_bonus,
           character->stress,
           character->nbcomb);
           
    // Afficher les accessoires équipés
    if (character->acc1) {
        printf("   Accessoire 1: %s\n", character->acc1->name);
    }
    if (character->acc2) {
        printf("   Accessoire 2: %s\n", character->acc2->name);
    }
}

// Ajouter un personnage à une liste
Character* add_character_to_list(Character* list, Character* new_char) {
    if (!new_char) return list;
    new_char->next = list;
    return new_char;
}

// Retirer un personnage d'une liste
Character* remove_character_from_list(Character* list, Character* char_to_remove) {
    if (!list || !char_to_remove) return list;
    
    if (list == char_to_remove) {
        Character* new_head = list->next;
        char_to_remove->next = NULL;
        return new_head;
    }
    
    Character* current = list;
    while (current->next && current->next != char_to_remove) {
        current = current->next;
    }
    
    if (current->next) {
        current->next = char_to_remove->next;
        char_to_remove->next = NULL;
    }
    
    return list;
}

// Libérer une liste de personnages
void free_character_list(Character* list) {
    while (list) {
        Character* temp = list;
        list = list->next;
        
        // Libérer les accessoires si présents
        if (temp->acc1) free(temp->acc1);
        if (temp->acc2) free(temp->acc2);
        
        free(temp);
    }
}