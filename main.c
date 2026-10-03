#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "dungeon.h"
#include "character.h"
#include "combat.h"
#include "save_load.h"

static int choice(const char *prompt) {
    char line[128], extra; int value;
    printf("%s", prompt);
    if (!fgets(line, sizeof(line), stdin)) return -1;
    return sscanf(line, "%d %c", &value, &extra) == 1 && value >= 0 ? value : 0;
}
static Character *hero_at(Character *list, int index) {
    if (index < 1) return NULL;
    while (list && --index) list = list->next;
    return list;
}
static Accessory *item_at(Accessory *list, int index) {
    if (index < 1) return NULL;
    while (list && --index) list = list->next;
    return list;
}
static void show_heroes(const char *label, Character *list) {
    puts(label);
    for (int i = 1; list; list = list->next, ++i) { printf("%d. ", i); display_character(list); }
}
static void show_items(const char *label, Accessory *list) {
    puts(label);
    for (int i = 1; list; list = list->next, ++i)
        printf("%d. %s | ATT +%d DEF +%d PV +%d SOIN +%d STRESS -%d | %d or\n",
               i, list->name, list->attbonus, list->defbonus, list->HPbonus,
               list->restbonus, list->strred, list->price);
}
static void campaign(GameState *s) {
    for (;;) {
        printf("\n=== Camp : niveau %d/10, %d or ===\n", s->current_level, s->gold);
        show_heroes("Personnages disponibles", s->available_characters);
        if (s->current_level > 10) { puts("Donjon final vaincu !"); return; }
        if (!has_surviving_heroes(s)) { puts("Tous les personnages sont morts."); return; }
        int action = choice("1 Combat | 2 Equiper | 3 Retirer | 4 Sanitarium | 5 Taverne | 6 Rappeler | 7 Roulotte | 8 Sauvegarder | 9 Menu\n> ");
        if (action < 0 || action == 9) return;
        int ok = 0;
        if (action == 1) {
            Character *team[3]; int count = choice("Nombre de combattants : ");
            if (count < 0) return;
            if (count > 0 && count <= (s->current_level <= 5 ? 2 : 3)) {
                for (int i = 0; i < count; ++i) team[i] = hero_at(s->available_characters, choice("Numero du personnage : "));
                ok = select_fighters(s, team, count);
                if (ok) { Enemy enemy = DUNGEON_ENEMIES[s->current_level - 1]; start_combat(s, &enemy); }
            }
        } else if (action >= 2 && action <= 5) {
            Character *c = hero_at(s->available_characters, choice("Numero du personnage : "));
            if (action == 2) {
                show_items("Inventaire", s->available_accessories);
                Accessory *a = item_at(s->available_accessories, choice("Numero de l'accessoire : "));
                ok = equip_accessory(s, c, a, choice("Emplacement (1 ou 2) : "));
            } else if (action == 3) ok = unequip_accessory(s, c, choice("Emplacement (1 ou 2) : "));
            else ok = send_to_rest(s, c, action == 5);
        } else if (action == 6) {
            show_heroes("Sanitarium", s->sanitarium_characters);
            show_heroes("Taverne", s->tavern_characters);
            int place = choice("1 Sanitarium | 2 Taverne : ");
            if (place == 1 || place == 2) ok = recall_character(s, hero_at(place == 1 ? s->sanitarium_characters : s->tavern_characters,
                                                                       choice("Numero du personnage : ")));
        } else if (action == 7) {
            show_items("Roulotte", s->shop_accessories);
            ok = buy_accessory(s, item_at(s->shop_accessories, choice("Numero de l'accessoire : ")));
        } else if (action == 8) ok = save_game("savegame.dcd", s);
        puts(ok ? "Action effectuee." : "Action impossible ou choix invalide.");
    }
}
int main(void) {
    srand((unsigned int)time(NULL));
    GameState *s = NULL;
    for (;;) {
        int action = choice("\nDarkestcdungeon\n1 Nouvelle partie | 2 Charger savegame.dcd | 3 Quitter\n> ");
        if (action < 0 || action == 3) break;
        GameState *next = action == 1 ? create_new_game() : action == 2 ? load_game("savegame.dcd") : NULL;
        if (!next) { puts("Creation ou chargement impossible."); continue; }
        free_game_state(s); s = next; campaign(s);
    }
    free_game_state(s);
    return 0;
}
