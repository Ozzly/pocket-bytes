#ifndef COLLISIONS_H
#define COLLISIONS_H

#include "player.h"
#include "box.h"
#include "platform.h"


void propagateMoveUp(GameObject object_on_top, int object_on_top_id, float displacement, Player *players, Box *boxes);
void resolvePlayerPlayerCollision(Player *players);
bool boxBlockedByPlayer(float new_box_x, float box_y, int box_sprite_id, Player *players, int pushing_player_index);
void resolvePlayerBoxCollision(Player *players, Box *boxes);
void resolvePlayerPlatformCollision(Player *players, Platform *platforms);

#endif