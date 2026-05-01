#include "box.h"
#include "tilemap.h"

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