#include "platform.h"
#include "globals.h"
#include "collisions.h"

void updatePlatforms(Platform *platforms, Player *players, Box *boxes) {
    for (int i=0; i < current_platform_count; i++) {
        Platform *p = &platforms[i];
        int start_x = p->x;
        if (p->active) {
            if (p->x < p->target_x) {
                p->x += p->speed;
                if (p->x > p->target_x) p->x = p->target_x;
            } else if (p->x > p->target_x) {
                p->x -= p->speed;
                if (p->x < p->target_x) p->x = p->target_x;
            }
            
        } else {
            if (p->x < p->start_x) {
                p->x += p->speed;
                if (p->x > p->start_x) p->x = p->start_x;
             } else if (p->x > p->start_x) {
                p->x -= p->speed;
                if (p->x < p->start_x) p->x = p->start_x;
            }
        }

        int displacement = p->x - start_x;
        if (displacement != 0) propagateMoveUp(p->object_on_top, p->object_on_top_id, displacement, players, boxes);
    }
}