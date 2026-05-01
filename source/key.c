#include "key.h"
#include "tilemap.h"





void keyPlayerTracking(Player *players, Key *key) {
    for (int i=0; i < current_player_count; i++) {
        Player *p = &players[i];
        if (overlaps(p->x, p->y, PLAYER_WIDTH, PLAYER_HEIGHT, key->x, key->y, KEY_WIDTH, KEY_HEIGHT) && key->carried_by != p->sprite_id && key->swap_buffer == 0 ) {
            key->carried_by = p->sprite_id;
            key->swap_buffer = 20;
        }
    }

    if (key->swap_buffer > 0) key->swap_buffer--;
    if (key->carried_by != -1) {
        Player *carrier = &players[key->carried_by];
        float target_x = carrier->x - KEY_WIDTH / 2.0f;
        float target_y = carrier->y - KEY_HEIGHT / 2.0f;
        key->x += (target_x - key->x) * 0.10f;
        key->y += (target_y - key->y) * 0.10f;
    }
}