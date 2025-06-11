#ifndef STRUCTURES_H
#define STRUCTURES_H

// Définition des structures
typedef enum
{
    CLASS_FURIE,
    CLASS_VESTALE,
    CLASS_CHASSEUR_DE_PRIMES,
    CLASS_MAITRE_CHIEN
} ClassType;

// Structure de classe
typedef struct
{
    ClassType type;
    int att;
    int def;
    int HPmax;
    int rest;
} Class;

// Structure d'accessoire
typedef struct Accessory
{
    char name[50];
    int attbonus;
    int defbonus;
    int HPbonus;
    int restbonus;
    int strred;
    int price; // Pour la roulotte
    struct Accessory *next;
} Accessory;

// Structure de personnage
typedef struct Character
{
    char name[50];
    Class class;
    int HP;
    int stress;
    Accessory *acc1;
    Accessory *acc2;
    int nbcomb;
    int is_defending; // Pour le combat
    struct Character *next;
} Character;

// Structure d'ennemi
typedef struct
{
    char name[50];
    int level;
    int attenn;
    int defenn;
    int HPenn;
    int attstrenn;
} Enemy;

// Structure d'état du jeu
typedef struct
{
    int current_level;
    int gold;
    Character *available_characters;
    Character *sanitarium_characters;
    Character *tavern_characters;
    Character *fighting_characters;
    Accessory *available_accessories;
    Accessory *shop_accessories;
} GameState;

#endif