#include <stdio.h>
#include <stdlib.h>
#include "dungeon.h"
#include "character.h"
#include "accessory.h"
#include "combat.h"
#include "save_load.h"

const Enemy DUNGEON_ENEMIES[10] = {
    {"Brigand", 1, 3, 3, 9, 0}, {"Squelette", 2, 6, 4, 13, 10},
    {"Goule", 3, 8, 8, 16, 20}, {"Gargouille", 4, 10, 10, 20, 25},
    {"Cultiste", 5, 12, 12, 25, 30}, {"Bandit", 6, 15, 15, 30, 35},
    {"Necromancien", 7, 18, 18, 35, 40}, {"Sorcier", 8, 20, 20, 40, 45},
    {"Dragon", 9, 25, 25, 50, 50}, {"Boss Final", 10, 30, 30, 60, 60}
};

static int contains(const Character *list, const Character *character) {
    for (; list; list = list->next) if (list == character) return 1;
    return 0;
}

static int owns_character(const GameState *state, const Character *character) {
    return state && character &&
        (contains(state->available_characters, character) ||
         contains(state->fighting_characters, character) ||
         contains(state->sanitarium_characters, character) ||
         contains(state->tavern_characters, character));
}

int list_size(const Character *list) {
    int size = 0;
    for (; list; list = list->next) ++size;
    return size;
}

int character_max_hp(const Character *c) {
    return c->class.HPmax + (c->acc1 ? c->acc1->HPbonus : 0) +
        (c->acc2 ? c->acc2->HPbonus : 0);
}

int character_attack(const Character *c) {
    return c->class.att + (c->acc1 ? c->acc1->attbonus : 0) +
        (c->acc2 ? c->acc2->attbonus : 0);
}

int character_healing(const Character *c) {
    return c->class.rest + (c->acc1 ? c->acc1->restbonus : 0) +
        (c->acc2 ? c->acc2->restbonus : 0);
}

GameState *create_new_game(void) {
    GameState *state = calloc(1, sizeof(*state));
    if (!state) return NULL;
    state->current_level = 1;
    const char *names[] = {"Boudicca", "Junia"};
    const ClassType classes[] = {CLASS_FURIE, CLASS_VESTALE};
    for (int i = 0; i < 2; ++i) {
        Character *c = create_character(names[i], classes[i]);
        if (!c) { free_game_state(state); return NULL; }
        state->available_characters = add_character_to_list(state->available_characters, c);
    }
    const char *items[] = {"Pendentif tranchant", "Calice de jeunesse", "Anneau de garde",
                           "Cape de repos", "Talisman de bravoure"};
    const int stats[][6] = {{5,1,0,0,0,0}, {0,3,5,0,5,0}, {0,3,0,0,0,7},
                            {0,1,5,3,3,16}, {6,4,8,3,8,35}};
    for (int i = 0; i < 5; ++i) {
        Accessory *a = create_accessory(items[i], stats[i][0], stats[i][1], stats[i][2],
                                        stats[i][3], stats[i][4]);
        if (!a) { free_game_state(state); return NULL; }
        a->price = stats[i][5];
        if (i < 2) state->available_accessories = add_accessory_to_list(state->available_accessories, a);
        else state->shop_accessories = add_accessory_to_list(state->shop_accessories, a);
    }
    return state;
}

int select_fighters(GameState *state, Character **team, int count) {
    if (!state || !team || state->fighting_characters || state->current_level > 10 ||
        count < 1 || count > (state->current_level <= 5 ? 2 : 3)) return 0;
    for (int i = 0; i < count; ++i) {
        if (!contains(state->available_characters, team[i]) || team[i]->HP <= 0 ||
            team[i]->stress >= 100) return 0;
        for (int j = 0; j < i; ++j) if (team[i] == team[j]) return 0;
    }
    for (int i = count - 1; i >= 0; --i) {
        state->available_characters = remove_character_from_list(state->available_characters, team[i]);
        state->fighting_characters = add_character_to_list(state->fighting_characters, team[i]);
    }
    return 1;
}

