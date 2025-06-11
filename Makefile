CC = gcc
CFLAGS = -Wall -Wextra -Werror -std=c99 -g
DEPS = structures.h character.h combat.h save_load.h accessory.h
OBJECTS = character.o combat.o save_load.o main.o accessory.o

# Règle par défaut
all: game

# Dépendances des fichiers headers
%.o: %.c $(DEPS)
	$(CC) $(CFLAGS) -c $< -o $@

# Création de l'exécutable
game: $(OBJECTS)
	$(CC) $(OBJECTS) -o $@ $(LDFLAGS)

# Nettoyage
clean:
	rm -f $(OBJECTS) game
	rm -f *~ 

# Pour éviter les conflits avec des fichiers du même nom
.PHONY: clean all