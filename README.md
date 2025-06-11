# DarkestDungeon

Jeu vidéo en C où des personnages s'affrontent en tour par tour jusqu'à atteindre le donjon final.

##  Fonctionnalités

- Gestion de personnages, accessoires, combats et sauvegardes
- Mécanique de tour par tour
- Système de fichiers pour les sauvegardes (`save_load.c`)
- Organisation modulaire du code avec fichiers `.c` / `.h`

##  Structure des fichiers

```
 darkestdungeon/
├── accessory.c / .h     → Gestion des objets
├── character.c / .h     → Gestion des personnages
├── combat.c / .h        → Logique de combat tour par tour
├── save_load.c / .h     → Sauvegarde et chargement de parties
├── structures.h         → Déclarations de structures
├── main.c               → Point d'entrée du programme
├── Makefile             → Compilation facilitée
├── Rapport_PLESSY.pdf   → Rapport du projet
```

##  Compilation

 compiler le projet avec :

```bash
make
```

Ou manuellement :

```bash
gcc main.c combat.c character.c accessory.c save_load.c -o darkestdungeon
```

##  Lancement

```bash
./darkestdungeon
```

> Assure-toi d’avoir les droits d’exécution ou fais `chmod +x darkestdungeon`.

##  Rapport

Le dossier contient :
- `Rapport_PLESSY_VINCENT_TP1.pdf`
- `projet_2024_2025.pdf`

##  Auteur

Vincent Plessy – Étudiant en L2 Informatique (2024–2025)
