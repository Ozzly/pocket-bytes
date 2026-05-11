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


void resetStackingInfo(Player *players, Box *boxes, Platform *platforms) {
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

    for (int i = 0; i < current_platform_count; i++) {
        platforms[i].object_on_top = NOTHING;
        platforms[i].object_on_top_id = -1;
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
    

    Key key;
    Button buttons[MAX_BUTTONS];
    Platform platforms[MAX_PLATFORMS];


    int current_level = 0;

    Player players[MAX_PLAYERS];
    

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
    // loadLevel(&LEVELS[current_level], &key);
    // resetLevel(players, &camera_x, &LEVELS[current_level], &key, boxes, buttons, platforms);

    int death_timer = PLAYER_DEATH_TIME;
    bool entering_state = true;
    
    typedef enum {
        SINGLEPLAYER,
        MULTIPLAYER,
        OPTIONS,
    } MenuOption;

    MenuOption menu_option = SINGLEPLAYER;
    touchPosition touch_pos;

    while (1)
    {

        if (state == STATE_TITLE) {

            // Setup title screen on 1st load
            if (entering_state) {
                NF_LoadTiledBg("bg/title-top", "title-top", 256, 256);
                NF_CreateTiledBg(0, 3, "title-top");
                NF_LoadTiledBg("bg/select-mode-singleplayer", "mode-singleplayer", 256, 256);
                NF_LoadTiledBg("bg/select-mode-multiplayer", "mode-multiplayer", 256, 256);
                NF_LoadTiledBg("bg/select-mode-options", "mode-options", 256, 256);
                NF_CreateTiledBg(1, 2, "mode-options");
                NF_CreateTiledBg(1, 1, "mode-multiplayer");
                NF_CreateTiledBg(1, 0, "mode-singleplayer");
                NF_HideBg(1, 1);
                NF_HideBg(1, 2);

                entering_state = false;
            }

            scanKeys();
            u16 keys_down = keysDown();
            u16 keys_held = keysHeld();
            bool option_selected = false;

            // Track option change for bg updates
            MenuOption prev_option = menu_option;

            // Check touch screen input
            if (keys_held & KEY_TOUCH) {
                touchRead(&touch_pos);

                if (touch_pos.px > 31 && touch_pos.px < 224) {
                    if (touch_pos.py >= 24 && touch_pos.py <= 58) {
                        menu_option = SINGLEPLAYER;
                        option_selected = true;
                    } else if (touch_pos.py >= 72 && touch_pos.py <= 106) {
                        menu_option = MULTIPLAYER;
                        option_selected = true;
                    } else if (touch_pos.py >= 120 && touch_pos.py <= 154) {
                        menu_option = OPTIONS;
                        option_selected = true;
                    }
                }
            }

            // Key inputs
            if (keys_down & KEY_DOWN) {
                if (menu_option == SINGLEPLAYER) {
                    menu_option = MULTIPLAYER;
                } else if (menu_option == MULTIPLAYER) {
                    menu_option = OPTIONS;
                }
            } else if (keys_down & KEY_UP) {
                if (menu_option == OPTIONS) {
                    menu_option = MULTIPLAYER;
                } else if (menu_option == MULTIPLAYER) {
                    menu_option = SINGLEPLAYER;
                }
            } else if (keys_down & KEY_A) {
                option_selected = true; 
            }

            // Change selected option through switching bg images
            if (menu_option != prev_option) {
                switch (menu_option) {
                    case SINGLEPLAYER:
                        NF_ShowBg(1, 0);
                        NF_HideBg(1, 1);
                        NF_HideBg(1, 2);
                        break;
                    case MULTIPLAYER:
                        NF_ShowBg(1, 1);
                        NF_HideBg(1, 0);
                        NF_HideBg(1, 2);
                        break;
                    case OPTIONS:
                        NF_ShowBg(1, 2);
                        NF_HideBg(1, 0);
                        NF_HideBg(1, 1);
                        break;
                }
            }

            // Select option
            if (option_selected) {
                entering_state = true;
                if (menu_option == SINGLEPLAYER) {
                    current_player_count = 2;
                    state = STATE_PLAYING;
                    unloadTitleScreen();
                } else if (menu_option == MULTIPLAYER) {
                    current_player_count = 2;
                } else if (menu_option == OPTIONS) {
                }
            }
        }


        if (state == STATE_PLAYING) {

            if (entering_state) {
                NF_LoadSpritePal("sprite/byte", 2);
                NF_LoadSpritePal("sprite/byte-mauve", 0);
                NF_LoadSpritePal("sprite/byte-saphire", 1);
                NF_LoadSpritePal("sprite/byte-bone", 3);
                NF_VramSpritePal(0, 0, 0);
                NF_VramSpritePal(0, 1, 1);
                NF_VramSpritePal(0, 2, 2);
                NF_VramSpritePal(0, 3, 3);


                loadLevel(&LEVELS[current_level], &key);
                resetLevel(players, &camera_x, &LEVELS[current_level], &key, boxes, buttons, platforms);

                for (int i=0; i < current_player_count; i++) {
                    players[i].sprite_id = i;
                    players[i].palette_id = i + 1;
                    players[i].sprite_frame = 0;
                    players[i].sprite_frame_debounce = 0;

                    NF_CreateSprite(0, SPRITE_BASE_PLAYER + i, GFX_SLOT_PLAYER, PAL_SLOT_PLAYER_BASE + i, LEVELS[current_level].spawn_x[i], LEVELS[current_level].spawn_y[i]);
                }


                entering_state = false;
            }


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
            resetStackingInfo(players, boxes, platforms); 
            // Player to player collision
            resolvePlayerPlayerCollision(players);

            checkPlayerButtonOverlap(buttons, players);
            updateButtons(buttons, players, platforms);

            resolvePlayerPlatformCollision(players, platforms);
            updatePlatforms(platforms, players, boxes);
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

        if (state == STATE_PLAYING || state == STATE_DYING) {
            updatePlayerPosition(players, camera_x);
    
            // Update object positions relative to camera
            if (key.door_unlocked) { // key
                NF_MoveSprite(0, key.sprite_id, 0, 192);
            } else {
                updateObjectPosition(key.sprite_id, key.x, key.y, KEY_WIDTH, camera_x); 
            }
            updateObjectPosition(5, LEVELS[current_level].door_x, LEVELS[current_level].door_y, DOOR_WIDTH, camera_x); // door
            for (int i = 0; i < current_box_count; i++) {
                updateObjectPosition(boxes[i].sprite_id, boxes[i].x, boxes[i].y, BOX_WIDTH, camera_x); //box 0
            }
            for (int i=0; i < current_button_count; i++) {
                updateObjectPosition(buttons[i].sprite_id, buttons[i].x, buttons[i].y, 16, camera_x);
            }
            for (int i = 0; i < current_platform_count; i++) {
                updateObjectPosition(platforms[i].sprite_id, platforms[i].x, platforms[i].y, platforms[i].width, camera_x);
            }
            
    
            // Copy data from NFLib OAM buffers to the real OAM, wait for VBlank
            NF_SpriteOamSet(0);
            NF_SpriteOamSet(1);
            oamUpdate(&oamMain);
            oamUpdate(&oamSub);
        }

        swiWaitForVBlank();
    }

    // If this is reached, the program will return to the loader if the loader
    // supports it.
    return 0;
}
