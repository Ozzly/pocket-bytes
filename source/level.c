

#include "globals.h"
#include "level.h"
#include <nf_lib.h>
#include "tilemap.h"


const LevelConfig LEVELS[] = {
    {
        .bg_name = "bg/level1",
        .col_name = "collision/level1_col",
        .width = 1024,
        .spawn_x = { 60.0f, 80.0f, 100.0f },
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
        .button_count = 1,
        .buttons = {
            // {
            //     .x = 100,
            //     .y = 120,
            //     .type = BUTTON_KILL_PLAYERS,
            // }, 
            {
                .x = 440,
                .y = 170,
                .type = BUTTON_MOVE_PLATFORM,
                .target_id = 0,
                .requires_hold = true,
            }
        },
        .platform_count = 1,
        .platforms = {
            {
                .x = 404,
                .y = 177,
                .width = 32,
                .height = 8,
                .speed = 1.0f,
                .target_x = 355,
                .platfrom_id = 0,

            }
        }
        

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
                .right_x = 570,
                .respawn_x = 416,
            },
        },
        .box_count = 2,
        .boxes = {
            {
                .spawn_x = 224,
                .spawn_y = 140,
                .push_required = 2,
            },
            {
                .spawn_x = 368,
                .spawn_y = 104,
                .push_required = 1,
            }
        },
        .button_count = 1,
        .buttons = {
            {
                .x = 608,
                .y = 122,
                .type = BUTTON_MOVE_PLATFORM,
                .target_id = 0,
                .requires_hold = true,
            },

        },

        .platform_count = 1,
        .platforms = {
            {
            .x = 560,
            .y = 152,
            .width = 32,
            .height = 8,
            .speed = 1.0f,
            .target_x = 500,
            .platfrom_id = 0,
            },
        }
    },
};



void loadLevel(const LevelConfig *config, Key *key) {
    current_level_width = config->width;
    NF_LoadTiledBg(config->bg_name, "level", config->width, 256);
    NF_CreateTiledBg(0, 3, "level");
    NF_LoadCollisionBg(config->col_name, 0, config->width, 256);

    NF_LoadSpriteGfx("sprite/key", GFX_SLOT_KEY, 16, 32);
    NF_VramSpriteGfx(0, GFX_SLOT_KEY, GFX_SLOT_KEY, false);
    NF_LoadSpritePal("sprite/key", PAL_SLOT_KEY);
    NF_VramSpritePal(0, PAL_SLOT_KEY, PAL_SLOT_KEY);
    NF_CreateSprite(0, SPRITE_BASE_KEY, GFX_SLOT_KEY, PAL_SLOT_KEY, 0, 0);
    key->sprite_id = SPRITE_BASE_KEY;

    NF_LoadSpriteGfx("sprite/door", GFX_SLOT_DOOR, 32, 32);
    NF_VramSpriteGfx(0, GFX_SLOT_DOOR, GFX_SLOT_DOOR, false);
    NF_LoadSpritePal("sprite/door", PAL_SLOT_DOOR);
    NF_VramSpritePal(0, PAL_SLOT_DOOR, PAL_SLOT_DOOR);
    NF_CreateSprite(0, SPRITE_BASE_DOOR, GFX_SLOT_DOOR, PAL_SLOT_DOOR, 0, 0);


    if (config->box_count > 0) {
        NF_LoadSpriteGfx("sprite/box", GFX_SLOT_BOX, 32, 32);
        NF_VramSpriteGfx(0, GFX_SLOT_BOX, GFX_SLOT_BOX, false);
        NF_LoadSpritePal("sprite/box", PAL_SLOT_BOX);
        NF_VramSpritePal(0, PAL_SLOT_BOX, PAL_SLOT_BOX);
    }
    for (int i=0; i < config->box_count; i++) {
        NF_CreateSprite(0, SPRITE_BASE_BOX + i, GFX_SLOT_BOX, PAL_SLOT_BOX, config->boxes[i].spawn_x, config->boxes[i].spawn_y);
    }




    NF_LoadSpriteGfx("sprite/button", GFX_SLOT_BUTTON, 16, 8);
    NF_VramSpriteGfx(0, GFX_SLOT_BUTTON, GFX_SLOT_BUTTON, false);
    NF_LoadSpritePal("sprite/button", PAL_SLOT_BUTTON);
    NF_VramSpritePal(0, PAL_SLOT_BUTTON, PAL_SLOT_BUTTON);
    for (int i = 0; i < config->button_count; i++) {
        NF_CreateSprite(0, SPRITE_BASE_BUTTON + i, GFX_SLOT_BUTTON, PAL_SLOT_BUTTON, config->buttons[i].x, config->buttons[i].y);
    }


    NF_LoadSpriteGfx("sprite/platform", GFX_SLOT_PLATFORM, 32, 8);
    NF_VramSpriteGfx(0, GFX_SLOT_PLATFORM, GFX_SLOT_PLATFORM, false);
    NF_LoadSpritePal("sprite/platform", PAL_SLOT_PLATFORM);
    NF_VramSpritePal(0, PAL_SLOT_PLATFORM, PAL_SLOT_PLATFORM);
    for (int i = 0; i < config->platform_count; i++) {
        NF_CreateSprite(0, SPRITE_BASE_PLATFORM + i, GFX_SLOT_PLATFORM, PAL_SLOT_PLATFORM, config->platforms[i].x, config->platforms[i].y);
    }

    current_box_count = config->box_count;
    current_button_count = config->button_count;
    current_platform_count = config->platform_count;
}

