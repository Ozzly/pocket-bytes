#include "player.h"
#include "globals.h"
#include <nf_lib.h>
#include "tilemap.h"

#define COL_SPIKE 1
#define COYOTE_TIME 4
#define JUMP_STRENGTH -4.7f



void updatePlayerInput(Player *p, u16 keys, u16 keys_down) {
    if (keys & p->key_right) {
        p->vel_x += WALK_ACCELERATION;
    } else if (keys & p->key_left) {
        p->vel_x -= WALK_ACCELERATION;
    } else {
        p->vel_x *= 1 - (p->on_ground ? GROUND_FRICTION : AIR_FRICTION);
    }

    if (p->vel_x > MAX_WALK) p->vel_x = MAX_WALK;
    if (p->vel_x < -MAX_WALK) p->vel_x = -MAX_WALK;

    if (p->vel_x > 0) NF_HflipSprite(0, p->sprite_id, false);
    else if (p->vel_x < 0) NF_HflipSprite(0, p->sprite_id, true);

    if (keys_down & p->key_jump) p->jump_buffer = JUMP_BUFFER_TIME;
    if (p->jump_buffer > 0) p->jump_buffer--;
}


void updatePlayerPhysics(Player *p, const LevelConfig *config) {
    // Coyote frames to let player jump after leaving platform
    if (p->on_ground) {
        p->coyote_frames = COYOTE_TIME;
    } else if (p->coyote_frames > 0) {
        p->coyote_frames--;
    }

    // Gravity
    if (!p->on_ground) p->vel_y += GRAVITY;
    if (p->vel_y > MAX_FALL) p->vel_y = MAX_FALL;

    // Player wraps to top of screen if in void
    if (p->y > 192) {

        for (int i=0; i < config->void_count; i++) {
            Void voidZone = config->voids[i];
            if (p->x >= voidZone.left_x && p->x <= voidZone.right_x) {
                p->x = voidZone.respawn_x;
                p->y = 0;
            }
        }

        p->y = -PLAYER_HEIGHT;
        p->on_ground = false;
        p->coyote_frames = 0;
    }
}

void updatePlayerSprite(Player *p) {
    bool is_moving = p->vel_x > 0.05f || p->vel_x < -0.05f;
    
    p->sprite_frame_debounce++;
    if (is_moving && p->sprite_frame_debounce > 5) {
        p->sprite_frame++;
        p->sprite_frame_debounce = 0;
        if (p->sprite_frame > 5) p->sprite_frame = 2;
        NF_SpriteFrame(0, p->sprite_id, p->sprite_frame);
    } else if (!is_moving) {
        p->sprite_frame = 0;
        p->sprite_frame_debounce = 0;
        NF_SpriteFrame(0, p->sprite_id, 0);
    } 

    if (!is_moving && (keysHeld() & p->key_right)) {
        NF_HflipSprite(0, p->sprite_id, false);
        NF_SpriteFrame(0, p->sprite_id, 1);
    } else if (!is_moving && (keysHeld() & p->key_left)) {
        NF_HflipSprite(0, p->sprite_id, true);
        NF_SpriteFrame(0, p->sprite_id, 1);
    }   
}




void resolvePlayerHorizontalTileCollision(Player *p, float displacement_x) {
    int int_player_x = (int)p->x, int_player_y = (int)p->y;

    if (displacement_x > 0) { // Check right edge
        if (isSolid(int_player_x + PLAYER_WIDTH, int_player_y + 2) || isSolid(int_player_x + PLAYER_WIDTH, int_player_y + PLAYER_HEIGHT- 2)) { 
            p->x = (float)(((int_player_x + PLAYER_WIDTH) / TILE) * TILE - PLAYER_WIDTH) - 0.01f;
            p->vel_x = 0;
        }
    }
    if (displacement_x < 0) { // Check left edge
        if (isSolid(int_player_x, int_player_y + 2) || isSolid(int_player_x, int_player_y + PLAYER_HEIGHT -2)) {
            if (int_player_x < 0) { // Fix character going past screen border, then teleporting to positive tile 1
                p->x = 0;    
            } else {
                p->x = (float)((int_player_x / TILE + 1) * TILE);
            }
            p->vel_x = 0;
        }
    }

}

void resolvePlayerTileCollision(Player *p) {
    // Horizontal Collision
    p->x += p->vel_x;
    resolvePlayerHorizontalTileCollision(p, p->vel_x);

    // Vertical Collision
    p->y += p->vel_y;
    int int_player_x = (int)p->x, int_player_y = (int)p->y;
    p->on_ground = (p->standing_on != NOTHING);

    if (p->vel_y >= 0) { // falling or standing still, check feet
        if (isSolid(int_player_x + 2, int_player_y + PLAYER_HEIGHT) || isSolid(int_player_x + PLAYER_WIDTH -2, int_player_y + PLAYER_HEIGHT) || isSolid(int_player_x + PLAYER_WIDTH / 2, int_player_y + PLAYER_HEIGHT)) {
            p->y = (float)(((int_player_y + PLAYER_HEIGHT) / TILE) * TILE - PLAYER_HEIGHT);
            p->vel_y = 0;
            p->on_ground = true;
        }
    }
    if (p->vel_y < 0) { // rising, check head
        if (isSolid(int_player_x + 2, int_player_y) || isSolid(int_player_x + PLAYER_WIDTH -2, int_player_y)) {
            p->y = (float)((int_player_y / TILE + 1) * TILE);
            p->vel_y = 0;
        }
    }
}


void executeJumps(Player *players) {
    for (int i=0; i < current_player_count; i++) {
        Player *p = &players[i];
        if (p->jump_buffer > 0 && p->coyote_frames > 0 && !p->has_player_on_top) {
            p->vel_y = JUMP_STRENGTH;
            p->coyote_frames = 0;
            p->jump_buffer = 0;
        }
    }
}


void resolvePlayerSpikeCollision(Player *players) {
    for (int i=0; i < current_player_count; i++) {
        Player *p = &players[i];
        int int_player_x = (int)p->x, int_player_y = (int)p->y;
        if (NF_GetPoint(0, int_player_x + 2, int_player_y + PLAYER_HEIGHT - 1) == COL_SPIKE ||     // Foot left
            NF_GetPoint(0, int_player_x + PLAYER_WIDTH /2, int_player_y + PLAYER_HEIGHT - 1) == COL_SPIKE || // Foot middle
            NF_GetPoint(0, int_player_x + PLAYER_WIDTH - 2, int_player_y + PLAYER_HEIGHT - 1) == COL_SPIKE || // Foot right
            NF_GetPoint(0, int_player_x + 2, int_player_y) == COL_SPIKE || // Head left
            NF_GetPoint(0, int_player_x + PLAYER_WIDTH / 2, int_player_y) == COL_SPIKE || // Head middle
            NF_GetPoint(0, int_player_x + PLAYER_WIDTH - 2, int_player_y) == COL_SPIKE // Head right
        ) {
            p->is_dead = true; // track who died
        }
    }
}