int equip_accessory(GameState *state, Character *c, Accessory *a, int slot) {
    if (!owns_character(state, c) || !a || c->HP <= 0 || (slot != 1 && slot != 2)) return 0;
    Accessory **place = slot == 1 ? &c->acc1 : &c->acc2;
    if (*place) return 0;
    Accessory *item = state->available_accessories;
    while (item && item != a) item = item->next;
    if (!item) return 0;
    state->available_accessories = remove_accessory_from_list(state->available_accessories, a);
    *place = a;
    c->HP += a->HPbonus;
    return 1;
}

int unequip_accessory(GameState *state, Character *c, int slot) {
    if (!owns_character(state, c) || (slot != 1 && slot != 2)) return 0;
    Accessory **place = slot == 1 ? &c->acc1 : &c->acc2;
    Accessory *a = *place;
    if (!a) return 0;
    c->HP -= a->HPbonus;
    if (c->HP < 1) c->HP = 1;
    *place = NULL;
    state->available_accessories = add_accessory_to_list(state->available_accessories, a);
    return 1;
}

int send_to_rest(GameState *state, Character *c, int tavern) {
    if (!state || !contains(state->available_characters, c)) return 0;
    Character **destination = tavern ? &state->tavern_characters : &state->sanitarium_characters;
    if (list_size(*destination) >= 2) return 0;
    state->available_characters = remove_character_from_list(state->available_characters, c);
    *destination = add_character_to_list(*destination, c);
    return 1;
}

int recall_character(GameState *state, Character *c) {
    if (!state || !c) return 0;
    if (contains(state->sanitarium_characters, c))
        state->sanitarium_characters = remove_character_from_list(state->sanitarium_characters, c);
    else if (contains(state->tavern_characters, c))
        state->tavern_characters = remove_character_from_list(state->tavern_characters, c);
    else return 0;
    state->available_characters = add_character_to_list(state->available_characters, c);
    return 1;
}

int buy_accessory(GameState *state, Accessory *a) {
    if (!state || !a || a->price <= 0 || state->gold < a->price) return 0;
    Accessory *item = state->shop_accessories;
    while (item && item != a) item = item->next;
    if (!item) return 0;
    state->gold -= a->price;
    state->shop_accessories = remove_accessory_from_list(state->shop_accessories, a);
    state->available_accessories = add_accessory_to_list(state->available_accessories, a);
    return 1;
}

int perform_player_action(GameState *state, Enemy *enemy, Character *actor,
                          char action, Character *target, char *message, size_t size) {
    if (!state || !enemy || !contains(state->fighting_characters, actor) ||
        actor->HP <= 0 || actor->stress >= 100 || enemy->HPenn <= 0) return 0;
    switch (action) {
        case 'A': case 'a': {
            int damage = calculate_damage(character_attack(actor), enemy->defenn);
            enemy->HPenn -= damage;
            if (enemy->HPenn < 0) enemy->HPenn = 0;
            snprintf(message, size, "%s inflige %d degats a %s.", actor->name, damage, enemy->name);
            return 1;
        }
        case 'D': case 'd':
            actor->is_defending = 1;
            snprintf(message, size, "%s se defend jusqu'a l'action ennemie.", actor->name);
            return 1;
        case 'R': case 'r': {
            if (!contains(state->fighting_characters, target) || target->HP <= 0 ||
                character_healing(actor) <= 0) return 0;
            int before = target->HP;
            apply_healing(target, character_healing(actor));
            snprintf(message, size, "%s restaure %d PV a %s.", actor->name, target->HP - before, target->name);
            return 1;
        }
        default: return 0;
    }
}

void reset_defense(Character *fighters) {
    for (; fighters; fighters = fighters->next) fighters->is_defending = 0;
}

void remove_dead_fighters(GameState *state) {
    Character *c = state->fighting_characters;
    while (c) {
        Character *next = c->next;
        if (c->HP <= 0) {
            state->fighting_characters = remove_character_from_list(state->fighting_characters, c);
            free_character_list(c);
        }
        c = next;
    }
}

int has_surviving_heroes(const GameState *state) {
    return state && (state->available_characters || state->fighting_characters ||
                     state->sanitarium_characters || state->tavern_characters);
}
