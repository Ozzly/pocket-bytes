#include <stdio.h>
#include <nds.h>
#include <nf_lib.h>
#include <filesystem.h>
#include <math.h>

#define GRAVITY 0.35f
#define MAX_FALL 6.0f
#define WALK_ACCELERATION 0.5f
#define MAX_WALK 1.5f
#define GROUND_FRICTION 0.8f // 1 is max friction
#define AIR_FRICTION 0.7f
#define JUMP_STRENGTH -4.7f
#define COYOTE_TIME 4
#define JUMP_BUFFER_TIME 3

#define PLAYER_WIDTH 22
#define PLAYER_HEIGHT 24
#define TILE 8

#define KEY_WIDTH 16
#define KEY_HEIGHT 24

#define DOOR_WIDTH 24
#define DOOR_HEIGHT 32

#define BOX_WIDTH 24
#define BOX_HEIGHT 24

#define COL_SOLID 2
#define COL_EMPTY 3
#define COL_SPIKE 1

#define MAX_VOIDS 4
#define MAX_BOXES 4

#define MAX_PLAYERS 4
#define PLAYER_DEATH_TIME 90

#define CAMERA_OFFSET 116 // Half of screen (128) - sprite center offset (12)

typedef enum {
    STATE_PLAYING,
    STATE_DYING,
} GameState;

GameState state = STATE_PLAYING;

typedef struct {
    int left_x;
    int right_x;
    int respawn_x;
} Void;

typedef enum {
    NOTHING,
    PLAYER,
    BOX,
} GameObject;

typedef struct {
    float x, y;
    float vel_x, vel_y;
    int push_required;
    bool on_ground;
    int sprite_id;
    GameObject object_on_top;
    int object_on_top_id;
    GameObject standing_on;
    int standing_on_id;
} Box;

typedef struct {
    int spawn_x, spawn_y;
    int push_required;
} BoxSpawn;

typedef struct {
    const char *bg_name;
    const char *col_name;
    int width;
    float spawn_x[MAX_PLAYERS];
    float spawn_y[MAX_PLAYERS];
    int key_spawn_x;
    int key_spawn_y;
    int door_x;
    int door_y;
    Void voids[MAX_VOIDS];
    int void_count;
    BoxSpawn boxes[MAX_BOXES];
    int box_count;
} LevelConfig;

static const LevelConfig LEVELS[] = {
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
        .box_count = 1,
        .boxes = {
            {
                .spawn_x = 100,
                .spawn_y = 10,
                .push_required = 1,
            },
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
                .spawn_y = 160,
                .push_required = 1,
            },
        },
    },
};

static int current_player_count = 2;
static int current_level_width = 0;
static int current_box_count = 0;

