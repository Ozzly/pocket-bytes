#include <stdio.h>
#include <nds.h>
#include <nf_lib.h>
#include <filesystem.h>
#include "player.h"
#include "box.h"
#include "level.h"
#include "collisions.h"
#include "camera.h"
#include "platform.h"










#define COL_EMPTY 3


#define PLAYER_DEATH_TIME 90




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
    
    NF_LoadSpritePal("sprite/byte", 2);
    NF_LoadSpritePal("sprite/byte-mauve", 0);
    NF_LoadSpritePal("sprite/byte-saphire", 1);
    NF_LoadSpritePal("sprite/byte-bone", 3);
    NF_VramSpritePal(0, 0, 0);
    NF_VramSpritePal(0, 1, 1);
    NF_VramSpritePal(0, 2, 2);
    NF_VramSpritePal(0, 3, 3);

    // Create sprite instance
    // NF_CreateSprite(0, 0, 0, 3, 120, 80);
    // NF_CreateSprite(0, 1, 0, 1, 160, 80);
    // NF_CreateSprite(0, 2, 0, 2, 140, 90);

    Key key;
    Button buttons[MAX_BUTTONS];
    Platform platforms[MAX_PLATFORMS];

    // Set background color
    BG_PALETTE[0] = RGB15(31, 31, 31);


    int current_level = 0;

    Player players[MAX_PLAYERS];
    for (int i=0; i < current_player_count; i++) {
        players[i].sprite_id = i;
        players[i].palette_id = i + 1;
        players[i].sprite_frame = 0;
        players[i].sprite_frame_debounce = 0;

        NF_CreateSprite(0, SPRITE_BASE_PLAYER + i, GFX_SLOT_PLAYER, PAL_SLOT_PLAYER_BASE + i, LEVELS[current_level].spawn_x[i], LEVELS[current_level].spawn_y[i]);
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
    loadLevel(&LEVELS[current_level], &key);
    resetLevel(players, &camera_x, &LEVELS[current_level], &key, boxes, buttons, platforms);

    int death_timer = PLAYER_DEATH_TIME;

    while (1)
    {
        if (state == STATE_PLAYING) {
            // Read keypad
            scanKeys();
            u16 keys = keysHeld();
            u16 keys_down = keysDown();
    
            

            
           
    
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

            checkPlayerButtonOverlap(buttons, players);
            updateButtons(buttons, players, platforms);

            for (int i = 0; i < current_player_count; i++) {
                if (players[i].is_dead) {
                    state = STATE_DYING;

                    NF_SpriteFrame(0, players[i].sprite_id, 6); // Set to death frame 
                    players[i].vel_y = -6.0f; // death bounce
                }
            }



            if (isLevelComplete(players)) {
                unloadLevel();

                current_level++;
                loadLevel(&LEVELS[current_level], &key);
                resetLevel(players, &camera_x, &LEVELS[current_level], &key, boxes, buttons, platforms);
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
                resetLevel(players, &camera_x, &LEVELS[current_level], &key, boxes, buttons, platforms);
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
        updateObjectPosition(6, boxes[0].x, boxes[0].y, BOX_WIDTH, camera_x); //box 0
        for (int i=0; i < current_button_count; i++) {
            updateObjectPosition(buttons[i].sprite_id, buttons[i].x, buttons[i].y, 16, camera_x);
        }
        for (int i = 0; i < current_platform_count; i++) {
            updateObjectPosition(platforms[i].sprite_id, platforms[i].x, platforms[i].y, platforms[i].width, camera_x);
        }
        
        

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
