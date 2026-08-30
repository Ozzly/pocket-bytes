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
#include "title_screen.h"
#include <dswifi9.h>

#define COL_EMPTY 3
#define PLAYER_DEATH_TIME 90

#define TOTAL_PLAYER_COLORS 8

#define RETURN_X 220
#define RETURN_Y 160
#define RETURN_WIDTH 24
#define RETURN_HEIGHT 24

static const char *PLAYER_COLOR_PALETTES[TOTAL_PLAYER_COLORS] = {
    "sprite/byte-mauve",
    "sprite/byte-saphire",
    "sprite/byte",
    "sprite/byte-bone",
    "sprite/byte-grey",
    "sprite/byte-pink",
    "sprite/byte-red",
    "sprite/byte-yellow",
};


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

typedef struct {
    u32 command;
} packet_host_to_client;

typedef struct {
    u32 keys_held;
} packet_client_to_host;


void checkReturnButton(GameState transition_to) {
    if (!(keysHeld() & KEY_TOUCH)) return;
    touchPosition touch_pos;
    touchRead(&touch_pos);
    if (touch_pos.px >= RETURN_X && 
        touch_pos.px <= RETURN_X + RETURN_WIDTH && 
        touch_pos.py >= RETURN_Y && 
        touch_pos.py <= RETURN_Y + RETURN_HEIGHT
    ) {
        state = transition_to;
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

    // Init Text
    NF_InitTextSys(1);

    // Init sprites
    NF_InitSpriteBuffers();
    NF_InitSpriteSys(0);
    NF_InitSpriteSys(1);
    // Load sprite files from NitroFS
    NF_LoadSpriteGfx("sprite/byte", 0, 32, 32);
    NF_VramSpriteGfx(0, 0, 0, false);

    NF_LoadSpriteGfx("sprite/return", 1, 32, 32);
    NF_VramSpriteGfx(1, 1, 1, false);
    NF_LoadSpritePal("sprite/return", 10);
    NF_VramSpritePal(1, 10, 10);


    // Load font
    NF_LoadTextFont("font/default", "normal", 256, 256, 0);
    NF_CreateTextLayer(1, 0, 0, "normal");

    // Set font color
    NF_DefineTextColor(1, 0, 1, 0, 0, 0); // black
    NF_SetTextColor(1, 0, 1);


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


    int color_highlighted = 0;
    int player_selected_colors[MAX_PLAYERS];
    int player_selecting_color = 0;
    int available_colors[TOTAL_PLAYER_COLORS];
    int available_colors_count = TOTAL_PLAYER_COLORS;

    while (1)
    {

        if (state == STATE_TITLE) {

            // Setup title screen on 1st load
            if (entering_state) {
                NF_LoadTiledBg("bg/title-top", "title-top", 256, 256);
                NF_CreateTiledBg(0, 3, "title-top");
                NF_LoadCollisionBg("collision/title-top-col", 0, 256, 200);
                current_level_width = 256;
                NF_LoadTiledBg("bg/select-mode-singleplayer", "mode-singleplayer", 256, 256);
                NF_LoadTiledBg("bg/select-mode-multiplayer", "mode-multiplayer", 256, 256);
                NF_LoadTiledBg("bg/select-mode-options", "mode-options", 256, 256);
                NF_CreateTiledBg(1, 3, "mode-options");
                NF_CreateTiledBg(1, 2, "mode-multiplayer");
                NF_CreateTiledBg(1, 1, "mode-singleplayer");
                NF_HideBg(1, 2);
                NF_HideBg(1, 3);

                createDemoPlayers();
                // initDemoKey();

                entering_state = false;

                
            }
            updateDemoPlayers();
            // updateDemoKey();

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
                        NF_ShowBg(1, 1);
                        NF_HideBg(1, 2);
                        NF_HideBg(1, 3);
                        break;
                    case MULTIPLAYER:
                        NF_ShowBg(1, 2);
                        NF_HideBg(1, 1);
                        NF_HideBg(1, 3);
                        break;
                    case OPTIONS:
                        NF_ShowBg(1, 3);
                        NF_HideBg(1, 1);
                        NF_HideBg(1, 2);
                        break;
                }
            }

            // Select option
            if (option_selected) {
                entering_state = true;
                unloadTitleScreen();
                if (menu_option == SINGLEPLAYER) {
                    current_player_count = 2;
                    state = STATE_SINGLEPLAYER_COLOR_SELECT;

                    // Unload bottom background
                    // NF_UnloadTiledBg("mode-singleplayer");
                    // NF_UnloadTiledBg("mode-multiplayer");
                    // NF_UnloadTiledBg("mode-options");
                    // NF_DeleteTiledBg(1, 0);
                    // NF_DeleteTiledBg(1, 1);
                    // NF_DeleteTiledBg(1, 2); 


                    // NF_UnloadSpritePal(0);
                    // NF_UnloadSpritePal(1);
                    // NF_UnloadSpritePal(2);


                } else if (menu_option == MULTIPLAYER) {
                    current_player_count = 2;
                    state = STATE_MULTIPLAYER_JOIN;
                    
                } else if (menu_option == OPTIONS) {
                }
            }
        }

        if (state == STATE_SINGLEPLAYER_COLOR_SELECT) {
            if (entering_state) {
                // Load background
                NF_LoadTiledBg("bg/player-color-select", "player-color-select", 256, 256);
                NF_CreateTiledBg(1, 3, "player-color-select");

                

                entering_state = false;

 
                for (int i=0; i < TOTAL_PLAYER_COLORS; i++) {
                    NF_LoadSpritePal(PLAYER_COLOR_PALETTES[i], i);
                    NF_VramSpritePal(1, i, i);
                }


                NF_VramSpriteGfx(1, 0, 0, false);
                for (int i=0; i < TOTAL_PLAYER_COLORS; i++) {
                    NF_CreateSprite(1, SPRITE_BASE_COLOR_SELECTION + i, GFX_SLOT_PLAYER, i, 112 + i * 30, 80);
                }

                color_highlighted = 0;

                for (int i=0; i < TOTAL_PLAYER_COLORS; i++) {
                    available_colors[i] = i;
                }


            }

            // Write string prompting player X to select color
            char string_select_player_color[32];

            if (player_selecting_color == 0) {
                snprintf(string_select_player_color, sizeof(string_select_player_color), "Select Player 1's Color");
            } else if (player_selecting_color == 1) {
                snprintf(string_select_player_color, sizeof(string_select_player_color), "Select Player 2's Color");
            }
            NF_WriteText(1, 0, 4, 5, string_select_player_color);

            NF_UpdateTextLayers();
            // Detect key movements
            scanKeys();
            u16 keys = keysHeld();
            u16 keys_down = keysDown();

            



            if (keys_down & KEY_RIGHT) {
                color_highlighted++;
                if (color_highlighted >= available_colors_count) color_highlighted = available_colors_count - 1;
            } else if (keys_down & KEY_LEFT) {
                color_highlighted--;
                if (color_highlighted < 0) color_highlighted = 0;
            } else if ((keys_down & KEY_A) && available_colors_count > 0) {
                int chosen_color = available_colors[color_highlighted];
                player_selected_colors[player_selecting_color] = chosen_color;
                player_selecting_color++;

                NF_MoveSprite(1, SPRITE_BASE_COLOR_SELECTION + chosen_color, SCREEN_WIDTH, SCREEN_HEIGHT + 10);

                for (int i = color_highlighted; i < available_colors_count - 1; i++) {
                    available_colors[i] = available_colors[i+1];
                }
                available_colors_count--;

                if (color_highlighted >= available_colors_count) {
                    color_highlighted = available_colors_count > 0 ? available_colors_count - 1 : 0;
                }
            }


            // Exit player color selection
            if (player_selecting_color >= current_player_count) {
                entering_state = true;
                state = STATE_PLAYING;

                for (int i=0; i < TOTAL_PLAYER_COLORS; i++) {
                    NF_UnloadSpritePal(i);
                    NF_DeleteSprite(1, SPRITE_BASE_COLOR_SELECTION + i);
                }

                NF_UnloadTiledBg("player-color-select");
            }

            for (int i=0; i < available_colors_count; i++) {
                NF_MoveSprite(1, SPRITE_BASE_COLOR_SELECTION + available_colors[i], 112 + 30 * (i - color_highlighted), 80);
            }
            
            

        }

        if (state == STATE_MULTIPLAYER_JOIN) {

            if (entering_state) {
                NF_LoadTiledBg("bg/host-client-select", "host-client-select", 256, 256);
                NF_CreateTiledBg(1, 3, "host-client-select");

                if (!Wifi_InitDefault(INIT_ONLY | WIFI_LOCAL_ONLY)) {
                    consoleDemoInit();
                    printf("Wifi no worke"); 
                }

                // Return sprite
                NF_CreateSprite(1, 1, 1, 10, 220, 160);

                entering_state = false;
            }
            

            // Detect if entering host or client mode
            scanKeys();
            u16 keys_held = keysHeld();

            if (keys_held & KEY_TOUCH) {
                touchRead(&touch_pos);
                if (touch_pos.px > 31 && touch_pos.px < 224) {
                    if (touch_pos.py >= 24 && touch_pos.py <= 84) {
                        // host mode
                        state = STATE_MULTIPLAYER_HOST;
                    } else if (touch_pos.px >= 112 && touch_pos.py <= 172) {
                        // client mode
                        state = STATE_MULTIPLAYER_CLIENT;
                    }
                    
                }

                
            }

            // Return
            checkReturnButton(STATE_TITLE);

            // Cleanup on exit
            if (state != STATE_MULTIPLAYER_JOIN) {
                entering_state = true;
                NF_UnloadTiledBg("host-client-select");
                NF_DeleteTiledBg(1, 3);
                NF_DeleteSprite(1, 1);

                Wifi_DisableWifi();
                Wifi_Deinit();
            }

        }

        if (state == STATE_MULTIPLAYER_HOST) {
            // host mode active
            if (entering_state) {
                // Set background
                NF_LoadTiledBg("bg/host-list", "host-list", 256, 256);
                NF_CreateTiledBg(1, 3, "host-list");

                entering_state = false;
                
                // Start Wifi host mode                
                Wifi_MultiplayerHostMode(MAX_PLAYERS, sizeof(packet_host_to_client), sizeof(packet_client_to_host));

                // Wait for library to enter host mode
                while (!Wifi_LibraryModeReady()) swiWaitForVBlank();

                Wifi_SetChannel(6);
                Wifi_MultiplayerAllowNewClients(true);
                Wifi_BeaconStart("NintendoDS", 0xABCDEF01);
            }

            NF_ClearTextLayer(1, 0);
            NF_UpdateTextLayers();

            int num_clients = Wifi_MultiplayerGetNumClients();
            u16 players_mask = Wifi_MultiplayerGetClientMask();
            char player_count_string[16];

            snprintf(player_count_string, sizeof(player_count_string), "Clients: %d", num_clients);

            NF_WriteText(1, 0, 4, 6, player_count_string);


            Wifi_ConnectedClient client[4];
            num_clients = Wifi_MultiplayerGetClients(4, &(client[0]));

            

            for (int i=0; i < num_clients; i++) {
                char client_info[32];
                snprintf(client_info, sizeof(client_info), "AID %d (State %d) %04X", client[i].association_id, client[i].state, client[i].macaddr[2]);
    
                NF_WriteText(1, 0, 4, i+7, client_info);
            }

            for (int i=num_clients; i < MAX_PLAYERS; i++) {
                NF_WriteText(1, 0, 4, i+7, "                  ");
            }

            NF_UpdateTextLayers();

           
            

        }

        if (state == STATE_MULTIPLAYER_CLIENT) {
            if (entering_state) {
                entering_state = false;

                NF_LoadTiledBg("bg/client", "client", 256, 256);
                NF_CreateTiledBg(1, 3, "client");
            
                Wifi_MultiplayerClientMode(sizeof(packet_client_to_host));

                // Wait for library to enter client mode
                while (!Wifi_LibraryModeReady()) swiWaitForVBlank();

                Wifi_ScanMode();
                // Wait for wifi to setup for a few frames
                for (int i=0; i < 5; i++) swiWaitForVBlank();
            }

            int num_ap = Wifi_GetNumAP();
            // Autoconnect to the first valid ap
            if (num_ap > 0) {
                Wifi_AccessPoint ap;
                Wifi_GetAPData(0, &ap);

                Wifi_ConnectOpenAP(&ap);

                while (true) {
                    swiWaitForVBlank();
                    int status = Wifi_AssocStatus();

                    if (status == ASSOCSTATUS_CANNOTCONNECT) {
                        // fail
                        NF_WriteText(1, 0, 10, 10, "CONNECTION FAILED");
                        NF_UpdateTextLayers();
                        break;
                    }
                    if (status == ASSOCSTATUS_ASSOCIATED) {
                        entering_state = true;
                        state = STATE_MULTIPLAYER_CLIENT_CONNECTED;
                        break;
                    }
                }
            }
        }
        
        if (state == STATE_MULTIPLAYER_CLIENT_CONNECTED) {
            if (entering_state) {
                entering_state = false;
            }
            NF_WriteText(1, 0, 1, 5, "CONNECTED!!");
            NF_UpdateTextLayers();
        }


        if (state == STATE_PLAYING) {

            if (entering_state) {

                for (int i = 0; i < current_player_count; i++) {
                    NF_LoadSpritePal(PLAYER_COLOR_PALETTES[player_selected_colors[i]], PAL_SLOT_PLAYER_BASE + i);
                    NF_VramSpritePal(0, PAL_SLOT_PLAYER_BASE + i, PAL_SLOT_PLAYER_BASE + i);
                }


                


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
            
    
        }
        
        // Copy data from NFLib OAM buffers to the real OAM, wait for VBlank
        NF_SpriteOamSet(0);
        NF_SpriteOamSet(1);
        oamUpdate(&oamMain);
        oamUpdate(&oamSub);
        swiWaitForVBlank();
    }

    // If this is reached, the program will return to the loader if the loader
    // supports it.
    return 0;
}
