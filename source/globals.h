#ifndef GLOBALS_H
#define GLOBALS_H

#define GRAVITY 0.35f
#define MAX_FALL 6.0f

#define SPRITE_BASE_KEY 0
#define SPRITE_BASE_DOOR 1
#define SPRITE_BASE_BOX 2 // 2-5
#define SPRITE_BASE_BUTTON 6 // 6-9
#define SPRITE_BASE_PLATFORM 10
#define SPRITE_BASE_PLAYER 20 // 20+
// #define SPRITE_BASE_COLOR_SELECTION 20

#define GFX_SLOT_PLAYER 0
#define GFX_SLOT_KEY 1
#define GFX_SLOT_DOOR 2
#define GFX_SLOT_BOX 3
#define GFX_SLOT_BUTTON 4
#define GFX_SLOT_PLATFORM 5
#define GFX_SLOT_RETURN 6

#define PAL_SLOT_KEY 0
#define PAL_SLOT_DOOR 1
#define PAL_SLOT_BOX 2
#define PAL_SLOT_BUTTON 3
#define PAL_SLOT_PLATFORM 4
#define PAL_SLOT_RETURN 5
#define PAL_SLOT_PLAYER_BASE 6


#define TILE 8

typedef enum
{
    NOTHING,
    PLAYER,
    BOX,
    PLATFORM,
} GameObject;

typedef enum
{
    STATE_TITLE,
    STATE_SINGLEPLAYER_COLOR_SELECT,
    STATE_MULTIPLAYER_JOIN,
    STATE_MULTIPLAYER_HOST,
    STATE_MULTIPLAYER_CLIENT,
    STATE_MULTIPLAYER_CLIENT_CONNECTED,
    STATE_CHARACTER_SELECT,
    STATE_LEVEL_SELECT,
    STATE_PLAYING,
    STATE_DYING,
} GameState;

extern GameState state;
extern int current_player_count;
extern int current_level_width;
extern int current_box_count;
extern int current_button_count;
extern int current_platform_count;

#endif