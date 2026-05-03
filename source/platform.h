#ifndef PLATFORM_H
#define PLATFORM_H

#include "nds.h"
#include "player.h"
#include "box.h"

#define MAX_PLATFORM_SEGMENTS 4

typedef struct
{
    float x, y;
    float target_x;
    float start_x;
    float speed;
    int width;
    int height;
    bool active;
    int sprite_id;
    int platform_id;
    GameObject object_on_top;
    int object_on_top_id;

} Platform;


void updatePlatforms(Platform *platforms, Player *players, Box *boxes);

#endif