#include "button.h"
#include "tilemap.h"
#include <nf_lib.h>

void updateButtons(Button *buttons, Player *players) {
    for (int i = 0; i < current_button_count; i++) {
        Button *btn = &buttons[i];
        if (!btn->pressed) continue;
        switch (btn->type) {
            case BUTTON_KILL_PLAYERS:
                players[btn->triggered_by_id].is_dead = true;
                break;
            case BUTTON_MOVE_PLATFORM:
                break;
        }
    }
}

void checkPlayerButtonOverlap(Button *buttons, Player *players) {
    for (int i=0; i < current_player_count; i++) {
        Player *p = &players[i];

        for (int j=0; j < current_button_count; j++) {
            Button *btn = &buttons[j];
            if (overlaps(p->x, p->y, PLAYER_WIDTH, PLAYER_HEIGHT, btn->x, btn->y, 16, 8)) {
                btn->pressed = true;
                btn->triggered_by_id = p->sprite_id;
                NF_SpriteFrame(0, btn->sprite_id, 1);
            }
        }
    }
}