#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "combat.h"
#include "accessory.h"

// Fonction pour obtenir un nombre aléatoire entre 0.8 et 1.2
float get_random_roll()
{
    return (float)(rand() % 41 + 80) / 100.0f; // Entre 0.8 et 1.2
}

// Fonction pour calculer les dégâts infligés
int calculate_damage(int attack, int defense)
{
    float roll = get_random_roll();
    int base_damage = attack - defense;
    if (base_damage <= 0)
        return 1;
    return (int)(base_damage * roll);
}

// Fonction pour appliquer un soin
void apply_healing(Character *target, int healing)
{
    if (!target)
        return;
    int max_hp = target->class.HPmax +
                 (target->acc1 ? target->acc1->HPbonus : 0) +
                 (target->acc2 ? target->acc2->HPbonus : 0);

    target->HP += healing;
    if (target->HP > max_hp)
        target->HP = max_hp;
}

// Fonction pour appliquer des dégâts
void apply_damage(Character *target, int damage)
{
    if (!target || target->HP <= 0)
        return;
    target->HP -= damage;
    if (target->HP < 0)
        target->HP = 0;
}

// Fonction pour appliquer du stress
void apply_stress(Character *target, int stress, int stress_resistance)
{
    if (!target || target->stress >= 100)
        return;
    int final_stress = stress - stress_resistance;
    if (final_stress > 0)
    {
        target->stress += final_stress;
        if (target->stress > 100)
            target->stress = 100;
    }
}

// Fonction pour sélectionner une cible de soin
Character *select_healing_target(Character *fighters)
{
    if (!fighters)
        return NULL;

    printf("Choisir la cible du soin:\n");
    int idx = 1;
    Character *current = fighters;
    while (current)
    {
        if (current->HP > 0)
        {
            printf("%d. %s (%d/%d HP)\n", idx, current->name,
                   current->HP, current->class.HPmax);
        }
        idx++;
        current = current->next;
    }

    int choice;
    scanf("%d", &choice);

    current = fighters;
    for (int i = 1; i < choice && current; i++)
    {
        current = current->next;
    }

    return current;
}

// Fonction pour vérifier si tous les combattants sont morts
int is_all_dead(Character *fighters)
{
    Character *current = fighters;
    while (current)
    {
        if (current->HP > 0)
            return 0;
        current = current->next;
    }
    return 1;
}

// Fonction pour vérifier si tous les combattants sont submergés par le stress
int is_all_stressed(Character *fighters)
{
    Character *current = fighters;
    while (current)
    {
        if (current->HP > 0 && current->stress < 100)
            return 0;
        current = current->next;
    }
    return 1;
}

// Fonction pour effectuer l'action de l'ennemi
void perform_enemy_action(Enemy *enemy, Character *fighters)
{
    // Compter les cibles valides
    int valid_targets = 0;
    Character *current = fighters;
    while (current)
    {
        if (current->HP > 0)
            valid_targets++;
        current = current->next;
    }

    if (valid_targets == 0)
        return;

    // Sélectionner une cible au hasard
    int target_idx = rand() % valid_targets;
    current = fighters;
    while (target_idx > 0 || current->HP <= 0)
    {
        if (current->HP > 0)
            target_idx--;
        current = current->next;
    }

    // Décider du type d'attaque
    if (rand() % 2 == 0 || current->stress >= 100)
    {
        // Attaque physique
        int defense = current->class.def +
                      (current->acc1 ? current->acc1->defbonus : 0) +
                      (current->acc2 ? current->acc2->defbonus : 0);

        // Bonus de défense si en position défensive
        if (current->is_defending)
        {
            defense = (int)(defense * 1.1);
        }

        int damage = calculate_damage(enemy->attenn, defense);
        apply_damage(current, damage);

        printf("L'ennemi attaque %s pour %d points de degats!\n",
               current->name, damage);

        if (current->HP <= 0)
        {
            printf("%s est mort au combat...\n", current->name);
        }
    }
    else
    {
        // Attaque de stress
        int stress_red = (current->acc1 ? current->acc1->strred : 0) +
                         (current->acc2 ? current->acc2->strred : 0);

        float roll = get_random_roll();
        int stress_damage = (int)((enemy->attstrenn - stress_red) * roll);

        apply_stress(current, stress_damage, 0);

        printf("L'ennemi stresse %s de %d points!\n",
               current->name, stress_damage);

        if (current->stress >= 100)
        {
            printf("%s est submerge par le stress!\n", current->name);
        }
    }
}

