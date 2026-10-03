CC = gcc
CFLAGS = -Wall -Wextra -Werror -std=c99 -g
CORE = character.c combat.c accessory.c save_load.c dungeon.c
DEPS = structures.h character.h combat.h save_load.h accessory.h dungeon.h
OBJECTS = $(CORE:.c=.o) main.o
MLV_CFLAGS = $(shell pkg-config --cflags MLV 2>/dev/null)
MLV_LIBS = $(shell pkg-config --libs MLV 2>/dev/null)

all: game

%.o: %.c $(DEPS)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

game: $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) -o $@ $(LDFLAGS)

gui: game-gui

game-gui: $(CORE) gui.c gui.h gui_main.c $(DEPS)
	pkg-config --exists MLV
	$(CC) $(CPPFLAGS) $(CFLAGS) $(MLV_CFLAGS) $(CORE) gui.c gui_main.c -o $@ $(LDFLAGS) $(MLV_LIBS)

tests/test-gameplay: tests/test_gameplay.c $(CORE) $(DEPS)
	$(CC) $(CPPFLAGS) $(CFLAGS) -I. $(CORE) tests/test_gameplay.c -o $@ $(LDFLAGS)

test: tests/test-gameplay
	./tests/test-gameplay

clean:
	rm -f $(OBJECTS) game game-gui tests/test-gameplay

.PHONY: all gui test clean
