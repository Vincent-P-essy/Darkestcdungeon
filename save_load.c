#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "save_load.h"

// Saubevgarder l'état du jeu dans un fichier
int save_game(const char *filename, GameState *state)
{
    FILE *file = fopen(filename, "w");
    if (!file)
        return 0;

    // Sauvegarder l'état global
    fprintf(file, "LEVEL %d\n", state->current_level);
    fprintf(file, "GOLD %d\n", state->gold);

    // Sauvegarder les personnages disponibles
    Character *current_char = state->available_characters;
    while (current_char)
    {
        fprintf(file, "AVAILABLE_CHAR %s %d %d %d %d %d %d %d\n",
                current_char->name,
                current_char->class.type,
                current_char->HP,
                current_char->class.att,
                current_char->class.def,
                current_char->class.rest,
                current_char->stress,
                current_char->nbcomb);
        current_char = current_char->next;
    }

    // Sauvegarder les accessoires
    Accessory *current_acc = state->available_accessories;
    while (current_acc)
    {
        fprintf(file, "ACCESSORY %s %d %d %d %d %d\n",
                current_acc->name,
                current_acc->attbonus,
                current_acc->defbonus,
                current_acc->HPbonus,
                current_acc->restbonus,
                current_acc->strred);
        current_acc = current_acc->next;
    }

    fclose(file);
    return 1;
}

// Charger l'état du jeu depuis un fichier
GameState *load_game(const char *filename)
{
    FILE *file = fopen(filename, "r");
    if (!file)
    {
        printf("Impossible d'ouvrir le fichier %s\n", filename);
        return NULL;
    }

    GameState *state = malloc(sizeof(GameState));
    if (!state)
    {
        printf("Erreur d'allocation de la memoire pour GameState\n");
        fclose(file);
        return NULL;
    }

    // Initialiser les listes à NULL
    state->available_characters = NULL;
    state->sanitarium_characters = NULL;
    state->tavern_characters = NULL;
    state->fighting_characters = NULL;
    state->available_accessories = NULL;
    state->shop_accessories = NULL;
    state->current_level = 1;
    state->gold = 0;

    char line[256];
    int line_count = 0;

    while (fgets(line, sizeof(line), file))
    {
        line_count++;
        size_t len = strlen(line);
        if (len > 0 && line[len - 1] == '\n')
        {
            line[len - 1] = '\0';
        }

        char type[50];
        if (sscanf(line, "%s", type) != 1)
        {
            printf("Erreur de lecture de la ligne %d: %s\n", line_count, line);
            continue;
        }

        if (strcmp(type, "LEVEL") == 0)
        {
            if (sscanf(line, "LEVEL %d", &state->current_level) != 1)
            {
                printf("Erreur de lecture du niveau\n");
            }
        }
        else if (strcmp(type, "GOLD") == 0)
        {
            if (sscanf(line, "GOLD %d", &state->gold) != 1)
            {
                printf("Erreur de lecture de l'or\n");
            }
        }
        else if (strcmp(type, "AVAILABLE_CHAR") == 0)
        {
            char name[50];
            int class_type, hp, att, def, rest, stress, nbcomb;
            if (sscanf(line, "AVAILABLE_CHAR %s %d %d %d %d %d %d %d",
                       name, &class_type, &hp, &att, &def, &rest, &stress, &nbcomb) == 8)
            {

                Character *new_char = create_character(name, (ClassType)class_type);
                if (new_char)
                {
                    new_char->HP = hp;
                    new_char->class.att = att;
                    new_char->class.def = def;
                    new_char->class.rest = rest;
                    new_char->stress = stress;
                    new_char->nbcomb = nbcomb;
                    state->available_characters = add_character_to_list(state->available_characters, new_char);
                    printf("Personnage charge: %s (classe: %d)\n", name, class_type);
                }
                else
                {
                    printf("Erreur lors de la creation du personnage %s\n", name);
                }
            }
            else
            {
                printf("Erreur format personnage: %s\n", line);
            }
        }
        else if (strcmp(type, "ACCESSORY") == 0)
        {
            char name[50] = {0};
            int attbonus, defbonus, HPbonus, restbonus, strred;
            // Utiliser une approche différente pour lire le nom avec espaces
            char *rest_of_line = line + strlen("ACCESSORY ");
            char *ptr = rest_of_line;
            int name_len = 0;

            // Trouver la fin du nom (dernière occurrence de chiffres)
            while (*ptr)
            {
                if (isdigit(*ptr) || *ptr == '-')
                {
                    break;
                }
                ptr++;
                name_len++;
            }

            // Reculer pour éliminer les espaces à la fin
            while (name_len > 0 && isspace(rest_of_line[name_len - 1]))
            {
                name_len--;
            }

            // Copier le nom
            strncpy(name, rest_of_line, name_len);
            name[name_len] = '\0';

            // Lire les valeurs numériques
            if (sscanf(ptr, "%d %d %d %d %d",
                       &attbonus, &defbonus, &HPbonus, &restbonus, &strred) == 5)
            {

                Accessory *new_acc = create_accessory(name, attbonus, defbonus, HPbonus, restbonus, strred);
                if (new_acc)
                {
                    state->available_accessories = add_accessory_to_list(state->available_accessories, new_acc);
                }
                else
                {
                    printf("Erreur lors de la creation de l'accessoire %s\n", name);
                }
            }
            else
            {
                printf("Erreur format accessoire: %s\n", line);
            }
        }
    }

    printf("Chargement termine. Niveau: %d, Or: %d\n", state->current_level, state->gold);
    fclose(file);
    return state;
}

