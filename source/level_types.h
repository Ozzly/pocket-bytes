#ifndef LEVEL_TYPES_H
#define LEVEL_TYPES_H

#define MAX_PLAYERS 4
#define MAX_VOIDS 4
#define MAX_BOXES 4

typedef struct
{
    int left_x;
    int right_x;
    int respawn_x;
} Void;

typedef struct
{
    int spawn_x, spawn_y;
    int push_required;
} BoxSpawn;

typedef struct
{
    const char *bg_name;
    const char *col_name;
    int width;
    float spawn_x[MAX_PLAYERS];
    float spawn_y[MAX_PLAYERS];
    int key_spawn_x;
    int key_spawn_y;
    int door_x;
    int door_y;
    Void voids[MAX_VOIDS];
    int void_count;
    BoxSpawn boxes[MAX_BOXES];
    int box_count;
} LevelConfig;

extern const LevelConfig LEVELS[];

#endif