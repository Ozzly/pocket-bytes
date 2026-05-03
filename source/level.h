#ifndef LEVEL_H
#define LEVEL_H

#include "box.h"
#include "key.h"
#include "player.h"
#include "level_types.h"
#include "button.h"
#include "platform.h"






void loadLevel(const LevelConfig *config, Key *key);
void resetLevel(Player *players, float *camera_x, const LevelConfig *config, Key *key, Box *boxes, Button *buttons, Platform *platforms);
void unloadLevel(void);
bool isLevelComplete(Player *players);
void checkDoor(Player *players, Key *key, const LevelConfig *config);

#endif