void free_game_state(GameState *state)
{
    if (!state)
        return;

    // Libération sécurisée des personnages
    if (state->available_characters)
    {
        Character *current = state->available_characters;
        while (current)
        {
            Character *next = current->next;
            if (current->acc1)
            {
                free(current->acc1);
                current->acc1 = NULL;
            }
            if (current->acc2)
            {
                free(current->acc2);
                current->acc2 = NULL;
            }
            free(current);
            current = next;
        }
        state->available_characters = NULL;
    }

    // Libération sécurisée des personnages du sanitarium
    if (state->sanitarium_characters)
    {
        Character *current = state->sanitarium_characters;
        while (current)
        {
            Character *next = current->next;
            if (current->acc1)
            {
                free(current->acc1);
                current->acc1 = NULL;
            }
            if (current->acc2)
            {
                free(current->acc2);
                current->acc2 = NULL;
            }
            free(current);
            current = next;
        }
        state->sanitarium_characters = NULL;
    }

    // Libération sécurisée des personnages de la taverne
    if (state->tavern_characters)
    {
        Character *current = state->tavern_characters;
        while (current)
        {
            Character *next = current->next;
            if (current->acc1)
            {
                free(current->acc1);
                current->acc1 = NULL;
            }
            if (current->acc2)
            {
                free(current->acc2);
                current->acc2 = NULL;
            }
            free(current);
            current = next;
        }
        state->tavern_characters = NULL;
    }

    // Libération sécurisée des personnages en combat
    if (state->fighting_characters)
    {
        Character *current = state->fighting_characters;
        while (current)
        {
            Character *next = current->next;
            if (current->acc1)
            {
                free(current->acc1);
                current->acc1 = NULL;
            }
            if (current->acc2)
            {
                free(current->acc2);
                current->acc2 = NULL;
            }
            free(current);
            current = next;
        }
        state->fighting_characters = NULL;
    }

    // Libération sécurisée des accessoires disponibles
    if (state->available_accessories)
    {
        Accessory *current = state->available_accessories;
        while (current)
        {
            Accessory *next = current->next;
            free(current);
            current = next;
        }
        state->available_accessories = NULL;
    }

    // Libération sécurisée des accessoires de la boutique
    if (state->shop_accessories)
    {
        Accessory *current = state->shop_accessories;
        while (current)
        {
            Accessory *next = current->next;
            free(current);
            current = next;
        }
        state->shop_accessories = NULL;
    }

    // Libération de la structure GameState elle-même
    free(state);
}