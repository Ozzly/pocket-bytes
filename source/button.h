#ifndef BUTTON_H
#define BUTTON_H


#include <nds.h>
#include "player.h"
#include "platform.h"



typedef struct
{
    float x, y;
    int sprite_id;
    bool pressed;
    int triggered_by_id;
    ButtonType type;
    bool requires_hold;
    int hold_timer;

    int target_id;
    int platform_x, platform_y;
} Button;


void updateButtons(Button *buttons, Player *players, Platform *platforms);
void checkPlayerButtonOverlap(Button *buttons, Player *players);


#endif