/* Pixel-perfect collision for Allegro 4 BITMAPs.
 *
 * Drop-in reimplementation of the two functions Zombie Slaughter used from
 * Ivan Baldo's 1998 PPCol library (check_pp_collision / _amount, bitmap form).
 * The mask (PPCOL_MASK) API in ppcol.h is declared but never called by the
 * game, so it is intentionally not implemented here.
 *
 * ponytail: naive per-pixel scan over the bounding-box overlap. O(overlap area)
 * per test. Fine for a 640x480 game with a handful of entities. If it ever
 * shows up in a profile, precompute 32-bit row bitmasks like the original did.
 */
#include "ppcol.h"

int check_pp_collision_normal(BITMAP *s1, BITMAP *s2, int x1, int y1, int x2, int y2)
{
    /* overlap rectangle of the two bounding boxes, in world coords */
    int left   = MAX(x1, x2);
    int right  = MIN(x1 + s1->w, x2 + s2->w);
    int top    = MAX(y1, y2);
    int bottom = MIN(y1 + s1->h, y2 + s2->h);

    if (left >= right || top >= bottom)
        return 0;

    int m1 = bitmap_mask_color(s1);
    int m2 = bitmap_mask_color(s2);

    for (int y = top; y < bottom; y++)
        for (int x = left; x < right; x++)
            if (getpixel(s1, x - x1, y - y1) != m1 &&
                getpixel(s2, x - x2, y - y2) != m2)
                return 1;

    return 0;
}

/* Slide sprite 1 one pixel at a time in direction (addx,addy) until it no
 * longer collides; return the number of pixels moved. That is exactly what the
 * game wants: "how far to push out of the ground/wall in this direction". */
int check_pp_collision_amount_normal(BITMAP *s1, BITMAP *s2,
                                     int x1, int y1, int x2, int y2,
                                     int addx, int addy)
{
    int amount = 0;
    while (check_pp_collision_normal(s1, s2, x1, y1, x2, y2)) {
        x1 += addx;
        y1 += addy;
        if (++amount > 10000) break; /* safety: never spin forever */
    }
    return amount;
}