// Fonction pour démarrer un combat
void start_combat(GameState *state, Enemy *enemy)
{
    printf("\nCombat contre %s (niveau %d)\n", enemy->name, enemy->level);
    printf("Points de vie: %d\n", enemy->HPenn);

    int turn = 1;

    while (!is_all_dead(state->fighting_characters) && enemy->HPenn > 0)
    {
        printf("\n=== Tour %d ===\n", turn);

        // Afficher état des combattants
        Character *current = state->fighting_characters;
        while (current)
        {
            if (current->HP > 0)
            {
                printf("%s: %d HP, %d stress\n",
                       current->name, current->HP, current->stress);
            }
            current = current->next;
        }

        // Tour des personnages
        current = state->fighting_characters;
        while (current)
        {
            if (current->HP > 0 && current->stress < 100)
            {
                printf("\nAction de %s (A:Attaque, D:Defense, R:Restauration): ",
                       current->name);
                char action;
                scanf(" %c", &action);

                switch (action)
                {
                case 'A':
                case 'a':
                {
                    int att_total = current->class.att +
                                    (current->acc1 ? current->acc1->attbonus : 0) +
                                    (current->acc2 ? current->acc2->attbonus : 0);

                    int damage = calculate_damage(att_total, enemy->defenn);
                    enemy->HPenn -= damage;
                    printf("%s inflige %d degats a l'ennemi!\n",
                           current->name, damage);
                    break;
                }
                case 'D':
                case 'd':
                    current->is_defending = 1;
                    printf("%s se met en position defensive.\n", current->name);
                    break;
                case 'R':
                case 'r':
                {
                    int healing = current->class.rest +
                                  (current->acc1 ? current->acc1->restbonus : 0) +
                                  (current->acc2 ? current->acc2->restbonus : 0);

                    Character *target = select_healing_target(state->fighting_characters);
                    if (target)
                    {
                        apply_healing(target, healing);
                        printf("%s soigne %s pour %d points de vie.\n",
                               current->name, target->name, healing);
                    }
                    break;
                }
                }
            }
            current = current->next;
        }

        // Réinitialiser les états de défense
        current = state->fighting_characters;
        while (current)
        {
            current->is_defending = 0;
            current = current->next;
        }

        // Vérifier si l'ennemi est mort
        if (enemy->HPenn <= 0)
        {
            printf("\nVictoire! L'ennemi est vaincu!\n");
            state->gold += 10;
            end_combat(state, 1);
            return;
        }

        // Tour de l'ennemi
        if (!is_all_dead(state->fighting_characters) &&
            !is_all_stressed(state->fighting_characters))
        {
            perform_enemy_action(enemy, state->fighting_characters);
        }

        turn++;
    }

    // Fin du combat
    if (is_all_dead(state->fighting_characters))
    {
        printf("\nDefaite! Tous vos personnages sont morts!\n");
        end_combat(state, 0);
    }
}

// Fonction pour terminer un combat
void end_combat(GameState* state, int victory) {
    if (!state) return;

    // Mettre à jour les compteurs de combat pour les survivants
    Character* current = state->fighting_characters;
    while (current) {
        if (current->HP > 0) {
            current->nbcomb++;
        }
        current = current->next;
    }
    
    // Gérer les accessoires et personnages morts
    current = state->fighting_characters;
    while (current) {
        if (current->HP <= 0) {
            // Détruire les accessoires des morts
            if (current->acc1) {
                free(current->acc1);
                current->acc1 = NULL;
            }
            if (current->acc2) {
                free(current->acc2);
                current->acc2 = NULL;
            }
        } else {
            // Si victoire, conserver les accessoires pour la prochaine fois
            if (victory) {
                if (current->acc1) {
                    state->available_accessories = add_accessory_to_list(state->available_accessories, current->acc1);
                    current->acc1 = NULL;
                }
                if (current->acc2) {
                    state->available_accessories = add_accessory_to_list(state->available_accessories, current->acc2);
                    current->acc2 = NULL;
                }
            }
        }
        current = current->next;
    }
}