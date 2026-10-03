#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "combat.h"
#include "accessory.h"
#include "character.h"
#include "dungeon.h"

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
    int damage = (int)(base_damage * roll);
    return damage > 0 ? damage : 1;
}

// Fonction pour appliquer un soin
void apply_healing(Character *target, int healing)
{
    if (!target || target->HP <= 0 || healing <= 0)
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
    if (!target || target->HP <= 0 || damage <= 0)
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
        if (stress_damage < 0) stress_damage = 0;

        apply_stress(current, stress_damage, 0);

        printf("L'ennemi stresse %s de %d points!\n",
               current->name, stress_damage);

        if (current->stress >= 100)
        {
            printf("%s est submerge par le stress!\n", current->name);
        }
    }
}

void start_combat(GameState *state, Enemy *enemy) {
    if (!state || !enemy || !state->fighting_characters) return;
    int turn = 1;
    while (!is_all_dead(state->fighting_characters) &&
           !is_all_stressed(state->fighting_characters) && enemy->HPenn > 0) {
        printf("\n=== Tour %d : %s (%d PV) ===\n", turn, enemy->name, enemy->HPenn);
        Character *current = state->fighting_characters;
        while (current && enemy->HPenn > 0) {
            if (current->HP > 0 && current->stress < 100) {
                display_character(current);
                char line[64], message[256], action;
                int accepted = 0;
                while (!accepted) {
                    printf("Action de %s (A:Attaque, D:Defense, R:Restauration): ", current->name);
                    if (!fgets(line, sizeof(line), stdin)) {
                        end_combat(state, 0);
                        return;
                    }
                    if (sscanf(line, " %c", &action) != 1) continue;
                    Character *target = NULL;
                    if (action == 'R' || action == 'r') {
                        int index;
                        printf("Cible du soin (numero dans l'equipe): ");
                        if (!fgets(line, sizeof(line), stdin)) { end_combat(state, 0); return; }
                        if (sscanf(line, "%d", &index) == 1 && index > 0) {
                            target = state->fighting_characters;
                            for (int i = 1; target && i < index; ++i) target = target->next;
                        }
                    }
                    accepted = perform_player_action(state, enemy, current, action, target,
                                                       message, sizeof(message));
                    puts(accepted ? message : "Action ou cible invalide, reessayez.");
                }
            }
            current = current->next;
        }
        if (enemy->HPenn <= 0) break;
        perform_enemy_action(enemy, state->fighting_characters);
        reset_defense(state->fighting_characters);
        remove_dead_fighters(state);
        ++turn;
    }
    int victory = enemy->HPenn <= 0;
    puts(victory ? "Victoire !" : "Combat perdu ou equipe submergee par le stress.");
    end_combat(state, victory);
}

void end_combat(GameState *state, int victory) {
    if (!state) return;
    while (state->fighting_characters) {
        Character *c = state->fighting_characters;
        state->fighting_characters = remove_character_from_list(state->fighting_characters, c);
        if (c->HP <= 0) {
            free_character_list(c);
            continue;
        }
        ++c->nbcomb;
        c->is_defending = 0;
        state->available_characters = add_character_to_list(state->available_characters, c);
        unequip_accessory(state, c, 1);
        unequip_accessory(state, c, 2);
    }
    for (Character *c = state->sanitarium_characters; c; c = c->next) apply_healing(c, 7);
    for (Character *c = state->tavern_characters; c; c = c->next) {
        c->stress -= 25;
        if (c->stress < 0) c->stress = 0;
    }
    if (!victory) return;
    int level = state->current_level;
    state->gold += 10;
    char name[50];
    snprintf(name, sizeof(name), "Relique du niveau %d", level);
    Accessory *loot = create_accessory(name, level + 1, level / 2, level, level / 3, level);
    state->available_accessories = add_accessory_to_list(state->available_accessories, loot);
    const char *names[] = {"William", "Tardif", "Alhazred", "Dismas"};
    const ClassType classes[] = {CLASS_MAITRE_CHIEN, CLASS_CHASSEUR_DE_PRIMES, CLASS_VESTALE, CLASS_FURIE};
    if (level % 2 == 0 && level >= 2 && level <= 8) {
        Character *recruit = create_character(names[level / 2 - 1], classes[level / 2 - 1]);
        state->available_characters = add_character_to_list(state->available_characters, recruit);
    }
    ++state->current_level;
}
