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

typedef struct
{
    float x, y;
    float vel_x, vel_y;
    int sprite_id;
    int palette_id;
    int keyframe;
    int keyframe_timer;
    const DemoKeyFrame *script;
    int script_length;
    bool on_ground;
    int sprite_frame;
    int sprite_frame_debounce;
} DemoPlayer;

void createDemoPlayers();
void updateDemoPlayers();
void unloadTitleScreen();

#endif