typedef struct {
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


typedef struct {
    float x, y;
    int sprite_id;
    int carried_by; // -1 if not being carried
    int swap_buffer;
    bool door_unlocked;
} Key;




// Functions for level setup and reset
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

// Helper functions
bool overlaps(float ax, float ay, float aw, float ah, float bx, float by, float bw, float bh) {
    return ax < bx + bw && ax + aw > bx && ay < by + bh && ay + ah > by;
}


// Functions for main game loop
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

void updateBoxPhysics(Box *b, const LevelConfig *config) {
    

    // Gravity
    if (!b->on_ground) b->vel_y += GRAVITY;
    if (b->vel_y > MAX_FALL) b->vel_y = MAX_FALL;

    // Box wraps to top of screen if in void
    if (b->y > 192) {

        for (int i=0; i < config->void_count; i++) {
            Void voidZone = config->voids[i];
            if (b->x >= voidZone.left_x && b->x <= voidZone.right_x) {
                b->x = voidZone.respawn_x;
                b->y = 0;
            }
        }

        b->y = -BOX_HEIGHT;
        b->on_ground = false;
    }
}
bool isSolid(int x, int y) {
    if (y >= 192 || y < 0) return false;
    if (x < 0 || x >= current_level_width) return true;
    return NF_GetPoint(0, x, y) == COL_SOLID;
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

void resolveBoxHorizontalTileCollision(Box *b, float displacement_x) {
    int int_box_x = (int)b->x, int_box_y = (int)b->y ;

    if (displacement_x > 0) { // Check right edge
        if (isSolid(int_box_x + BOX_WIDTH, int_box_y + 2) || isSolid(int_box_x + BOX_WIDTH, int_box_y + BOX_HEIGHT -2)) {
            b->x = (float)(((int_box_x + BOX_WIDTH) / TILE) * TILE - BOX_WIDTH) -0.01f;
            b->vel_x = 0;
        }
    }
    if (displacement_x < 0) { // Check left edge
        if (isSolid(int_box_x, int_box_y + 2) || isSolid(int_box_x, int_box_y + BOX_HEIGHT -2 )) {
            if (int_box_x < 0) {
                b->x = 0;
            } else {
                b->x = (float)((int_box_x / TILE + 1) * TILE);
            }
            b->vel_x = 0;
        }

    }
}

void resolveBoxTileCollision(Box *b) {
    // Horizontal Collision
    b->x += b->vel_x;
    int int_box_x = (int)b->x, int_box_y = (int)b->y;

    resolveBoxHorizontalTileCollision(b, b->vel_x);

    // Vertical Collision
    b->y += b->vel_y;
    int_box_x = (int)b->x, int_box_y = (int)b->y;
    b->on_ground = false;

    if (b->vel_y >= 0) { // falling or standing still, check feet
        if (isSolid(int_box_x + 2, int_box_y + BOX_HEIGHT) || isSolid(int_box_x + BOX_WIDTH -2, int_box_y + BOX_HEIGHT) || isSolid(int_box_x + BOX_WIDTH / 2, int_box_y + BOX_HEIGHT)) {
            b->y = (float)(((b->y + BOX_HEIGHT) / TILE) * TILE - BOX_HEIGHT);
            b->vel_y = 0;
            b->on_ground = true;
        }
    }
    if (b->vel_y < 0) { // rising, check head
        if (isSolid(int_box_x + 2, int_box_y) || isSolid(int_box_x + BOX_WIDTH -2, int_box_y)) {
            b->y = (float)((b->y / TILE + 1) * TILE);
            b->vel_y = 0;
        }
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


void propagateMoveUp(GameObject object_on_top, int object_on_top_id, float displacement, Player *players, Box *boxes) {
    if (object_on_top == PLAYER) {
        Player *rider = &players[object_on_top_id];
        rider->x += displacement;
        resolvePlayerHorizontalTileCollision(rider, displacement);
        propagateMoveUp(rider->object_on_top, rider->object_on_top_id, displacement, players, boxes);
    } else if (object_on_top == BOX) {
        Box *above = &boxes[object_on_top_id - 6];
        above->x += displacement;
        resolveBoxHorizontalTileCollision(above, displacement);
        propagateMoveUp(above->object_on_top, above->object_on_top_id, displacement, players, boxes);
    }
}


// void applyCarry(Player *players, float *prev_x, Box *boxes, float *prev_box_x) {
//     for (int i = 0; i < current_player_count; i++) {
//         if (players[i].standing_on == PLAYER) {
//             int bot = players[i].standing_on_id;
//             float displacement_x = players[bot].x - prev_x[bot];
//             players[i].x += displacement_x;
//             resolvePlayerHorizontalTileCollision(&players[i], displacement_x);
//         } else if (players[i].standing_on == BOX) {
//             int bot = players[i].standing_on_id - 6;
//             float displacement_x = boxes[bot].x - prev_box_x[bot];
//             players[i].x += displacement_x;
//             resolvePlayerHorizontalTileCollision(&players[i], displacement_x);
//         }
//     }
// }

// void applyBoxCarry(Player *players, float *prev_x, Box *boxes, float *prev_box_x) {
//     for (int i = 0; i < current_box_count; i++) {
//         if (boxes[i].standing_on == PLAYER) {
//             int bot = boxes[i].standing_on_id;
//             float displacement_x = players[bot].x - prev_x[bot];
//             boxes[i].x += displacement_x;
//             resolveBoxHorizontalTileCollision(&boxes[i], displacement_x);
//         } else if (boxes[i].standing_on == BOX) {
//             int bot = boxes[i].standing_on_id;
//             // float displacement_x = boxes[]
//         }
//     }
// }

void resetStackingInfo(Player *players, Box *boxes) {
    for (int i = 0; i < current_player_count; i++)  { 
        players[i].standing_on = NOTHING;
        players[i].standing_on_id = -1;
        players[i].has_player_on_top = false;
        players[i].object_on_top = NOTHING;
        players[i].object_on_top_id = -1;
    }

    for (int i = 0; i < current_box_count; i++) {
        boxes[i].standing_on = NOTHING;
        boxes[i].standing_on_id = -1;
        boxes[i].object_on_top = NOTHING;
        boxes[i].object_on_top_id = -1;
    }
}

void resolvePlayerPlayerCollision(Player *players) {
    for (int i = 0; i < current_player_count; i++) {
        for (int j = i + 1; j < current_player_count; j++) {
            Player *a = &players[i];
            Player *b = &players[j];

            if (a->in_door || b->in_door) continue; // skip collision if players in the door

            float a_left = a->x, a_top = a->y;
            float a_right = a->x + PLAYER_WIDTH, a_bottom = a->y + PLAYER_HEIGHT;
            float b_left = b->x, b_top = b->y;
            float b_right = b->x + PLAYER_WIDTH, b_bottom = b->y + PLAYER_HEIGHT;

            if (a_right <= b_left || b_right <= a_left) continue; // No horizontal overlap
            if (a_bottom < b_top || b_bottom < a_top) continue; // No vertical overlap

            float penetration_x = a_left < b_left 
            ? a_right - b_left // a is left of b so penetration is how much a's right edge overlaps b's left edge
            : -(b_right - a_left); // negative - push a right, b left
            float penetration_y = a_top < b_top // true if a is above b (as coords increase downwards) 
            ? a_bottom - b_top // a is above b so penetration is how much a's bottom edge overlaps b's top edge
            : -(b_bottom - a_top);

            if (fabsf(penetration_x) <= fabsf(penetration_y)) { // Smallest distance between 2 players sides is horizontal
                if (penetration_x > 0) { // a is left of b
                    if (a->vel_x > 0 && b->vel_x < 0) { // a and b are walking into each other, so split the diff
                        a->x -= penetration_x / 2.0f;
                        b->x += penetration_x / 2.0f;
                        a->vel_x = 0;
                        b->vel_x = 0;
                    } else if (a->vel_x > 0) { // a is moving right into b, so push back a
                        a->x -= penetration_x;
                        a->vel_x = 0;
                    } else { // b walked left into a 
                        b->x += penetration_x;
                        b->vel_x = 0;    
                    }
                } else { // b is left of a
                    if (b->vel_x > 0 && a->vel_x < 0) { // both walking into each other
                        a->x += (-penetration_x) / 2.0f;
                        b->x -= (-penetration_x) / 2.0f;
                        a->vel_x = 0;
                        b->vel_x = 0;
                    } else if (b->vel_x > 0) { // b walked right into a
                        b->x -= (-penetration_x); // penetration_x is negative, so cancel out the negative to move b left
                        b->vel_x = 0;
                    } else { // a walked left into b
                        a->x +=  (-penetration_x);
                        a->vel_x = 0;
                    }
                }

            } else { // Smallest distance between 2 players is foot to head
                if (a_top < b_top) {
                    if (penetration_y >= 0) { // a is inside b, so move a up
                        a->y = (float)(b_top - PLAYER_HEIGHT); // 1 pixel inside b to stick to b while they move
                        a->vel_y = 0;
                        a->on_ground = true;
                        a->standing_on_id = b->sprite_id;
                        a->standing_on = PLAYER;
                        b->has_player_on_top = true;
                        b->object_on_top = PLAYER;
                        b->object_on_top_id = a->sprite_id;

                    }
                } else { // b is inside a, so move b up
                    if (penetration_y <= 0) {
                        b->y = (float)(a_top - PLAYER_HEIGHT);
                        b->vel_y = 0;
                        b->on_ground = true;
                        b->standing_on_id = a->sprite_id;
                        b->standing_on = PLAYER;
                        a->has_player_on_top = true;
                        a->object_on_top = PLAYER;
                        a->object_on_top_id = b->sprite_id;

                    }
                }
            }
        }
    }
}

bool boxBlockedByPlayer(float new_box_x, float box_y, int box_sprite_id, Player *players, int pushing_player_index) {
    for (int i=0; i < current_player_count; i++) {
        if (i == pushing_player_index || players[i].in_door) continue;
        if (players[i].y + PLAYER_HEIGHT <= box_y + 3) continue;
        if (overlaps(new_box_x, box_y, BOX_WIDTH, BOX_HEIGHT, players[i].x, players[i].y, PLAYER_WIDTH, PLAYER_HEIGHT)) {
            return true;
        }
    }
    return false;
}

void resolvePlayerBoxCollision(Player *players, Box *boxes) {
 for (int i = 0; i < current_player_count; i++) {
        for (int j = 0; j < current_box_count; j++) {
            Player *a = &players[i];
            Box *b = &boxes[j];

            if (a->in_door) continue; // skip collision if players in the door

            float a_left = a->x, a_top = a->y;
            float a_right = a->x + PLAYER_WIDTH, a_bottom = a->y + PLAYER_HEIGHT;
            float b_left = b->x, b_top = b->y;
            float b_right = b->x + BOX_WIDTH, b_bottom = b->y + BOX_HEIGHT;

            if (a_right <= b_left || b_right <= a_left) continue; // No horizontal overlap
            if (a_bottom < b_top || b_bottom < a_top) continue; // No vertical overlap

            float penetration_x = a_left < b_left 
            ? a_right - b_left // a is left of b so penetration is how much a's right edge overlaps b's left edge
            : -(b_right - a_left); // negative - push a right, b left
            float penetration_y = a_top < b_top // true if a is above b (as coords increase downwards) 
            ? a_bottom - b_top // a is above b so penetration is how much a's bottom edge overlaps b's top edge
            : -(b_bottom - a_top);

            if (!(fabsf(penetration_x) <= fabsf(penetration_y)))  { // Smallest distance between box and player is foot to head
                if (a_top < b_top) {
                    if (penetration_y >= 0) { // player is inside box, so move player up
                        a->y = (float)(b_top - PLAYER_HEIGHT); // 1 pixel inside b to stick to b while they move
                        a->vel_y = 0;
                        a->on_ground = true;
                        a->standing_on = BOX;
                        a->standing_on_id = b->sprite_id;
                        b->object_on_top = PLAYER;
                        b->object_on_top_id = a->sprite_id;
                    }
                } else { // b is inside a, so move b up
                    if (penetration_y <= 0) {
                        b->y = (float)(a_top - PLAYER_HEIGHT);
                        b->vel_y = 0;
                        b->on_ground = true;
                        b->standing_on = PLAYER;
                        b->standing_on_id = a->sprite_id;
                        a->object_on_top = BOX;
                        a->object_on_top_id = b->sprite_id;
                    }
                }
            }
        }
    }

    for (int i = 0; i < current_player_count; i++) {
        for (int j = 0; j < current_box_count; j++) {
            Player *a = &players[i];
            Box *b = &boxes[j];

            if (a->in_door) continue; // skip collision if players in the door

            float a_left = a->x, a_top = a->y;
            float a_right = a->x + PLAYER_WIDTH, a_bottom = a->y + PLAYER_HEIGHT;
            float b_left = b->x, b_top = b->y;
            float b_right = b->x + BOX_WIDTH, b_bottom = b->y + BOX_HEIGHT;

            if (a_right <= b_left || b_right <= a_left) continue; // No horizontal overlap
            if (a_bottom <= b_top || b_bottom <= a_top) continue; // No vertical overlap

            float penetration_x = a_left < b_left 
            ? a_right - b_left // a is left of b so penetration is how much a's right edge overlaps b's left edge
            : -(b_right - a_left); // negative - push a right, b left
            float penetration_y = a_top < b_top // true if a is above b (as coords increase downwards) 
            ? a_bottom - b_top // a is above b so penetration is how much a's bottom edge overlaps b's top edge
            : -(b_bottom - a_top);

            if (fabsf(penetration_x) <= fabsf(penetration_y)) { // Smallest distance between box and player is horizontal
                if (penetration_x > 0) { // player is left of box
                    
                    if (a->vel_x > 0) { // player is moving right into box, so move box
                    
                        float new_x = b->x + penetration_x;
                        int int_new_x = (int)new_x;
                        bool tile_blocked = isSolid(int_new_x + BOX_WIDTH, (int)b->y + 2) || isSolid(int_new_x + BOX_WIDTH, (int)b->y + BOX_HEIGHT - 2);
                        bool player_blocked = boxBlockedByPlayer(new_x, b->y, b->sprite_id, players, i);

                        if (!tile_blocked && !player_blocked) {
                            float displacement = new_x - b->x;
                            b->x = new_x;

                            propagateMoveUp(b->object_on_top, b->object_on_top_id, displacement, players, boxes);
                        } else {
                            a->x = b->x - PLAYER_WIDTH - 0.01f;
                            a->vel_x = 0;
                        }
                        
                    
                    } else { // box moved into player from right, so push back box
                        b->x += penetration_x;
                    }
                } else { // box is left of player
                    if (a->vel_x < 0) { // player walked left into box
                        float new_x = b->x + penetration_x;
                        int int_new_x = (int)new_x;
                        bool tile_blocked = isSolid(int_new_x, (int)b->y + 2) || isSolid(int_new_x, (int)b->y + BOX_HEIGHT - 2);
                        bool player_blocked = boxBlockedByPlayer(new_x, b->y, b->sprite_id, players, i);

                        if (!tile_blocked && !player_blocked){
                            float displacement = new_x - b->x;
                            b->x = new_x;
                            propagateMoveUp(b->object_on_top, b->object_on_top_id, displacement, players, boxes);

                        } else {
                            a->x = b->x + BOX_WIDTH + 0.01f;
                            a->vel_x = 0;
                        }

                    } else { // box moved into player
                        b->x += -(penetration_x); 
                    }
                }

            }
        }
    }
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
            state = STATE_DYING;
            NF_SpriteFrame(0, p->sprite_id, 6); // Set to death frame
            p->is_dead = true; // track who died
            p->vel_y = -6.0f; // death bounce
        }
    }
}

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

int main(int argc, char **argv)
{
    // Screen for NitroFS init
    NF_Set2D(0, 0);
    NF_Set2D(1, 0);
    consoleDemoInit();
    printf("Initializing NitroFS... ");
    swiWaitForVBlank();

    // NitroFS init
    nitroFSInit(NULL);
    NF_SetRootFolder("NITROFS");

    // NitroFS is ready
    // Init 2D mode on both screens
    NF_Set2D(0, 0);
    NF_Set2D(1, 0);

    // Init tiled bg
    NF_InitTiledBgBuffers();
    NF_InitTiledBgSys(0);
    NF_InitTiledBgSys(1);
    NF_InitCmapBuffers();

    // Init sprites
    NF_InitSpriteBuffers();
    NF_InitSpriteSys(0);
    NF_InitSpriteSys(1);
    // Load sprite files from NitroFS
    NF_LoadSpriteGfx("sprite/byte", 0, 32, 32);
    NF_VramSpriteGfx(0, 0, 0, false);
    
    NF_LoadSpritePal("sprite/byte", 0);
    NF_LoadSpritePal("sprite/byte-mauve", 1);
    NF_LoadSpritePal("sprite/byte-saphire", 2);
    NF_LoadSpritePal("sprite/byte-bone", 3);
    NF_VramSpritePal(0, 0, 0); // green
    NF_VramSpritePal(0, 1, 1); // mauve
    NF_VramSpritePal(0, 2, 2); //saphire
    NF_VramSpritePal(0, 3, 3); // bone

    // Create sprite instance
    NF_CreateSprite(0, 0, 0, 3, 120, 80);
    NF_CreateSprite(0, 1, 0, 1, 160, 80);
    // NF_CreateSprite(0, 2, 0, 2, 140, 90);

    Key key;

    // Set background color
    BG_PALETTE[0] = RGB15(31, 31, 31);

    Player players[MAX_PLAYERS];
    for (int i=0; i < current_player_count; i++) {
        players[i].sprite_id = i;
        players[i].palette_id = i + 1;
        players[i].sprite_frame = 0;
        players[i].sprite_frame_debounce = 0;
    }

    players[0].key_left = KEY_LEFT;
    players[0].key_right = KEY_RIGHT; 
    players[0].key_jump = KEY_UP;

    players[1].key_left = KEY_Y;
    players[1].key_right = KEY_A;
    players[1].key_jump = KEY_X;

    players[2].key_left = KEY_L;
    players[2].key_right = KEY_R;
    players[2].key_jump = KEY_B;

    Box boxes[MAX_BOXES];

    float camera_x = 0;
    int current_level = 0;
    loadLevel(&LEVELS[current_level], &key);
    resetLevel(players, &camera_x, &LEVELS[current_level], &key, boxes);

    int death_timer = PLAYER_DEATH_TIME;

    while (1)
    {
        if (state == STATE_PLAYING) {
            // Read keypad
            scanKeys();
            u16 keys = keysHeld();
            u16 keys_down = keysDown();
    
            // Store previous player positions to apply carry movement (for people on top of moving players)
            float prev_x[MAX_PLAYERS];
            for (int i = 0; i < current_player_count; i++) prev_x[i] = players[i].x;

            float prev_box_x[MAX_BOXES];
            for (int i = 0; i < current_box_count; i++) prev_box_x[i] = boxes[i].x;
           
    
            // Player movement, collision (map), and sprite animation
            for (int i = 0; i < current_player_count; i++) {
                Player *p = &players[i];

                // Skip player in door, if they press up, leave the door
                if (p->in_door) {
                    if (keys_down & p->key_jump) {
                        p->in_door = false;
                    }
                    continue;
                }

                updatePlayerInput(p, keys, keys_down);
                updatePlayerPhysics(p, &LEVELS[current_level]);
                float old_x = p->x;
                resolvePlayerTileCollision(p); 
                float displacement_x = p->x - old_x;
                if (displacement_x != 0) propagateMoveUp(p->object_on_top, p->object_on_top_id, displacement_x, players, boxes);
                updatePlayerSprite(p);
            }

            for (int i = 0; i < LEVELS[current_level].box_count; i++) {
                Box *b = &boxes[i];

                updateBoxPhysics(b, &LEVELS[current_level]);
                float old_x = b->x;
                resolveBoxTileCollision(b);
                float displacement_x = b->x - old_x;
                if (displacement_x != 0) propagateMoveUp(b->object_on_top, b->object_on_top_id, displacement_x, players, boxes);
            }
    
            // Apply carry from last frame's standing_on
            // applyBoxCarry(players, prev_x, boxes, prev_box_x);
            // applyCarry(players, prev_x, boxes, prev_box_x);
            // Reset stacking info before checking player to player collision, freeing players from each other
            resetStackingInfo(players, boxes); 
            // Player to player collision
            resolvePlayerPlayerCollision(players);

            resolvePlayerBoxCollision(players, boxes);

            checkDoor(players, &key, LEVELS);

            // Jumping after resolving all collisions
            executeJumps(players);
            // Player clamping to camera bounds
            playerClampToCamera(players, camera_x);
            // Camera follows players, but lets them walk to opposite ends of the screen 
            camera_x = getCameraPosition(players, camera_x);
            NF_ScrollBg(0, 3, camera_x, 0);
            // Check spike collision after all movement and other collisions resolved
            resolvePlayerSpikeCollision(players);

            // Track interactions with the key & have the key follow the player
            keyPlayerTracking(players, &key);



            if (isLevelComplete(players)) {
                unloadLevel();

                current_level++;
                loadLevel(&LEVELS[current_level], &key);
                resetLevel(players, &camera_x, &LEVELS[current_level], &key, boxes);
            }
        }


        if (state == STATE_DYING) {
            death_timer--;

            // Apply gravity to dead players until they fall off screen
            for (int i=0; i < current_player_count; i++) {
                Player *p = &players[i];
                if (p->is_dead && death_timer < PLAYER_DEATH_TIME-15 && p->y < 192) { 
                    p->vel_y += GRAVITY;
                    p->y += p->vel_y;
                }
            }

            // Rest level after death animation
            if (death_timer <= 0) {
                state = STATE_PLAYING;
                death_timer = PLAYER_DEATH_TIME;
                resetLevel(players, &camera_x, &LEVELS[current_level], &key, boxes);
            }
        }


        // Update player position on screen based on camera
        // Keep below all player and collision updates
        updatePlayerPosition(players, camera_x);

        // Update object positions relative to camera
        if (key.door_unlocked) { // key
            NF_MoveSprite(0, key.sprite_id, 0, 192);
        } else {
            updateObjectPosition(key.sprite_id, key.x, key.y, KEY_WIDTH, camera_x); 
        }
        updateObjectPosition(5, LEVELS[current_level].door_x, LEVELS[current_level].door_y, DOOR_WIDTH, camera_x); // door
        updateObjectPosition(6, boxes[0].x, boxes[0].y, BOX_WIDTH, camera_x);
        
        

        // Copy data from NFLib OAM buffers to the real OAM, wait for VBlank
        NF_SpriteOamSet(0);
        NF_SpriteOamSet(1);
        // Wait for the screen refresh
        swiWaitForVBlank();
        // Actually update the OAM
        oamUpdate(&oamMain);
        oamUpdate(&oamSub);
    }

    // If this is reached, the program will return to the loader if the loader
    // supports it.
    return 0;
}