void resetLevel(Player *players, float *camera_x, const LevelConfig *config, Key *key, Box *boxes, Button *buttons, Platform *platforms) {
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
        boxes[i].sprite_id = SPRITE_BASE_BOX + i;
        boxes[i].object_on_top = NOTHING;
        boxes[i].object_on_top_id = -1;
        NF_SpriteFrame(0, 6, boxes[i].push_required);
    }

    for (int i=0; i < config->button_count; i++) {
        buttons[i].pressed = false;
        buttons[i].x = config->buttons[i].x;
        buttons[i].y = config->buttons[i].y;
        buttons[i].sprite_id = SPRITE_BASE_BUTTON + i;
        buttons[i].type = config->buttons[i].type;
        buttons[i].triggered_by_id = -1;
        buttons[i].requires_hold = config->buttons[i].requires_hold;
        buttons[i].hold_timer = 0;
        NF_SpriteFrame(0, buttons[i].sprite_id, 0);
    }

    for (int i = 0; i < config->platform_count; i++) {
        platforms[i].active = false;
        platforms[i].x = config->platforms[i].x;
        platforms[i].y = config->platforms[i].y;
        platforms[i].start_x = config->platforms[i].x;
        platforms[i].target_x = config->platforms[i].target_x;
        platforms[i].speed = config->platforms[i].speed;
        platforms[i].width = config->platforms[i].width;
        platforms[i].height = config->platforms[i].height;
        platforms[i].sprite_id = SPRITE_BASE_PLATFORM + i;
        platforms[i].platform_id = config->platforms[i].platfrom_id;
    }
} 


void unloadLevel(void) {
    NF_DeleteTiledBg(0, 3);
    NF_UnloadTiledBg("level");
    NF_UnloadCollisionBg(0);

    // Unload key
    NF_DeleteSprite(0, SPRITE_BASE_KEY);
    NF_FreeSpriteGfx(0, GFX_SLOT_KEY);
    NF_UnloadSpriteGfx(GFX_SLOT_KEY);
    NF_UnloadSpritePal(PAL_SLOT_KEY);

    // Unload door
    NF_DeleteSprite(0, SPRITE_BASE_DOOR);
    NF_FreeSpriteGfx(0, GFX_SLOT_DOOR);
    NF_UnloadSpriteGfx(GFX_SLOT_DOOR);
    NF_UnloadSpritePal(PAL_SLOT_DOOR);

  
    // Unload box
    for (int i = 0; i < current_box_count; i++) {
        NF_DeleteSprite(0, SPRITE_BASE_BOX + i);
    }
    if (current_box_count > 0) {
        NF_FreeSpriteGfx(0, GFX_SLOT_BOX);
        NF_UnloadSpriteGfx(GFX_SLOT_BOX);
        NF_UnloadSpritePal(PAL_SLOT_BOX);

    }
    

    // Unload Button
    for (int i = 0; i < current_button_count; i++) {
        NF_DeleteSprite(0, SPRITE_BASE_BUTTON + i);
    }
    NF_FreeSpriteGfx(0, GFX_SLOT_BUTTON);
    NF_UnloadSpriteGfx(GFX_SLOT_BUTTON);
    NF_UnloadSpritePal(PAL_SLOT_BUTTON);


    // Unload Platform
    for (int i = 0; i < current_platform_count; i++) {
        NF_DeleteSprite(0, SPRITE_BASE_PLATFORM + i);
    }
    NF_FreeSpriteGfx(0, GFX_SLOT_PLATFORM);
    NF_UnloadSpriteGfx(GFX_SLOT_PLATFORM);
    NF_UnloadSpritePal(PAL_SLOT_PLATFORM);


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