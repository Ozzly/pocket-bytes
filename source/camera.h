#ifndef CAMERA_H
#define CAMERA_H

#include "player.h"


void playerClampToCamera(Player *players, float camera_x);
int getCameraPosition(Player *players, int current);

void updatePlayerPosition(Player *players, float camera_x);
void updateObjectPosition(int sprite_id, float x, float y, int sprite_width, float camera_x);

#endif