#include "title_screen.h"
#include <nf_lib.h>
#include "globals.h"
#include "player.h"
#include "collisions.h"
#include "key.h"
#define DEMO_PLAYER_COUNT 3
#define SPRITE_BASE_DEMO 30

static const DemoKeyFrame SCRIPT_0[] = {
    {DEMO_IDLE, 250},
    {DEMO_JUMP, 1},
};

static const DemoKeyFrame SCRIPT_1[] = {
    {DEMO_IDLE, 330},
    {DEMO_JUMP, 1},
};

static const DemoKeyFrame SCRIPT_2[] = {
    {DEMO_IDLE, 199},
    {DEMO_JUMP, 1},   
};

typedef struct {
    const DemoKeyFrame *frames;
    int length;
    int keyframe;
    int keyframe_timer;
} DemoScript;

static DemoScript scripts[DEMO_PLAYER_COUNT] = {
    { .frames = SCRIPT_0, .length = 2 },
    { .frames = SCRIPT_1, .length = 2 },
    { .frames = SCRIPT_2, .length = 2 },
};

Player demo_players[DEMO_PLAYER_COUNT] = {
    {
        .x = 60.0f,
        .y = 140.0f,
        .vel_x = 0.0f,
        .vel_y = 0.0f,
        .sprite_id = -1,
        .palette_id = 0,
        .on_ground = false,
    },
    {
        .x = 140.0f,
        .y = 140.0f,
        .vel_x = 0.0f,
        .vel_y = 0.0f,
        .sprite_id = -1,
        .palette_id = 1,
        .on_ground = false,
    },
    {
        .x = 220.0f,
        .y = 70.0f,
        .vel_x = 0.0f,
        .vel_y = 0.0f,
        .sprite_id = -1,
        .palette_id = 2,
        .on_ground = false,
    }
};

// Key demo_key = {
//     .x = 234.0f,
//     .y = 75.0f,
//     .sprite_id = 4,
//     .carried_by = -1,
//     .swap_buffer = 0,
//     .door_unlocked = false,    
// };

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

const LevelConfig demoLevelConfig = {
    .bg_name = "bg/title-top",
    .col_name = "collision/title-top-col",
    .width = 256,
    .void_count = 0,

};

void updateDemoPlayers() {
    


    for (int i = 0; i < DEMO_PLAYER_COUNT; i++) {
        Player *p = &demo_players[i];
        DemoScript *s = &scripts[i];
       
        
        s->keyframe_timer++;
        if (s->keyframe_timer >= s->frames[s->keyframe].duration) {
            s->keyframe_timer = 0;
            s->keyframe = (s->keyframe + 1) % s->length;
        }

        DemoAction action = s->frames[s->keyframe].action;

        switch(action) {
            case DEMO_WALK_RIGHT: 
                p->vel_x = MAX_WALK;
                break;
            case DEMO_WALK_LEFT:
                p->vel_x = -MAX_WALK;
                break;
            case DEMO_IDLE:
                p->vel_x = 0;
                break;
            case DEMO_JUMP:
                p->vel_y = -4.7f;
                break;
            case DEMO_JUMP_RIGHT:
                p->vel_y = -4.7f;
                p->vel_x = MAX_WALK;
                break;
            case DEMO_JUMP_LEFT:
                p->vel_y = -4.7f;
                p->vel_x = -MAX_WALK;
                break;
        }

        updatePlayerPhysics(p, &demoLevelConfig);
        float old_x = p->x;
        resolvePlayerTileCollision(p);
        float displacement_x = p->x - old_x;
        if (displacement_x != 0) propagateMoveUp(p->object_on_top, p->object_on_top_id, displacement_x, demo_players, NULL);
        if (p->vel_x > 0) NF_HflipSprite(0, p->sprite_id, false);
        else if (p->vel_x < 0) NF_HflipSprite(0, p->sprite_id, true);
        updatePlayerSprite(p);


        NF_MoveSprite(0, p->sprite_id, p->x - 4, p->y - 4);
    }

    executeJumps(demo_players);

    // Reset stacking info
    for (int i =0; i < DEMO_PLAYER_COUNT; i++) {
        demo_players[i].standing_on = NOTHING;
        demo_players[i].standing_on_id = -1;
        demo_players[i].has_player_on_top = false;
        demo_players[i].object_on_top = NOTHING;
        demo_players[i].object_on_top_id = -1;
    }

    resolvePlayerPlayerCollision(demo_players);
}

void initDemoKey() {
    NF_LoadSpriteGfx("sprite/key", GFX_SLOT_KEY, 16, 32);
    NF_VramSpriteGfx(0, GFX_SLOT_KEY, GFX_SLOT_KEY, false);
    NF_LoadSpritePal("sprite/key", PAL_SLOT_KEY);
    NF_VramSpritePal(0, PAL_SLOT_KEY, PAL_SLOT_KEY);
    // NF_CreateSprite(0, demo_key.sprite_id, GFX_SLOT_KEY, PAL_SLOT_KEY, (int)demo_key.x, (int)demo_key.y);
}

// void updateDemoKey() {
//     keyPlayerTracking(demo_players, &demo_key);
// }

void unloadTitleScreen() {
    NF_UnloadTiledBg("title-top");
    NF_DeleteTiledBg(0, 3);
    NF_UnloadColisionBg(0);
    NF_UnloadTiledBg("mode-singleplayer");
    NF_UnloadTiledBg("mode-multiplayer");
    NF_UnloadTiledBg("mode-options");
    NF_DeleteTiledBg(1, 1);
    NF_DeleteTiledBg(1, 2);
    NF_DeleteTiledBg(1, 3);

    for (int i = 0; i < DEMO_PLAYER_COUNT; i++) {
        NF_DeleteSprite(0, demo_players[i].sprite_id);
    }

    NF_UnloadSpritePal(0);
    NF_UnloadSpritePal(1);
    NF_UnloadSpritePal(2);

    // NF_SpriteOamSet(0);
    // NF_SpriteOamSet(1);
    // oamUpdate(&oamMain);
    // oamUpdate(&oamSub);
    // swiWaitForVBlank();

    

    // NF_DeleteSprite(0, demo_key.sprite_id);
    // NF_FreeSpriteGfx(0, GFX_SLOT_KEY);
    // NF_UnloadSpriteGfx(GFX_SLOT_KEY);
    // NF_UnloadSpritePal(PAL_SLOT_KEY);
}