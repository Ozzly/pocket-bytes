#ifndef GLOBALS_H
#define GLOBALS_H

#define GRAVITY 0.35f
#define MAX_FALL 6.0f


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

#endif