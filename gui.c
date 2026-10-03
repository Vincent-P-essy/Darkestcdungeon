#include <MLV/MLV_all.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include "gui.h"
#include "dungeon.h"
#include "character.h"
#include "combat.h"
#include "save_load.h"

#define W 1100
#define H 760
#define BG MLV_rgba(20,22,27,255)
#define PANEL MLV_rgba(31,34,40,255)
#define GOLD MLV_rgba(215,175,105,255)
#define TEXT MLV_rgba(225,225,217,255)
#define MUTED MLV_rgba(153,158,168,255)
#define RED MLV_rgba(173,67,72,255)
#define GREEN MLV_rgba(77,142,116,255)

enum { MENU, CAMP, EQUIPMENT, BATTLE, RESULT };
enum { NEW_GAME=1, LOAD_GAME, QUIT, SAVE_GAME, CAMP_TAB, GEAR_TAB, HERO,
       SELECT_HERO, SANITARIUM, TAVERN, RECALL, START, ITEM, SHOP, EQUIP1, EQUIP2,
       REMOVE1, REMOVE2, PREVIOUS, NEXT, ATTACK, DEFEND, HEAL, TARGET, RETREAT };
typedef struct { int x,y,w,h,id; void *value; } Button;
typedef struct {
    GameState *state;
    int page, running, team_count, turn, healing, offset;
    Character *selected, *team[3], *actor;
    Accessory *item;
    Enemy enemy;
    char log[4][160];
    MLV_Font *small, *normal, *title;
    Button buttons[64]; int button_count;
} Gui;

