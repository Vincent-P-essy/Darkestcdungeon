#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "save_load.h"
#include "dungeon.h"

static int valid_name(const char *s) {
    return s && *s && strlen(s) < 50 && !strpbrk(s, "\t\r\n");
}
static void write_item(FILE *f, const char *place, const Accessory *a) {
    fprintf(f, "ITEM\t%s\t%s\t%d\t%d\t%d\t%d\t%d\t%d\n", place,
            a->name, a->attbonus, a->defbonus, a->HPbonus, a->restbonus, a->strred, a->price);
}
static int write_characters(FILE *f, const char *place, const Character *c) {
    for (; c; c = c->next) {
        if (!valid_name(c->name) || (c->acc1 && !valid_name(c->acc1->name)) ||
            (c->acc2 && !valid_name(c->acc2->name))) return 0;
        fprintf(f, "CHAR\t%s\t%s\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\n", place,
                c->name, c->class.type, c->HP, c->class.HPmax, c->class.att,
                c->class.def, c->class.rest, c->stress, c->nbcomb);
        if (c->acc1) write_item(f, "EQ1", c->acc1);
        if (c->acc2) write_item(f, "EQ2", c->acc2);
    }
    return 1;
}
int save_game(const char *filename, GameState *s) {
    if (!filename || !*filename || !s || s->fighting_characters) return 0;
    char *tmp = malloc(strlen(filename) + 5);
    if (!tmp) return 0;
    sprintf(tmp, "%s.tmp", filename);
    FILE *f = fopen(tmp, "w");
    if (!f) { free(tmp); return 0; }
    fprintf(f, "DCD_SAVE 2\nLEVEL %d\nGOLD %d\n", s->current_level, s->gold);
    int ok = write_characters(f, "AVAILABLE", s->available_characters) &&
             write_characters(f, "SANITARIUM", s->sanitarium_characters) &&
             write_characters(f, "TAVERN", s->tavern_characters);
    const Accessory *lists[] = {s->available_accessories, s->shop_accessories};
    const char *places[] = {"INVENTORY", "SHOP"};
    for (int i = 0; i < 2; ++i) for (const Accessory *a = lists[i]; a; a = a->next) {
        if (!valid_name(a->name)) ok = 0;
        write_item(f, places[i], a);
    }
    fputs("END\n", f);
    if (ferror(f)) ok = 0;
    if (fclose(f)) ok = 0;
    if (ok && rename(tmp, filename)) ok = 0;
    if (!ok) remove(tmp);
    free(tmp);
    return ok;
}
static int number(const char *text, int *value) {
    char *end;
    errno = 0;
    long n = strtol(text, &end, 10);
    if (!*text || *end || errno || n < 0 || n > 1000000) return 0;
    *value = (int)n;
    return 1;
}
static void append_character(Character **list, Character *c) {
    while (*list) list = &(*list)->next;
    *list = c;
}
static void append_item(Accessory **list, Accessory *a) {
    while (*list) list = &(*list)->next;
    *list = a;
}
static int fields(char *line, char **out, int capacity) {
    int n = 0;
    for (;;) {
        if (n == capacity || !*line) return -1;
        out[n++] = line;
        char *tab = strchr(line, '\t');
        if (!tab) return n;
        *tab = '\0'; line = tab + 1;
    }
}
static int read_record(GameState *s, char *line, Character **owner) {
    char *p[12]; int count = fields(line, p, 12), v[8];
    if (count == 11 && !strcmp(p[0], "CHAR")) {
        for (int i = 0; i < 8; ++i) if (!number(p[i + 3], &v[i]) || v[i] > 10000) return 0;
        if (!valid_name(p[2]) || v[0] > 3 || !v[1] || !v[2] || v[6] > 100) return 0;
        Character **list;
        if (!strcmp(p[1], "AVAILABLE")) list = &s->available_characters;
        else if (!strcmp(p[1], "SANITARIUM")) list = &s->sanitarium_characters;
        else if (!strcmp(p[1], "TAVERN")) list = &s->tavern_characters;
        else return 0;
        Character *c = create_character(p[2], (ClassType)v[0]);
        if (!c) return 0;
        c->HP = v[1]; c->class.HPmax = v[2]; c->class.att = v[3];
        c->class.def = v[4]; c->class.rest = v[5]; c->stress = v[6]; c->nbcomb = v[7];
        append_character(list, c); *owner = c;
        return 1;
    }
    if (count == 9 && !strcmp(p[0], "ITEM")) {
        if (!valid_name(p[2])) return 0;
        for (int i = 0; i < 6; ++i) if (!number(p[i + 3], &v[i]) || v[i] > 10000) return 0;
        Accessory **list;
        if (!strcmp(p[1], "EQ1") && *owner && !(*owner)->acc1) list = &(*owner)->acc1;
        else if (!strcmp(p[1], "EQ2") && *owner && !(*owner)->acc2) list = &(*owner)->acc2;
        else if (!strcmp(p[1], "INVENTORY")) { list = &s->available_accessories; *owner = NULL; }
        else if (!strcmp(p[1], "SHOP") && v[5] > 0) { list = &s->shop_accessories; *owner = NULL; }
        else return 0;
        Accessory *a = create_accessory(p[2], v[0], v[1], v[2], v[3], v[4]);
        if (!a) return 0;
        a->price = v[5]; append_item(list, a);
        return 1;
    }
    return 0;
}
/* Les nombres de l'ancien format sont lus depuis la droite pour conserver les noms. */
static int legacy_numbers(char *name, int *v, int count) {
    for (int i = count - 1; i >= 0; --i) {
        char *space = strrchr(name, ' ');
        if (!space || !number(space + 1, &v[i]) || v[i] > 10000) return 0;
        *space = '\0';
    }
    return valid_name(name);
}
GameState *load_game(const char *filename) {
    if (!filename) return NULL;
    FILE *f = fopen(filename, "r");
    if (!f) return NULL;
    GameState *s = calloc(1, sizeof(*s));
    if (!s) { fclose(f); return NULL; }
    char line[512]; Character *owner = NULL;
    int version = 0, level = 0, gold = 0, ended = 0, ok = 1, records = 0;
    while (fgets(line, sizeof(line), f)) {
        if (++records > 1024 || !strchr(line, '\n') || ended) { ok = 0; break; }
        line[strcspn(line, "\r\n")] = '\0';
        if (records == 1 && !strcmp(line, "DCD_SAVE 2")) { version = 2; continue; }
        if (!strncmp(line, "LEVEL ", 6) && !level) {
            ok = number(line + 6, &s->current_level) && s->current_level >= 1 && s->current_level <= 11; level = 1;
        } else if (!strncmp(line, "GOLD ", 5) && !gold) { ok = number(line + 5, &s->gold); gold = 1; }
        else if (version == 2 && !strcmp(line, "END")) ended = 1;
        else if (version == 2) ok = read_record(s, line, &owner);
        else if (!strncmp(line, "AVAILABLE_CHAR ", 15)) {
            int v[7]; char *name = line + 15;
            ok = legacy_numbers(name, v, 7) && v[0] <= 3 && v[1] > 0 && v[5] <= 100;
            if (ok) {
                Character *c = create_character(name, (ClassType)v[0]);
                if (!c) ok = 0;
                else { c->HP = v[1]; c->class.att = v[2]; c->class.def = v[3];
                    c->class.rest = v[4]; c->stress = v[5]; c->nbcomb = v[6];
                    append_character(&s->available_characters, c); }
            }
        } else if (!strncmp(line, "ACCESSORY ", 10)) {
            int v[5]; char *name = line + 10;
            ok = legacy_numbers(name, v, 5);
            if (ok) {
                Accessory *a = create_accessory(name, v[0], v[1], v[2], v[3], v[4]);
                if (!a) ok = 0; else append_item(&s->available_accessories, a);
            }
        } else ok = 0;
        if (!ok) break;
    }
    if (ferror(f) || !level || !gold || (version == 2 && !ended) ||
        list_size(s->sanitarium_characters) > 2 || list_size(s->tavern_characters) > 2) ok = 0;
    Character *lists[] = {s->available_characters, s->sanitarium_characters, s->tavern_characters};
    for (int i = 0; i < 3; ++i) for (Character *c = lists[i]; c; c = c->next)
        if (c->HP > character_max_hp(c)) ok = 0;
    fclose(f);
    if (!ok) { free_game_state(s); return NULL; }
    return s;
}
void free_game_state(GameState *s) {
    if (!s) return;
    free_character_list(s->available_characters);
    free_character_list(s->sanitarium_characters);
    free_character_list(s->tavern_characters);
    free_character_list(s->fighting_characters);
    free_accessory_list(s->available_accessories);
    free_accessory_list(s->shop_accessories);
    free(s);
}
