

#include "globals.h"
#include "level.h"
#include <nf_lib.h>
#include "tilemap.h"


const LevelConfig LEVELS[] = {
    {
        .bg_name = "bg/level1",
        .col_name = "collision/level1_col",
        .width = 1024,
        .spawn_x = { 40.0f, 60.0f, 80.0f },
        .spawn_y = { 150.0f, 150.0f, 150.0f },
        .key_spawn_x = 600,
        .key_spawn_y = 72,
        .door_x = 780,
        .door_y = 145,
        .void_count = 2,
        .voids = {
            {
                .left_x = 210,
                .right_x = 247,
                .respawn_x = 168,
            },
            {
                .left_x = 330,
                .right_x = 407,
                .respawn_x = 290,
            }
        },
        

    },
    {
        .bg_name = "bg/level2",
        .col_name = "collision/level2_col",
        .width = 1024,
        .spawn_x = { 40.0f, 60.0f, 80.0f },
        .spawn_y = { 150.0f, 150.0f, 150.0f },
        .key_spawn_x = 600,
        .key_spawn_y = 72,
        .door_x = 780,
        .door_y = 145,
        .void_count = 1,
        .voids = {
            {
                .left_x = 460,
                .right_x = 535,
                .respawn_x = 416,
            },
        },
        .box_count = 1,
        .boxes = {
            {
                .spawn_x = 224,
                .spawn_y = 140,
                .push_required = 2,
            },
        },
    },
};


void loadLevel(const LevelConfig *config, Key *key) {
    current_level_width = config->width;
    NF_LoadTiledBg(config->bg_name, "level", config->width, 256);
    NF_CreateTiledBg(0, 3, "level");
    NF_LoadCollisionBg(config->col_name, 0, config->width, 256);

    NF_LoadSpriteGfx("sprite/key", 1, 16, 32);
    NF_VramSpriteGfx(0, 1, 1, false);
    NF_LoadSpritePal("sprite/key", 4);
    NF_VramSpritePal(0, 4, 4);
    NF_CreateSprite(0, 4, 1, 4, 0, 0);
    key->sprite_id = 4;

    NF_LoadSpriteGfx("sprite/door", 2, 32, 32);
    NF_VramSpriteGfx(0, 2, 2, false);
    NF_LoadSpritePal("sprite/door", 5);
    NF_VramSpritePal(0, 5, 5);
    NF_CreateSprite(0, 5, 2, 5, 0, 0);

    NF_LoadSpriteGfx("sprite/box", 3, 32, 32);
    NF_VramSpriteGfx(0, 3, 3, false);
    NF_LoadSpritePal("sprite/box", 6);
    NF_VramSpritePal(0, 6, 6);
    NF_CreateSprite(0, 6, 3, 6, config->boxes[0].spawn_x, config->boxes[0].spawn_y);

    current_box_count = config->box_count;
}

void resetLevel(Player *players, float *camera_x, const LevelConfig *config, Key *key, Box *boxes) {
    for (int i=0; i < current_player_count; i++) {
        players[i].x = config->spawn_x[i];
        players[i].y = config->spawn_y[i];
        players[i].vel_x = 0.0f;
        players[i].vel_y = 0.0f;
        players[i].on_ground = false;
        players[i].coyote_frames = 0;
        players[i].jump_buffer = 0;
        players[i].standing_on = NOTHING;
        players[i].standing_on_id = -1;
        players[i].object_on_top = NOTHING;
        players[i].object_on_top_id = -1;
        players[i].has_player_on_top = false;
        players[i].is_dead = false;
        players[i].in_door = false;
    }
    *camera_x = 0;

    key->x = config->key_spawn_x;
    key->y = config->key_spawn_y;
    key->carried_by = -1;
    key->swap_buffer = 0;
    key->door_unlocked = false;

    NF_SpriteFrame(0, 5, 0);

    for (int i=0; i < config->box_count; i++) {
        boxes[i].x = config->boxes[i].spawn_x;
        boxes[i].y = config->boxes[i].spawn_y;
        boxes[i].push_required = config->boxes[i].push_required;
        boxes[i].standing_on = NOTHING;
        boxes[i].standing_on_id = -1;
        boxes[i].sprite_id = 6 + i;
        boxes[i].object_on_top = NOTHING;
        boxes[i].object_on_top_id = -1;
        NF_SpriteFrame(0, 6, boxes[i].push_required);
    }
} 


void unloadLevel(void) {
    NF_DeleteTiledBg(0, 3);
    NF_UnloadTiledBg("level");
    NF_UnloadCollisionBg(0);

    NF_DeleteSprite(0, 4); // key
    NF_FreeSpriteGfx(0, 1);
    NF_UnloadSpriteGfx(1);
    NF_UnloadSpritePal(4);

    NF_DeleteSprite(0, 5);
    NF_FreeSpriteGfx(0, 2);
    NF_UnloadSpriteGfx(2);
    NF_UnloadSpritePal(5);

    NF_DeleteSprite(0, 6);
    NF_FreeSpriteGfx(0, 3);
    NF_UnloadSpriteGfx(3);
    NF_UnloadSpritePal(6);
}


bool isLevelComplete(Player *players) {
    int players_in_door = 0;
    for (int i = 0; i < current_player_count; i++) {
        if (players[i].in_door) {
            players_in_door++;
        }
    }
    if (players_in_door == current_player_count) return true;
    else return false;
}


void checkDoor(Player *players, Key *key, const LevelConfig *config) {
    int door_x = config->door_x;
    int door_y = config->door_y;

    // Door not unlocked yet
    if (!key->door_unlocked && key->carried_by != -1) {
        Player *carrier = &players[key->carried_by];
        // Check player overlaps door & presses up
        if (carrier->jump_buffer > 0 && overlaps(carrier->x, carrier->y, PLAYER_WIDTH, PLAYER_HEIGHT, door_x + 10, door_y, DOOR_WIDTH - 20, DOOR_HEIGHT) && carrier->standing_on == NOTHING) {
            key->door_unlocked = true;
            carrier->in_door = true;
            carrier->jump_buffer = 0;
            NF_SpriteFrame(0, 5, 1);
        }
    }

    if (key->door_unlocked) {
        for (int i=0; i < current_player_count; i++) {
            if (!players[i].in_door && players[i].jump_buffer > 0 && overlaps(players[i].x, players[i].y, PLAYER_WIDTH, PLAYER_HEIGHT, door_x + 10, door_y, DOOR_WIDTH - 20, DOOR_HEIGHT) && players[i].standing_on == NOTHING) {
                players[i].in_door = true;
                players[i].jump_buffer = 0;
            }
        }
    }
}