#include "collisions.h"
#include <math.h>
#include "tilemap.h"

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