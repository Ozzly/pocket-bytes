#ifndef PLAYER_H
#define PLAYER_H


#include "globals.h"
#include <nds.h>
#include "level_types.h"

#define WALK_ACCELERATION 0.5f
#define MAX_WALK 1.5f
#define GROUND_FRICTION 0.8f // 1 is max friction
#define AIR_FRICTION 0.7f
#define JUMP_BUFFER_TIME 3

#define PLAYER_HEIGHT 24
#define PLAYER_WIDTH 22

typedef struct
{
    float x, y;
    float vel_x, vel_y;
    bool on_ground;
    int coyote_frames;
    int jump_buffer;
    int sprite_id;
    int palette_id;
    u8 sprite_frame;
    u8 sprite_frame_debounce;
    u16 key_left, key_right, key_jump;

    GameObject standing_on;
    int standing_on_id;
    GameObject object_on_top;
    int object_on_top_id;
    bool has_player_on_top;
    bool is_dead;
    bool in_door;
} Player;

void updatePlayerInput(Player *p, u16 keys, u16 keys_down);
void updatePlayerPhysics(Player *p, const LevelConfig *config);
void updatePlayerSprite(Player *p);
void resolvePlayerHorizontalTileCollision(Player *p, float displacement_x);
void resolvePlayerTileCollision(Player *p);
void executeJumps(Player *players);
void resolvePlayerSpikeCollision(Player *players);

#endif