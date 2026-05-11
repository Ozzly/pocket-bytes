#include "title_screen.h"
#include <nf_lib.h>
#include "globals.h"
#include "player.h"
#define DEMO_PLAYER_COUNT 3
#define SPRITE_BASE_DEMO 30

static const DemoKeyFrame SCRIPT_0[] = {
    {DEMO_WALK_RIGHT, 60},
    {DEMO_JUMP, 30},
    {DEMO_WALK_RIGHT, 60},
    {DEMO_IDLE, 30},
    {DEMO_WALK_LEFT, 60},
    {DEMO_JUMP, 30},
    {DEMO_WALK_LEFT, 60},
    {DEMO_IDLE, 30},
};

static const DemoKeyFrame SCRIPT_1[] = {
    {DEMO_WALK_LEFT, 60},
    {DEMO_JUMP, 30},
    {DEMO_WALK_LEFT, 60},
    {DEMO_IDLE, 30},
    {DEMO_WALK_RIGHT, 60},
    {DEMO_JUMP, 30},
    {DEMO_WALK_RIGHT, 60},
    {DEMO_IDLE, 30},
};

static const DemoKeyFrame SCRIPT_2[] = {
    {DEMO_STAND_ON_PLAYER, 120},
    {DEMO_IDLE, 60},
    {DEMO_STAND_ON_PLAYER, 120},
    {DEMO_IDLE, 60},
};

DemoPlayer demo_players[DEMO_PLAYER_COUNT] = {
    {
        .x = 50.0f,
        .y = 120.0f,
        .vel_x = 0.0f,
        .vel_y = 0.0f,
        .sprite_id = -1,
        .palette_id = 0,
        .keyframe = 0,
        .keyframe_timer = 0,
        .script = SCRIPT_0, 
        .script_length = 8,
        .on_ground = false,
    },
    {
        .x = 100.0f,
        .y = 120.0f,
        .vel_x = 0.0f,
        .vel_y = 0.0f,
        .sprite_id = -1,
        .palette_id = 1,
        .keyframe = 0,
        .keyframe_timer = 0,
        .script = SCRIPT_1, 
        .script_length = 8,
        .on_ground = false,
    },
    {
        .x = 150.0f,
        .y = 120.0f,
        .vel_x = 0.0f,
        .vel_y = 0.0f,
        .sprite_id = -1,
        .palette_id = 2,
        .keyframe = 0,
        .keyframe_timer = 0,
        .script = SCRIPT_2,
        .script_length = 4,
        .on_ground = false,
    }
};

void createDemoPlayers() {
    NF_LoadSpritePal("sprite/byte", 2);
    NF_LoadSpritePal("sprite/byte-mauve", 0);
    NF_LoadSpritePal("sprite/byte-saphire", 1);
    NF_VramSpritePal(0, 0, 0);
    NF_VramSpritePal(0, 1, 1);
    NF_VramSpritePal(0, 2, 2);

    for (int i = 0; i < DEMO_PLAYER_COUNT; i++) {
        demo_players[i].sprite_id = SPRITE_BASE_DEMO + i;
        demo_players[i].sprite_frame = 0;
        demo_players[i].sprite_frame_debounce = 0;
        NF_CreateSprite(0, SPRITE_BASE_DEMO + i, GFX_SLOT_PLAYER, demo_players[i].palette_id, (int)demo_players[i].x, (int)demo_players[i].y);

    }
}

static int titleIsFloorAt(int x, int y) {
    if (x > 0 && x < 55 && y > 159) return 159;
    if (x >= 55 && x < 103 && y >= 175) return 175;
    if (x >= 103 && x < 175 && y >= 183) return 183;
    if (x >= 176 && x < 223 && y >= 159) return 159;
    if (x >= 224 && x < 255 && y >= 111) return 111;

    return -1;
}

void updateDemoPlayers() {
    for (int i = 0; i < DEMO_PLAYER_COUNT; i++) {
        DemoPlayer *p = &demo_players[i];
        DemoKeyFrame current_keyframe = p->script[p->keyframe];

        p->keyframe_timer++;
        if (p->keyframe_timer >= current_keyframe.duration) {
            p->keyframe_timer = 0;
            p->keyframe = (p->keyframe + 1) % p->script_length;
        }


        // Simple movement logic based on current keyframe action
        switch (current_keyframe.action) {
            case DEMO_WALK_RIGHT:
                p->vel_x = 1.0f;
                break;
            case DEMO_WALK_LEFT:
                p->vel_x = -1.0f;
                break;
            case DEMO_JUMP:
                if (p->vel_y == 0) p->vel_y = -4.0f;
                break;
            case DEMO_IDLE:
                p->vel_x = 0.0f;
                break;
            case DEMO_STAND_ON_PLAYER:
                p->vel_x = 0.0f;
                // Position ontop of stored player ontop
                break;
            
        }

        p->sprite_frame_debounce++;

        if (p->vel_x > 0 && p->sprite_frame_debounce > 5) {
            NF_HflipSprite(0, p->sprite_id, false);
            
            p->sprite_frame_debounce = 0;
            p->sprite_frame++;
            if (p->sprite_frame > 5) p->sprite_frame = 2;
            NF_SpriteFrame(0, p->sprite_id, p->sprite_frame);
        } else if (p->vel_x < 0 && p->sprite_frame_debounce > 5) {
            NF_HflipSprite(0, p->sprite_id, true);
            p->sprite_frame_debounce = 0;
            p->sprite_frame++;
            if (p->sprite_frame > 5) p->sprite_frame = 2;
            NF_SpriteFrame(0, p->sprite_id, p->sprite_frame);
        }

        // Apply gravity
        p->vel_y += GRAVITY;
        p->x += p->vel_x;
        p->y += p->vel_y;

        int isPlayerLeftOnGround = titleIsFloorAt(p->x, p->y + PLAYER_HEIGHT);
        if (isPlayerLeftOnGround != -1) {
            p->y = isPlayerLeftOnGround - PLAYER_HEIGHT;
            p->vel_y = 0;
        }
        int isPlayerRightOnGround = titleIsFloorAt(p->x + PLAYER_WIDTH, p->y + PLAYER_HEIGHT);
        if (isPlayerRightOnGround != -1) {
            p->y = isPlayerRightOnGround - PLAYER_HEIGHT;
            p->vel_y = 0;
        }
        

        NF_MoveSprite(0, p->sprite_id, (int)p->x, (int)p->y);
    }

}

void unloadTitleScreen() {
    NF_UnloadTiledBg("title-top");
    NF_DeleteTiledBg(0, 3);
    NF_UnloadTiledBg("mode-singleplayer");
    NF_UnloadTiledBg("mode-multiplayer");
    NF_UnloadTiledBg("mode-options");
    NF_DeleteTiledBg(1, 0);
    NF_DeleteTiledBg(1, 1);
    NF_DeleteTiledBg(1, 2);

    NF_UnloadSpritePal(0);
    NF_UnloadSpritePal(1);
    NF_UnloadSpritePal(2);

    for (int i = 0; i < DEMO_PLAYER_COUNT; i++) {
        NF_DeleteSprite(0, demo_players[i].sprite_id);
    }
}