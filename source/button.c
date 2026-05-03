#include "button.h"
#include "tilemap.h"
#include <nf_lib.h>

void updateButtons(Button *buttons, Player *players, Platform *platforms) {
    for (int i = 0; i < current_button_count; i++) {
        if (buttons[i].requires_hold && buttons[i].pressed) {
            buttons[i].hold_timer = buttons[i].hold_timer - 1;
            if (buttons[i].hold_timer <= 0) {
                buttons[i].pressed = false;
                if (buttons[i].type == BUTTON_MOVE_PLATFORM) platforms[buttons[i].target_id].active = false;
                NF_SpriteFrame(0, buttons[i].sprite_id, 0);
            }
        }


        Button *btn = &buttons[i];
        if (!btn->pressed) continue;
        switch (btn->type) {
            case BUTTON_KILL_PLAYERS:
                players[btn->triggered_by_id].is_dead = true;
                break;
            case BUTTON_MOVE_PLATFORM:
                for (int i = 0; i < current_platform_count; i++) {
                    if (btn->target_id == platforms[i].platform_id) platforms[i].active = true; 
                }
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
                btn->hold_timer = 10;
                btn->triggered_by_id = p->sprite_id;
                NF_SpriteFrame(0, btn->sprite_id, 1);
            }
        }
    }
}