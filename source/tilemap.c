#include "globals.h"
#include <nf_lib.h>
#define COL_SOLID 2

bool isSolid(int x, int y) {
    if (y >= 192 || y < 0) return false;
    if (x < 0 || x >= current_level_width) return true;
    return NF_GetPoint(0, x, y) == COL_SOLID;
}


bool overlaps(float ax, float ay, float aw, float ah, float bx, float by, float bw, float bh) {
    return ax < bx + bw && ax + aw > bx && ay < by + bh && ay + ah > by;
}