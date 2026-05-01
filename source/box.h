#ifndef BOX_H
#define BOX_H

#include "globals.h"
#include "level_types.h"
#include <nds.h>

#define BOX_WIDTH 24
#define BOX_HEIGHT 24

typedef struct
{
    float x, y;
    float vel_x, vel_y;
    int push_required;
    bool on_ground;
    int sprite_id;
    GameObject object_on_top;
    int object_on_top_id;
    GameObject standing_on;
    int standing_on_id;
} Box;


void updateBoxPhysics(Box *b, LevelConfig const *config);
void resolveBoxHorizontalTileCollision(Box *b, float displacement_x);
void resolveBoxTileCollision(Box *b);

#endif