/* pak_build: bake data.dat into web/assets/data.pak for the Emscripten build.
 *
 *   pak_build <data.dat> <out.pak>
 *
 * The web build has no Allegro to parse the datafile format, so we decode it
 * once here (with real Allegro) into a dead-simple blob: 32-bit bitmaps
 * (magenta = transparent), rasterized font glyph atlases, and s16 PCM samples.
 *
 * Format (all little-endian, which is every platform we care about):
 *   u32 'ZPAK'; u32 count
 *   count x entry:
 *     u8 type (0 other, 1 bmp, 2 font, 3 sample); u16 index
 *     bmp:  u16 w,h; w*h u32   (0x00RRGGBB, 0xFF00FF where transparent)
 *     font: u16 height; u16 nglyphs(=96, chars 32..127); per glyph u16 w,h; w*h u32
 *     samp: u32 freq; u32 nframes; u8 stereo; nframes*(1|2) s16
 *     other: nothing
 * A trailing entry type=2 index=0xFFFF holds Allegro's built-in `font`.
 */
#include <allegro.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

static FILE *g;
static void w8(int v){ fputc(v & 0xff, g); }
static void w16(int v){ fputc(v & 0xff, g); fputc((v >> 8) & 0xff, g); }
static void w32(unsigned v){ w16(v); w16(v >> 16); }

/* one glyph -> magenta-keyed 32-bit, using color=-1 for colour fonts and a
 * white fill for the mono built-in font (the shim tints mono at draw time). */
static void write_font(FONT *fnt, int mono)
{
    int h = text_height(fnt);
    w16(h);
    w16(96);
    BITMAP *tmp = create_bitmap_ex(32, 64, h + 4);
    for (int c = 32; c < 128; c++) {
        char s[2] = { (char)c, 0 };
        int gw = text_length(fnt, s);
        if (gw < 1) gw = 1;
        if (gw > 64) gw = 64;
        clear_to_color(tmp, makecol(255, 0, 255));
        textout_ex(tmp, fnt, s, 0, 0, mono ? makecol(255, 255, 255) : -1, -1);
        w16(gw); w16(h);
        for (int y = 0; y < h; y++)
            for (int x = 0; x < gw; x++) {
                int px = getpixel(tmp, x, y);
                int r = getr(px), gg = getg(px), b = getb(px);
                if (r == 255 && gg == 0 && b == 255) w32(0xFF00FF);
                else w32((r << 16) | (gg << 8) | b);
            }
    }
    destroy_bitmap(tmp);
}

int main(int argc, char *argv[])
{
    if (argc < 3) { printf("usage: %s <data.dat> <out.pak>\n", argv[0]); return 1; }
    if (allegro_init() != 0) { printf("allegro_init failed\n"); return 1; }
    set_color_depth(24);

    DATAFILE *df = load_datafile(argv[1]);
    if (!df) { printf("cannot load %s\n", argv[1]); return 1; }

    PALETTE pal;
    for (int i = 0; df[i].type != DAT_END; i++)
        if (df[i].type == DAT_PALETTE) { memcpy(pal, df[i].dat, sizeof(PALETTE)); break; }
    select_palette(pal);

    int count = 0;
    while (df[count].type != DAT_END) count++;

    g = fopen(argv[2], "wb");
    if (!g) { printf("cannot write %s\n", argv[2]); return 1; }
    fwrite("ZPAK", 1, 4, g);
    w32(count + 1);   /* +1 for the built-in font appended at the end */

    for (int i = 0; i < count; i++) {
        if (df[i].type == DAT_BITMAP) {
            BITMAP *src = (BITMAP *)df[i].dat;
            BITMAP *rgb = create_bitmap_ex(24, src->w, src->h);
            blit(src, rgb, 0, 0, 0, 0, src->w, src->h);   /* index -> real RGB */
            int mask = bitmap_mask_color(src);            /* transparent in src's depth */
            w8(1); w16(i); w16(src->w); w16(src->h);
            for (int y = 0; y < src->h; y++)
                for (int x = 0; x < src->w; x++) {
                    if (getpixel(src, x, y) == mask) { w32(0xFF00FF); continue; }
                    int px = getpixel(rgb, x, y);
                    w32((getr(px) << 16) | (getg(px) << 8) | getb(px));
                }
            destroy_bitmap(rgb);
        } else if (df[i].type == DAT_FONT) {
            w8(2); w16(i);
            write_font((FONT *)df[i].dat, 0);
        } else if (df[i].type == DAT_SAMPLE) {
            SAMPLE *s = (SAMPLE *)df[i].dat;
            int ch = s->stereo ? 2 : 1;
            w8(3); w16(i); w32(s->freq); w32(s->len); w8(s->stereo ? 1 : 0);
            long n = (long)s->len * ch;
            if (s->bits == 8) {
                unsigned char *d = (unsigned char *)s->data;
                for (long k = 0; k < n; k++) w16((short)((d[k] - 128) << 8));
            } else {
                unsigned short *d = (unsigned short *)s->data;
                for (long k = 0; k < n; k++) w16((short)(d[k] - 0x8000));
            }
        } else {
            w8(0); w16(i);
        }
    }

    w8(2); w16(0xFFFF);
    write_font(font, 1);   /* Allegro's built-in 8x8, mono */

    fclose(g);
    unload_datafile(df);
    printf("wrote %s (%d objects)\n", argv[2], count + 1);
    return 0;
}
