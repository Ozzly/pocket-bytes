#ifndef KEY_H
#define KEY_H

#include "player.h"

#define KEY_WIDTH 16
#define KEY_HEIGHT 24
#define DOOR_WIDTH 24
#define DOOR_HEIGHT 32

typedef struct
{
    float x, y;
    int sprite_id;
    int carried_by; // -1 if not being carried
    int swap_buffer;
    bool door_unlocked;
} Key;

void keyPlayerTracking(Player *players, Key *key);

#endif