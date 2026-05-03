#ifndef GLOBALS_H
#define GLOBALS_H

#define GRAVITY 0.35f
#define MAX_FALL 6.0f

#define SPRITE_BASE_PLAYER 0 // 0-3
#define SPRITE_BASE_KEY 4
#define SPRITE_BASE_DOOR 5
#define SPRITE_BASE_BOX 6 // 6-9
#define SPRITE_BASE_BUTTON 10 // 10-13

#define GFX_SLOT_PLAYER 0
#define GFX_SLOT_KEY 1
#define GFX_SLOT_DOOR 2
#define GFX_SLOT_BOX 3
#define GFX_SLOT_BUTTON 4

#define PAL_SLOT_PLAYER_BASE 0
#define PAL_SLOT_KEY 4
#define PAL_SLOT_DOOR 5
#define PAL_SLOT_BOX 6
#define PAL_SLOT_BUTTON 7



#define TILE 8

typedef enum
{
    NOTHING,
    PLAYER,
    BOX,
} GameObject;

typedef enum
{
    STATE_PLAYING,
    STATE_DYING,
} GameState;

extern GameState state;
extern int current_player_count;
extern int current_level_width;
extern int current_box_count;
extern int current_button_count;

#endif