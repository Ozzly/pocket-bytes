#ifndef TITLE_SCREEN_H
#define TITLE_SCREEN_H
#include <stdbool.h>

typedef enum
{
    DEMO_WALK_RIGHT,
    DEMO_WALK_LEFT,
    DEMO_JUMP,
    DEMO_JUMP_RIGHT,
    DEMO_JUMP_LEFT,
    DEMO_IDLE,
    DEMO_STAND_ON_PLAYER,
} DemoAction;

typedef struct
{
    DemoAction action;
    int duration;
} DemoKeyFrame;



void createDemoPlayers();
void updateDemoPlayers();
// void initDemoKey();
// void updateDemoKey(); 
void unloadTitleScreen();

#endif