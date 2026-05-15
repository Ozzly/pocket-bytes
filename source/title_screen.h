#ifndef TITLE_SCREEN_H
#define TITLE_SCREEN_H
#include <stdbool.h>

typedef enum
{
    DEMO_WALK_RIGHT,
    DEMO_WALK_LEFT,
    DEMO_JUMP,
    DEMO_IDLE,
    DEMO_STAND_ON_PLAYER
} DemoAction;

typedef struct
{
    DemoAction action;
    int duration;
} DemoKeyFrame;



void createDemoPlayers();
void updateDemoPlayers();
void unloadTitleScreen();

#endif