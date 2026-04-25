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


#define COL_SOLID 2
#define COL_EMPTY 3
#define COL_SPIKE 1

#define MAX_PLAYERS 4
#define PLAYER_DEATH_TIME 90

#define CAMERA_OFFSET 116 // Half of screen (128) - sprite center offset (12)

typedef enum {
    STATE_PLAYING,
    STATE_DYING,
} GameState;

GameState state = STATE_PLAYING;

typedef struct {
    const char *bg_name;
    const char *col_name;
    int width;
    float spawn_x[MAX_PLAYERS];
    float spawn_y[MAX_PLAYERS];
    int player_count;
} LevelConfig;

static const LevelConfig LEVELS[] = {
    {
        .bg_name = "bg/level1",
        .col_name = "collision/level1_col",
        .width = 1024,
        .spawn_x = { 40.0f, 60.0f, 80.0f },
        .spawn_y = { 150.0f, 150.0f, 150.0f },
    },
};

static int current_player_count = 2;
static int current_level_width = 0;

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
    int standing_on;
    bool has_player_on_top;
    bool is_dead;
} Player;




// Functions for level setup and reset
void loadLevel(const LevelConfig *config) {
    current_level_width = config->width;
    NF_LoadTiledBg(config->bg_name, "level", config->width, 256);
    NF_CreateTiledBg(0, 3, "level");
    NF_LoadCollisionBg(config->col_name, 0, config->width, 256);
}

void resetLevel(Player *players, int *camera_x, const LevelConfig *config) {
    for (int i=0; i < current_player_count; i++) {
        players[i].x = config->spawn_x[i];
        players[i].y = config->spawn_y[i];
        players[i].vel_x = 0.0f;
        players[i].vel_y = 0.0f;
        players[i].on_ground = false;
        players[i].coyote_frames = 0;
        players[i].jump_buffer = 0;
        players[i].standing_on = -1;
        players[i].has_player_on_top = false;
        players[i].is_dead = false;
    }
    *camera_x = 0;
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

void updatePlayerPhysics(Player *p) {
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
        p->y = -PLAYER_HEIGHT;
        p->on_ground = false;
        p->coyote_frames = 0;
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
    p->on_ground = false;

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

void applyCarry(Player *players, float *prev_x) {
    for (int i = 0; i < current_player_count; i++) {
        if (players[i].standing_on != -1) {
            int bot = players[i].standing_on;
            float displacement_x = players[bot].x - prev_x[bot];
            players[i].x += displacement_x;
            resolvePlayerHorizontalTileCollision(&players[i], displacement_x);
        }
    }
}

void resetStackingInfo(Player *players) {
    for (int i = 0; i < current_player_count; i++)  { 
        players[i].standing_on = -1;
        players[i].has_player_on_top = false;
    }
}

void resolvePlayerPlayerCollision(Player *players) {
    for (int i = 0; i < current_player_count; i++) {
        for (int j = i + 1; j < current_player_count; j++) {
            Player *a = &players[i];
            Player *b = &players[j];

            float a_left = a->x, a_top = a->y;
            float a_right = a->x + PLAYER_WIDTH, a_bottom = a->y + PLAYER_HEIGHT;
            float b_left = b->x, b_top = b->y;
            float b_right = b->x + PLAYER_WIDTH, b_bottom = b->y + PLAYER_HEIGHT;

            if (a_right <= b_left || b_right <= a_left) continue; // No horizontal overlap
            if (a_bottom <= b_top || b_bottom <= a_top) continue; // No vertical overlap

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
                if (penetration_y > 0) { // a is inside b, so move a up
                    a->y = (float)(b_top - PLAYER_HEIGHT + 1); // 1 pixel inside b to stick to b while they move
                    a->vel_y = 0;
                    a->on_ground = true;
                    a->standing_on = b->sprite_id;
                    b->has_player_on_top = true;
                } else { // b is inside a, so move b up
                    b->y = (float)(a_top - PLAYER_HEIGHT + 1);
                    b->vel_y = 0;
                    b->on_ground = true;
                    b->standing_on = a->sprite_id;
                    a->has_player_on_top = true;
                }
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

void playerClampToCamera(Player *players, int camera_x) {
    for (int i=0; i < current_player_count; i++) {
        Player *p = &players[i];
        if (p->x < camera_x) {
            p->x = (float)camera_x;
            p->vel_x = 0;
        }
        if (p->x + PLAYER_WIDTH > camera_x + 256) {
            p->x = (float)(camera_x + 256 - PLAYER_WIDTH);
            p->vel_x = 0;
        }
    }
}

int getCameraPosition(Player *players) {
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

void updatePlayerPosition(Player *players, int camera_x) {
    for (int i =0; i < current_player_count; i++) {
        Player *p = &players[i];
        NF_MoveSprite(0, p->sprite_id, p->x - camera_x - 4, p->y -4);
    }
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

    int camera_x = 0;

    int current_level = 0;
    loadLevel(&LEVELS[current_level]);
    resetLevel(players, &camera_x, &LEVELS[current_level]);

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
           
    
            // Player movement, collision (map), and sprite animation
            for (int i = 0; i < current_player_count; i++) {
                Player *p = &players[i];
                updatePlayerInput(p, keys, keys_down);
                updatePlayerPhysics(p);
                resolvePlayerTileCollision(p); 
                updatePlayerSprite(p);
            }
    
            // Apply carry from last frame's standing_on
            applyCarry(players, prev_x);
            // Reset stacking info before checking player to player collision, freeing players from each other
            resetStackingInfo(players); 
            // Player to player collision
            resolvePlayerPlayerCollision(players);
            // Jumping after resol;ving all collisions 
            executeJumps(players);
            // Player clamping to camera bounds
            playerClampToCamera(players, camera_x);
            // Camera follows players, but lets them walk to opposite ends of the screen 
            camera_x = getCameraPosition(players);
            NF_ScrollBg(0, 3, camera_x, 0);
            // Check spike collision after all movement and other collisions resolved
            resolvePlayerSpikeCollision(players);
    
            
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
                resetLevel(players, &camera_x, &LEVELS[current_level]);
            }
        }

        // Update player position on screen based on camera
        // Keep below all player and collision updates
        updatePlayerPosition(players, camera_x);

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
