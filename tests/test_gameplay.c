#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "dungeon.h"
#include "combat.h"
#include "character.h"
#include "save_load.h"

static void malformed(const char *text) {
    FILE *f = fopen("test-save.dcd", "w"); assert(f);
    assert(fputs(text, f) >= 0); assert(!fclose(f));
    assert(load_game("test-save.dcd") == NULL);
}
int main(void) {
    srand(4);
    assert(!create_character("Invalid", (ClassType)99));
    free_game_state(NULL);
    GameState *s = create_new_game(); assert(s);
    Character *junia = s->available_characters, *boudicca = junia->next;
    Accessory *calice = s->available_accessories;
    assert(equip_accessory(s, boudicca, calice, 1));
    assert(boudicca->HP == 25 && character_max_hp(boudicca) == 25);
    assert(!equip_accessory(s, junia, calice, 1));
    boudicca->HP = 2;
    assert(unequip_accessory(s, boudicca, 1) && boudicca->HP == 1);
    apply_healing(boudicca, 500); assert(boudicca->HP == 20);
    Character *duplicates[] = {junia, junia};
    assert(!select_fighters(s, duplicates, 2) && list_size(s->available_characters) == 2);
    junia->stress = 100;
    Character *team[] = {junia, boudicca};
    assert(!select_fighters(s, team, 2)); junia->stress = 0;
    assert(select_fighters(s, team, 2));
    assert(!save_game("test-save.dcd", s));
    Enemy enemy = DUNGEON_ENEMIES[0]; char message[256];
    assert(perform_player_action(s, &enemy, boudicca, 'D', NULL, message, sizeof(message)));
    assert(boudicca->is_defending); reset_defense(s->fighting_characters);
    assert(!boudicca->is_defending);
    assert(!perform_player_action(s, &enemy, boudicca, 'R', boudicca, message, sizeof(message)));
    boudicca->HP = 5;
    assert(perform_player_action(s, &enemy, junia, 'R', boudicca, message, sizeof(message)));
    assert(boudicca->HP == 15);
    for (int i = 0; i < 100; ++i) assert(calculate_damage(4, 3) >= 1);
    end_combat(s, 1);
    assert(s->gold == 10 && s->current_level == 2 && !s->fighting_characters);
    assert(junia->nbcomb == 1 && boudicca->nbcomb == 1);
    assert(send_to_rest(s, junia, 1)); junia->stress = 75;
    boudicca->HP = 10; assert(send_to_rest(s, boudicca, 0));
    assert(!select_fighters(s, team, 2));
    /* Un autre combattant permet de verifier les soins pendant un vrai combat. */
    Character *reserve = create_character("Reserve 2", CLASS_MAITRE_CHIEN); assert(reserve);
    s->available_characters = add_character_to_list(s->available_characters, reserve);
    Character *solo[] = {reserve}; assert(select_fighters(s, solo, 1));
    end_combat(s, 1);
    assert(junia->stress == 50 && boudicca->HP == 17 && s->current_level == 3);
    assert(list_size(s->available_characters) == 2); /* Recrue du niveau 2. */
    assert(buy_accessory(s, s->shop_accessories) == 0); /* 35 > 20 or. */
    Accessory *cheap = s->shop_accessories->next->next;
    assert(buy_accessory(s, cheap) && s->gold == 13);
    assert(equip_accessory(s, reserve, s->available_accessories, 1));
    assert(save_game("test-save.dcd", s));
    GameState *loaded = load_game("test-save.dcd"); assert(loaded);
    assert(loaded->current_level == 3 && loaded->gold == 13);
    assert(loaded->tavern_characters->stress == 50 && loaded->sanitarium_characters->HP == 17);
    Character *copy = loaded->available_characters->next;
    assert(!strcmp(copy->name, "Reserve 2") && copy->acc1 && copy->HP == reserve->HP);
    assert(!strcmp(copy->acc1->name, reserve->acc1->name));
    assert(loaded->shop_accessories->price == 35);
    assert(recall_character(loaded, loaded->tavern_characters));
    free_game_state(loaded); free_game_state(s);
    malformed("DCD_SAVE 2\nLEVEL 1\nGOLD 0\n");
    malformed("DCD_SAVE 2\nLEVEL 1\nGOLD 0\nCHAR\tAVAILABLE\tBroken\t99\t20\t20\t13\t0\t0\t0\t0\nEND\n");
    malformed("DCD_SAVE 2\nLEVEL 12\nGOLD 0\nEND\n");
    FILE *f = fopen("test-save.dcd", "w"); assert(f);
    fputs("LEVEL 1\nGOLD 7\nAVAILABLE_CHAR Hero 2 0 20 13 0 0 0 0\nACCESSORY Relique 12 2 1 0 0 0\n", f);
    assert(!fclose(f)); loaded = load_game("test-save.dcd"); assert(loaded);
    assert(!strcmp(loaded->available_characters->name, "Hero 2"));
    assert(!strcmp(loaded->available_accessories->name, "Relique 12"));
    free_game_state(loaded);
    s = create_new_game(); assert(s); junia = s->available_characters;
    Character *dead[] = {junia}; assert(select_fighters(s, dead, 1));
    assert(equip_accessory(s, junia, s->available_accessories, 1));
    junia->HP = 0; apply_healing(junia, 100); assert(!junia->HP);
    remove_dead_fighters(s); assert(!s->fighting_characters);
    assert(list_size(s->available_characters) == 1);
    free_game_state(s); assert(!remove("test-save.dcd"));
    puts("Gameplay, ownership, rest, rewards, save/load and malformed saves: OK");
    return 0;
}
