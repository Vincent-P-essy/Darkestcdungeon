#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "structures.h"
#include "character.h"
#include "combat.h"
#include "save_load.h"
#include "accessory.h"

// Initialisation des ennemis
Enemy ENEMIES[] = {
    {"Brigand", 1, 3, 3, 9, 0},
    {"Squelette", 2, 6, 4, 13, 10},
    {"Goule", 3, 8, 8, 16, 20},
    {"Gargouille", 4, 10, 10, 20, 25},
    {"Cultiste", 5, 12, 12, 25, 30},
    {"Bandit", 6, 15, 15, 30, 35},
    {"Necromancien", 7, 18, 18, 35, 40},
    {"Sorcier", 8, 20, 20, 40, 45},
    {"Dragon", 9, 25, 25, 50, 50},
    {"Boss Final", 10, 30, 30, 60, 60}};

int main()
{
    srand(time(NULL));

    GameState *game = malloc(sizeof(GameState));
    if (!game)
        return 1;

    int choice;
    do
    {
        printf("\n=== Darkest C Dungeon ===\n");
        printf("1. Nouveau jeu\n");
        printf("2. Charger une partie\n");
        printf("3. Quitter\n");
        printf("Choix : ");
        scanf("%d", &choice);

        switch (choice)
        {
        case 1:
        {
            // Initialiser une nouvelle partie
            game->current_level = 1;
            game->gold = 0;

            // Créer les deux premiers personnages
            Character *char1 = create_character("Boudicca", CLASS_FURIE);
            Character *char2 = create_character("Junia", CLASS_VESTALE);

            game->available_characters = NULL;
            game->available_characters = add_character_to_list(game->available_characters, char1);
            game->available_characters = add_character_to_list(game->available_characters, char2);

            // Créer les premiers accessoires
            Accessory *acc1 = create_accessory("Pendentif tranchant", 5, 1, 0, 0, 0);
            Accessory *acc2 = create_accessory("Calice de jeunesse", 0, 3, 5, 0, 5);

            game->available_accessories = NULL;
            game->available_accessories = add_accessory_to_list(game->available_accessories, acc1);
            game->available_accessories = add_accessory_to_list(game->available_accessories, acc2);

            // Initialiser les autres listes
            game->sanitarium_characters = NULL;
            game->tavern_characters = NULL;
            game->fighting_characters = NULL;
            game->shop_accessories = NULL;

            // Boucle principale du jeu
            while (game->current_level <= 10)
            {
                printf("\n=== Niveau %d ===\n", game->current_level);
                printf("Or: %d\n\n", game->gold);

                // Gérer le sanitarium
                printf("=== Sanitarium ===\n");
                if (game->sanitarium_characters)
                {
                    Character *current = game->sanitarium_characters;
                    while (current)
                    {
                        display_character(current);
                        printf("Faire sortir %s du sanitarium? (O/N): ", current->name);
                        char choice;
                        scanf(" %c", &choice);
                        if (choice == 'O' || choice == 'o')
                        {
                            game->sanitarium_characters = remove_character_from_list(
                                game->sanitarium_characters, current);
                            game->available_characters = add_character_to_list(
                                game->available_characters, current);
                        }
                        current = current->next;
                    }
                }

                // Gérer la taverne
                printf("\n=== Taverne ===\n");
                if (game->tavern_characters)
                {
                    Character *current = game->tavern_characters;
                    while (current)
                    {
                        display_character(current);
                        printf("Faire sortir %s de la taverne? (O/N): ", current->name);
                        char choice;
                        scanf(" %c", &choice);
                        if (choice == 'O' || choice == 'o')
                        {
                            game->tavern_characters = remove_character_from_list(
                                game->tavern_characters, current);
                            game->available_characters = add_character_to_list(
                                game->available_characters, current);
                        }
                        current = current->next;
                    }
                }

                // Sélection des combattants
                printf("\n=== Selection des combattants ===\n");
                int max_fighters = game->current_level <= 5 ? 2 : 3;

                Character *current = game->available_characters;
                int idx = 1;
                while (current)
                {
                    printf("%d. ", idx++);
                    display_character(current);
                    current = current->next;
                }

                game->fighting_characters = NULL;
                for (int i = 0; i < max_fighters; i++)
                {
                    printf("Choisir combattant %d (0 pour terminer): ", i + 1);
                    int choice;
                    scanf("%d", &choice);

                    if (choice == 0)
                        break;

                    current = game->available_characters;
                    for (int j = 1; j < choice && current; j++)
                    {
                        current = current->next;
                    }

                    if (current)
                    {
                        game->available_characters = remove_character_from_list(
                            game->available_characters, current);
                        game->fighting_characters = add_character_to_list(
                            game->fighting_characters, current);
                    }
                }

                // Combat
                start_combat(game, &ENEMIES[game->current_level - 1]);

                // Après le combat
                if (!game->fighting_characters)
                {
                    printf("Game Over!\n");
                    break;
                }

                // Retour des survivants
                while (game->fighting_characters)
                {
                    Character *fighter = game->fighting_characters;
                    game->fighting_characters = remove_character_from_list(
                        game->fighting_characters, fighter);
                    game->available_characters = add_character_to_list(
                        game->available_characters, fighter);
                }

                // Nouveau personnage aux niveaux 2, 4, 6, 8
                if (game->current_level % 2 == 0 && game->current_level < 9)
                {
                    printf("\nUn nouveau heros rejoint votre equipe!\n");
                    Character *new_char = NULL;
                    switch (game->current_level)
                    {
                    case 2:
                        new_char = create_character("William", CLASS_MAITRE_CHIEN);
                        break;
                    case 4:
                        new_char = create_character("Tardif", CLASS_CHASSEUR_DE_PRIMES);
                        break;
                    case 6:
                        new_char = create_character("Alhazred", CLASS_VESTALE);
                        break;
                    case 8:
                        new_char = create_character("Dismas", CLASS_FURIE);
                        break;
                    }
                    if (new_char)
                    {
                        game->available_characters = add_character_to_list(
                            game->available_characters, new_char);
                        display_character(new_char);
                    }
                }

                game->current_level++;

                // Option de sauvegarde
                printf("\nSauvegarder la partie? (O/N): ");
                char save_choice;
                scanf(" %c", &save_choice);
                if (save_choice == 'O' || save_choice == 'o')
                {
                    char filename[100];
                    printf("Nom du fichier de sauvegarde: ");
                    scanf("%s", filename);
                    if (save_game(filename, game))
                    {
                        printf("Partie sauvegardee avec succes!\n");
                    }
                    else
                    {
                        printf("Erreur lors de la sauvegarde.\n");
                    }
                }
            }
            break;
        }

        case 2:
        {
            char filename[100];
            printf("Nom du fichier de sauvegarde: ");
            scanf("%s", filename);
            GameState *loaded = load_game(filename);
            if (loaded)
            {
                game = loaded;
            }
            else
            {
                printf("Erreur lors du chargement.\n");
            }
            break;
        }

        case 3:
            printf("Au revoir!\n");
            break;

        default:
            printf("Choix invalide.\n");
        }
    } while (choice != 3);

    // Libérer la mémoire
    free_game_state(game);
    return 0;
}