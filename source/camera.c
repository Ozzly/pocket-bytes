#include "camera.h"
#include <nf_lib.h>


#define CAMERA_OFFSET 116 // Half of screen (128) - sprite center offset (12)


void playerClampToCamera(Player *players, float camera_x) {
    for (int i=0; i < current_player_count; i++) {
        Player *p = &players[i];
        if (p->x < camera_x) {
            p->x = camera_x;
            p->vel_x = 0;
        }
        if (p->x + PLAYER_WIDTH > camera_x + 256) {
            p->x = (float)(camera_x + 256 - PLAYER_WIDTH);
            p->vel_x = 0;
        }
    }
}


int getCameraPosition(Player *players, int current) {
    float lead_x;
    float tail_x;
    for (int i=0; i < current_player_count; i++) {
        if ( i == 0 || players[i].x > lead_x) lead_x = players[i].x;
        if ( i == 0 || players[i].x < tail_x) tail_x = players[i].x;
    }
    float mid_x = (tail_x + lead_x) / 2.0f;
    int desired = (int)mid_x - CAMERA_OFFSET;
    // Stop camera scrolling past level boundaries
    if (desired < 0) desired = 0;
    if (desired > current_level_width - 256) desired = current_level_width - 256;
    // Stop camera scrolling if player touching left of screen
    for (int i=0; i < current_player_count; i++) {
        if (desired > players[i].x) {
            desired = players[i].x;
        }
    }

    if (desired - current > 2) desired = current + 2;
    if (desired - current < -2) desired = current - 2;
    return desired;
} 


void updatePlayerPosition(Player *players, float camera_x) {
    for (int i =0; i < current_player_count; i++) {
        Player *p = &players[i];
        if (p->in_door) { NF_MoveSprite(0, p->sprite_id, 0, 192); }
        else NF_MoveSprite(0, p->sprite_id, p->x - camera_x - 4, p->y -4);
    }
}

void updateObjectPosition(int sprite_id, float x, float y, int sprite_width, float camera_x) {
    float screen_x = x - camera_x;
    if (screen_x + sprite_width < 0 || screen_x >= 256) {
        NF_MoveSprite(0, sprite_id, 0, 192); // hide below screen to prevent OAM wrapping sprite
    } else {
        NF_MoveSprite(0, sprite_id, screen_x, y);
    }
}