#ifndef LEVEL_TYPES_H
#define LEVEL_TYPES_H

#include <stdbool.h>

#define MAX_PLAYERS 4
#define MAX_VOIDS 4
#define MAX_BOXES 4
#define MAX_BUTTONS 4
#define MAX_PLATFORMS 4

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

typedef enum
{
    BUTTON_MOVE_PLATFORM,
    BUTTON_KILL_PLAYERS,
} ButtonType;


typedef struct {
    float x,y;
    ButtonType type;
    int target_id;
    int platform_x, platform_y;
    bool requires_hold;
} ButtonSpawn;

typedef struct {
    int x, y;
    int target_x;
    float speed;
    int width;
    int height;
    int platfrom_id;
} PlatformSpawn;

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
    ButtonSpawn buttons[MAX_BUTTONS];
    int button_count;
    PlatformSpawn platforms[MAX_PLATFORMS];
    int platform_count;
} LevelConfig;

extern const LevelConfig LEVELS[];

#endif