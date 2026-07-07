#ifndef FBLEND_SHIM_H
#define FBLEND_SHIM_H
#include <allegro.h>

/* ponytail: the original game linked the external FBlend lib for one call,
   fblend_add(). Allegro 4's built-in add blender covers it in 2 lines, so we
   skip the dependency. Add real FBlend only if a profiler ever says this is hot. */
static inline void fblend_add(BITMAP *src, BITMAP *dst, int x, int y, int alpha)
{
    if (alpha < 0) alpha = 0;
    if (alpha > 255) alpha = 255;
    set_add_blender(0, 0, 0, alpha);
    draw_trans_sprite(dst, src, x, y);
}

#endif