static void text(Gui *g, int x, int y, MLV_Color color, int size, const char *format, ...) {
    char buffer[256]; va_list args; va_start(args,format);
    vsnprintf(buffer,sizeof(buffer),format,args); va_end(args);
    MLV_draw_text_with_font(x,y,"%s",size==2?g->title:size==1?g->normal:g->small,color,buffer);
}
static void log_message(Gui *g, const char *message) {
    for (int i=0;i<3;++i) memcpy(g->log[i],g->log[i+1],sizeof(g->log[i]));
    snprintf(g->log[3],sizeof(g->log[3]),"%s",message);
}
static void button(Gui *g, int x,int y,int w,int h,const char *label,int id,void *value,int active) {
    MLV_draw_filled_rectangle(x,y,w,h,active?MLV_rgba(80,66,46,255):PANEL);
    MLV_draw_rectangle(x,y,w,h,active?GOLD:MLV_rgba(67,70,78,255));
    text(g,x+12,y+(h-18)/2,active?GOLD:TEXT,0,"%s",label);
    if (g->button_count<64) g->buttons[g->button_count++]=(Button){x,y,w,h,id,value};
}
static void bar(int x,int y,int w,int value,int max,MLV_Color color) {
    MLV_draw_filled_rectangle(x,y,w,7,MLV_rgba(51,53,61,255));
    if (max>0 && value>0) MLV_draw_filled_rectangle(x,y,w*(value>max?max:value)/max,7,color);
}
static int in_team(const Gui *g,const Character *c) {
    for(int i=0;i<g->team_count;++i) if(g->team[i]==c) return 1;
    return 0;
}
static int available(const Gui *g,const Character *c) {
    for(Character *p=g->state->available_characters;p;p=p->next) if(p==c) return 1;
    return 0;
}
static void portrait(int x,int y,ClassType type,int enemy) {
    MLV_Color cloak=enemy?RED:type==CLASS_VESTALE?MLV_rgba(139,131,159,255):
        type==CLASS_FURIE?MLV_rgba(151,82,72,255):MLV_rgba(98,129,127,255);
    int px[]={x-32,x+32,x+48,x-48},py[]={y+4,y+4,y+104,y+104};
    MLV_draw_filled_ellipse(x,y+114,53,10,MLV_rgba(10,11,14,255));
    MLV_draw_filled_polygon(px,py,4,cloak);
    MLV_draw_filled_circle(x,y-18,22,MLV_rgba(178,168,148,255));
    MLV_draw_filled_rectangle(x-23,y-40,46,14,cloak);
    MLV_draw_line(x-17,y-19,x+17,y-19,BG);
    MLV_draw_filled_rectangle(x-30,y+95,21,20,BG);
    MLV_draw_filled_rectangle(x+9,y+95,21,20,BG);
    if(type==CLASS_VESTALE && !enemy) {
        MLV_draw_line(x+49,y-38,x+49,y+110,GOLD);
        MLV_draw_circle(x+49,y-45,12,GOLD);
    } else {
        MLV_draw_filled_rectangle(x+40,y-22,5,95,MLV_rgba(178,182,191,255));
        MLV_draw_filled_rectangle(x+26,y+44,33,6,GOLD);
    }
    if(enemy) {
        MLV_draw_filled_circle(x-8,y-20,4,BG);
        MLV_draw_filled_circle(x+8,y-20,4,BG);
    }
}
static void header(Gui *g,const char *subtitle) {
    MLV_clear_window(BG); g->button_count=0;
    text(g,34,22,GOLD,2,"DARKESTCDUNGEON");
    text(g,35,65,MUTED,0,"%s",subtitle);
    MLV_draw_line(34,104,1066,104,MLV_rgba(76,66,52,255));
    if(g->state) text(g,830,35,TEXT,1,"NIVEAU %d/10   |   %d OR",
                     g->state->current_level>10?10:g->state->current_level,g->state->gold);
}
static void draw_menu(Gui *g) {
    header(g,"Campagne tactique en C / combats, equipement et gestion du stress");
    for(int i=0;i<7;++i) {
        MLV_draw_rectangle(245+i*24,160+i*18,610-i*48,365-i*25,MLV_rgba(48+i*3,43+i*2,43,255));
    }
    portrait(415,285,CLASS_FURIE,0); portrait(590,285,CLASS_VESTALE,0);
    text(g,324,440,GOLD,1,"Dix niveaux. Chaque expedition compte.");
    button(g,330,530,205,48,"Nouvelle partie",NEW_GAME,NULL,1);
    button(g,555,530,205,48,"Charger la partie",LOAD_GAME,NULL,0);
    button(g,445,600,205,44,"Quitter",QUIT,NULL,0);
    text(g,35,710,MUTED,0,"%s",g->log[3]);
}
static void draw_roster(Gui *g) {
    Character *lists[]={g->state->available_characters,g->state->sanitarium_characters,g->state->tavern_characters};
    const char *places[]={"Disponible","Sanitarium","Taverne"}; int row=0;
    text(g,35,125,GOLD,1,"PERSONNAGES");
    for(int j=0;j<3;++j) for(Character *c=lists[j];c;c=c->next,++row) {
        int y=165+row*77;
        button(g,35,y,330,67,"",HERO,c,g->selected==c);
        text(g,47,y+8,TEXT,1,"%s",c->name);
        text(g,195,y+10,in_team(g,c)?GOLD:MUTED,0,"%s",in_team(g,c)?"DANS L'EQUIPE":places[j]);
        text(g,47,y+36,MUTED,0,"%s  /  PV %d/%d  /  Stress %d",get_class_name(c->class.type),c->HP,character_max_hp(c),c->stress);
        bar(47,y+58,144,c->HP,character_max_hp(c),GREEN); bar(203,y+58,144,c->stress,100,RED);
    }
}
static void draw_details(Gui *g) {
    Character *c=g->selected;
    if(!c) { text(g,401,165,MUTED,1,"Selectionnez un personnage."); return; }
    text(g,401,160,GOLD,1,"%s / %s",c->name,get_class_name(c->class.type));
    text(g,401,196,TEXT,0,"Attaque %d    Defense %d    Soin %d    Combats %d",character_attack(c),
         c->class.def+(c->acc1?c->acc1->defbonus:0)+(c->acc2?c->acc2->defbonus:0),character_healing(c),c->nbcomb);
    text(g,401,223,MUTED,0,"1 : %s",c->acc1?c->acc1->name:"emplacement libre");
    text(g,401,248,MUTED,0,"2 : %s",c->acc2?c->acc2->name:"emplacement libre");
}
static void draw_camp(Gui *g) {
    header(g,"LE CAMP / Preparez l'equipe, gerez les soins et partez au combat");
    draw_roster(g); draw_details(g);
    button(g,400,292,208,41,"Ajouter / retirer equipe",SELECT_HERO,NULL,g->selected&&in_team(g,g->selected));
    button(g,626,292,208,41,"Equipement et roulotte",GEAR_TAB,NULL,0);
    button(g,400,350,145,38,"Sanitarium",SANITARIUM,NULL,0);
    button(g,558,350,145,38,"Taverne",TAVERN,NULL,0);
    button(g,716,350,145,38,"Rappeler",RECALL,NULL,0);
    text(g,400,402,MUTED,0,"Les soins se font pendant le combat des autres personnages.");
    text(g,400,427,MUTED,0,"Sanitarium : +7 PV   /   Taverne : -25 stress par combat");
    MLV_draw_filled_rectangle(400,475,665,138,PANEL);
    text(g,420,489,GOLD,1,"PROCHAINE EXPEDITION");
    if(g->state->current_level<=10) {
        const Enemy *e=&DUNGEON_ENEMIES[g->state->current_level-1];
        text(g,420,526,TEXT,0,"%s  /  PV %d  /  Attaque %d  /  Defense %d",e->name,e->HPenn,e->attenn,e->defenn);
        text(g,420,559,MUTED,0,"Equipe : %d/%d personnages  |  Victoire : 10 or et un accessoire",
             g->team_count,g->state->current_level<=5?2:3);
    } else text(g,420,532,GOLD,1,"Le donjon final est vaincu. Campagne terminee !");
    button(g,400,634,215,46,"Partir au combat",START,NULL,1);
    button(g,632,634,205,46,"Sauvegarder",SAVE_GAME,NULL,0);
    button(g,852,634,213,46,"Menu principal",QUIT,NULL,0);
    text(g,35,716,MUTED,0,"%s",g->log[3]);
}
static void draw_equipment(Gui *g) {
    header(g,"EQUIPEMENT / Deux accessoires par personnage, chaque objet est unique");
    draw_roster(g); draw_details(g);
    button(g,400,290,149,36,"Equiper en 1",EQUIP1,NULL,0);
    button(g,564,290,149,36,"Equiper en 2",EQUIP2,NULL,0);
    button(g,728,290,149,36,"Retirer le 1",REMOVE1,NULL,0);
    button(g,892,290,173,36,"Retirer le 2",REMOVE2,NULL,0);
    text(g,400,344,GOLD,1,"INVENTAIRE");
    Accessory *a=g->state->available_accessories;
    for(int i=0;a&&i<g->offset;++i) a=a->next;
    for(int row=0;a&&row<5;a=a->next,++row) {
        int y=378+row*45; char label[160];
        snprintf(label,sizeof(label),"%s | ATT +%d DEF +%d PV +%d SOIN +%d",a->name,a->attbonus,a->defbonus,a->HPbonus,a->restbonus);
        button(g,400,y,665,36,label,ITEM,a,g->item==a);
    }
    button(g,400,605,92,32,"Precedent",PREVIOUS,NULL,0);
    button(g,505,605,92,32,"Suivant",NEXT,NULL,0);
    button(g,852,605,213,32,"Retour au camp",CAMP_TAB,NULL,0);
    int col=0;
    for(a=g->state->shop_accessories;a&&col<3;a=a->next,++col) {
        char label[90]; snprintf(label,sizeof(label),"%s / %d",a->name,a->price);
        button(g,400+col*223,657,213,36,label,SHOP,a,0);
    }
    text(g,35,716,MUTED,0,"%s",g->log[3]);
}
static void draw_battle(Gui *g) {
    char subtitle[100]; snprintf(subtitle,sizeof(subtitle),"EXPEDITION / Tour %d / %s",g->turn,g->enemy.name);
    header(g,subtitle);
    MLV_draw_filled_rectangle(35,125,1030,285,MLV_rgba(28,27,32,255));
    for(int i=0;i<7;++i) {
        MLV_draw_rectangle(44+i*145,131,136,232,MLV_rgba(44,41,44,255));
        MLV_draw_line(44+i*145,210,180+i*145,210,MLV_rgba(44,41,44,255));
    }
    MLV_draw_filled_rectangle(35,370,1030,40,MLV_rgba(43,37,35,255));
    int i=0;
    for(Character *c=g->state->fighting_characters;c;c=c->next,++i) {
        int x=155+i*215; portrait(x,244,c->class.type,0);
        text(g,x-54,149,c==g->actor?GOLD:TEXT,1,"%s",c->name);
        if(c==g->actor) text(g,x-41,386,GOLD,0,g->healing?"CIBLE DU SOIN":"A VOTRE TOUR");
        int y=435+i*64; char label[128];
        snprintf(label,sizeof(label),"%s  |  PV %d/%d  |  Stress %d/100",c->name,c->HP,character_max_hp(c),c->stress);
        button(g,35,y,645,54,label,TARGET,c,c==g->actor);
        bar(50,y+42,300,c->HP,character_max_hp(c),GREEN);bar(368,y+42,295,c->stress,100,RED);
    }
    portrait(886,240,CLASS_CHASSEUR_DE_PRIMES,1);
    text(g,780,149,RED,1,"%s",g->enemy.name);
    bar(774,184,252,g->enemy.HPenn,DUNGEON_ENEMIES[g->enemy.level-1].HPenn,RED);
    text(g,713,434,GOLD,1,"JOURNAL DU COMBAT");
    for(int j=0;j<4;++j) text(g,713,474+j*30,j==3?TEXT:MUTED,0,"%.43s",g->log[j]);
    button(g,35,649,190,46,"Attaquer",ATTACK,NULL,1);
    button(g,242,649,190,46,"Se defendre",DEFEND,NULL,0);
    button(g,449,649,231,46,"Soigner un personnage",HEAL,NULL,g->healing);
    button(g,850,649,215,46,"Battre en retraite",RETREAT,NULL,0);
    text(g,35,717,MUTED,0,g->healing?"Cliquez sur le personnage a soigner. Recliquez sur Soigner pour annuler.":
         "Chaque personnage agit une fois, puis l'ennemi attaque. Stress 100 : action impossible.");
}
static Character *next_actor(Character *c) {
    while(c && (c->HP<=0 || c->stress>=100)) c=c->next;
    return c;
}
static void finish(Gui *g,int victory) {
    end_combat(g->state,victory);
    g->page=RESULT;g->actor=g->selected=NULL;g->team_count=0;g->item=NULL;g->healing=0;
    log_message(g,victory?"Victoire ! 10 or, un accessoire et retour au camp.":"Expedition interrompue. Les survivants rejoignent le camp.");
}
static void action(Gui *g,char command,Character *target) {
    char message[256];
    if(!perform_player_action(g->state,&g->enemy,g->actor,command,target,message,sizeof(message))) {
        log_message(g,"Action impossible : verifiez la cible et la capacite de soin.");return;
    }
    log_message(g,message);g->healing=0;
    if(g->enemy.HPenn<=0) { finish(g,1); return; }
    g->actor=next_actor(g->actor->next);
    if(!g->actor) {
        int hp=0,stress=0;
        for(Character *c=g->state->fighting_characters;c;c=c->next) {hp+=c->HP;stress+=c->stress;}
        perform_enemy_action(&g->enemy,g->state->fighting_characters);
        int after_hp=0,after_stress=0;
        for(Character *c=g->state->fighting_characters;c;c=c->next) {after_hp+=c->HP;after_stress+=c->stress;}
        snprintf(message,sizeof(message),"Riposte : %d PV perdus, +%d stress.",hp-after_hp,after_stress-stress);
        log_message(g,message);reset_defense(g->state->fighting_characters);remove_dead_fighters(g->state);
        g->actor=next_actor(g->state->fighting_characters);++g->turn;
        if(!g->actor) finish(g,0);
    }
}
static void handle(Gui *g,Button b) {
    int ok=0;
    if(b.id==NEW_GAME || b.id==LOAD_GAME) {
        GameState *s=b.id==NEW_GAME?create_new_game():load_game("savegame.dcd");
        if(!s) {log_message(g,"Partie indisponible ou sauvegarde invalide.");return;}
        free_game_state(g->state);g->state=s;g->page=CAMP;g->team_count=0;g->selected=s->available_characters;
        g->item=NULL;g->offset=0;memset(g->log,0,sizeof(g->log));return;
    }
    if(b.id==QUIT) { if(g->page==MENU) g->running=0;else g->page=MENU;return; }
    if(b.id==HERO) {g->selected=b.value;return;}
    if(b.id==CAMP_TAB || b.id==GEAR_TAB) {g->page=b.id==CAMP_TAB?CAMP:EQUIPMENT;g->item=NULL;return;}
    if(b.id==ITEM) {g->item=b.value;return;}
    if(b.id==SELECT_HERO) {
        for(int i=0;i<g->team_count;++i) if(g->team[i]==g->selected) {
            for(int j=i;j<g->team_count-1;++j) g->team[j]=g->team[j+1];
            --g->team_count;return;
        }
        if(g->selected && available(g,g->selected) && g->selected->stress<100 &&
           g->team_count<(g->state->current_level<=5?2:3)) {g->team[g->team_count++]=g->selected;return;}
        log_message(g,"Selection impossible : personnage indisponible ou equipe complete.");return;
    }
    if(b.id==START) {
        if(!select_fighters(g->state,g->team,g->team_count)) {log_message(g,"Choisissez au moins un personnage disponible.");return;}
        g->enemy=DUNGEON_ENEMIES[g->state->current_level-1];g->actor=next_actor(g->state->fighting_characters);
        g->page=BATTLE;g->turn=1;g->healing=0;memset(g->log,0,sizeof(g->log));log_message(g,"L'expedition commence.");return;
    }
    if(b.id==ATTACK || b.id==DEFEND) {action(g,b.id==ATTACK?'A':'D',NULL);return;}
    if(b.id==HEAL) {g->healing=!g->healing;return;}
    if(b.id==TARGET) {if(g->healing) action(g,'R',b.value);return;}
    if(b.id==RETREAT) {finish(g,0);return;}
    if(b.id==PREVIOUS || b.id==NEXT) {
        if(b.id==PREVIOUS) {g->offset-=5;if(g->offset<0)g->offset=0;}
        else {int n=0;for(Accessory *a=g->state->available_accessories;a;a=a->next)++n;if(g->offset+5<n)g->offset+=5;}
        return;
    }
    if(b.id==SAVE_GAME) ok=save_game("savegame.dcd",g->state);
    if(b.id==SANITARIUM || b.id==TAVERN) {
        ok=send_to_rest(g->state,g->selected,b.id==TAVERN);
        if(ok) {g->team_count=0;}
    }
    if(b.id==RECALL) ok=recall_character(g->state,g->selected);
    if(b.id==SHOP) {ok=buy_accessory(g->state,b.value);g->item=NULL;}
    if(b.id==EQUIP1 || b.id==EQUIP2) {ok=equip_accessory(g->state,g->selected,g->item,b.id==EQUIP1?1:2);if(ok)g->item=NULL;}
    if(b.id==REMOVE1 || b.id==REMOVE2) ok=unequip_accessory(g->state,g->selected,b.id==REMOVE1?1:2);
    log_message(g,ok?(b.id==SAVE_GAME?"Partie sauvegardee dans savegame.dcd.":"Action effectuee."):
                    "Action impossible : place occupee, personnage indisponible ou or insuffisant.");
}
static void close_window(void *data) { ((Gui *)data)->running=0; }
int run_gui(void) {
    Gui g;memset(&g,0,sizeof(g));g.running=1;
    const char *font=getenv("DUNGEON_FONT");
    if(!font) font="/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf";
    FILE *f=fopen(font,"rb");
    if(!f) {fprintf(stderr,"Police introuvable. Definissez DUNGEON_FONT vers un fichier TrueType.\n");return 1;}
    fclose(f);MLV_execute_at_exit(close_window,&g);MLV_create_window_with_default_font("Darkestcdungeon","Darkestcdungeon",W,H,font,15);
    g.small=MLV_load_font(font,14);g.normal=MLV_load_font(font,19);g.title=MLV_load_font(font,28);
    if(!g.small || !g.normal || !g.title) {MLV_free_window();return 1;}
    while(g.running) {
        if(g.page==MENU) draw_menu(&g);
        else if(g.page==CAMP) draw_camp(&g);
        else if(g.page==EQUIPMENT) draw_equipment(&g);
        else if(g.page==BATTLE) draw_battle(&g);
        else {
            header(&g,"RETOUR D'EXPEDITION");
            text(&g,160,255,GOLD,1,"%s",g.log[3]);
            text(&g,160,310,TEXT,1,"Niveau suivant : %d    Or disponible : %d",g.state->current_level,g.state->gold);
            button(&g,400,420,280,48,"Retour au camp",CAMP_TAB,NULL,1);
        }
        MLV_actualise_window();
        MLV_Keyboard_button key;MLV_Mouse_button mouse;MLV_Button_state state;int x,y;
        MLV_Event event=MLV_get_event(&key,NULL,NULL,NULL,NULL,&x,&y,&mouse,&state);
        if(event==MLV_MOUSE_BUTTON && mouse==MLV_BUTTON_LEFT && state==MLV_PRESSED) {
            for(int i=0;i<g.button_count;++i) {
                Button b=g.buttons[i];
                if(x>=b.x && x<b.x+b.w && y>=b.y && y<b.y+b.h) {handle(&g,b);break;}
            }
        }
        if(event==MLV_KEY && state==MLV_PRESSED && key==MLV_KEYBOARD_ESCAPE) {
            if(g.page==BATTLE) g.healing=0;else g.page=MENU;
        }
        MLV_wait_milliseconds(16);
    }
    free_game_state(g.state);MLV_free_font(g.small);MLV_free_font(g.normal);MLV_free_font(g.title);MLV_free_window();
    return 0;
}
