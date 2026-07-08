/* Minimal Allegro-4 API shim for the Emscripten/WASM build of Zombie Slaughter.
 * Implements only what the game (and PPCol) actually call, over SDL3. Compiled
 * INSTEAD of the real <allegro.h> on the web (native builds still use liballeg).
 * ponytail: this is the whole Allegro surface this game touches -- ~35 calls. */
#ifndef ALLEG_SHIM_H
#define ALLEG_SHIM_H
#ifdef __cplusplus
extern "C" {
#endif
#include <stdint.h>
#include <stddef.h>   /* NULL, size_t — real <allegro.h> transitively provides these */

#define TRUE 1
#define FALSE 0
#ifndef MIN
#define MIN(x,y) (((x)<(y))?(x):(y))
#define MAX(x,y) (((x)>(y))?(x):(y))
#define CLAMP(a,x,b) MAX((a),MIN((x),(b)))
#endif

typedef int32_t fixed;
/* cast to int32_t, not `fixed`: `using namespace std` in the game makes the
 * name `fixed` ambiguous with std::fixed at the macro expansion site. */
#define itofix(x) ((int32_t)((x) << 16))
#define ftofix(x) ((int32_t)((x) * 65536.0 + ((x) < 0 ? -0.5 : 0.5)))
#define fixtoi(x) ((int)((x) >> 16))

typedef struct BITMAP {
    int w, h;
    unsigned char **line;   /* line[y] -> row of w uint32 pixels (PPCol needs this) */
    uint32_t *dat;          /* backing store */
} BITMAP;

typedef struct RGB { unsigned char r, g, b, filler; } RGB;
typedef RGB PALETTE[256];

typedef struct FONT_GLYPH { int w, h; uint32_t *px; } FONT_GLYPH;
typedef struct FONT { int height; FONT_GLYPH glyph[96]; } FONT;

typedef struct SAMPLE {
    int freq, stereo;
    unsigned long len;      /* frames */
    int16_t *data;          /* interleaved s16 */
} SAMPLE;

typedef struct MIDI MIDI;
typedef struct DATAFILE { void *dat; int type; long size; void *prop; } DATAFILE;

/* --- globals ------------------------------------------------------------- */
#define KEY_MAX 128
extern volatile char key[KEY_MAX];
extern volatile int mouse_x, mouse_y, mouse_b, mouse_z;
extern BITMAP *screen;
extern FONT *font;
extern int al_scr_w, al_scr_h;
#define SCREEN_W al_scr_w
#define SCREEN_H al_scr_h

/* --- scancodes (values are internal; only self-consistency matters) ------ */
enum {
    KEY_A=1,KEY_B,KEY_C,KEY_D,KEY_E,KEY_F,KEY_G,KEY_H,KEY_I,KEY_J,KEY_K,KEY_L,
    KEY_M,KEY_N,KEY_O,KEY_P,KEY_Q,KEY_R,KEY_S,KEY_T,KEY_U,KEY_V,KEY_W,KEY_X,
    KEY_Y,KEY_Z,
    KEY_0,KEY_1,KEY_2,KEY_3,KEY_4,KEY_5,KEY_6,KEY_7,KEY_8,KEY_9,
    KEY_ESC,KEY_ENTER,KEY_SPACE,KEY_BACKSPACE,KEY_TAB,
    KEY_LEFT,KEY_RIGHT,KEY_UP,KEY_DOWN,
    KEY_LSHIFT,KEY_RSHIFT,KEY_LCONTROL,KEY_RCONTROL,KEY_ALT
};

#define GFX_AUTODETECT_WINDOWED 1
#define GFX_AUTODETECT_FULLSCREEN 2
#define DIGI_AUTODETECT -1
#define DIGI_NONE 0
#define MIDI_AUTODETECT -1
#define MIDI_NONE 0

/* interrupt/timer helpers become no-ops: the web main loop drives tick_counter */
#define LOCK_VARIABLE(v)
#define LOCK_FUNCTION(f)
#define END_OF_FUNCTION(f)
#define END_OF_MAIN()
#define BPS_TO_TIMER(x) (1000000L / (x))
#define MSEC_TO_TIMER(x) ((x) * 1000L)

#define bitmap_color_depth(bmp) 32
#define bitmap_mask_color(bmp) 0xFF00FF

/* --- system -------------------------------------------------------------- */
int allegro_init(void);
void install_keyboard(void);
void install_mouse(void);
void install_timer(void);
int install_sound(int digi, int midi, const char *cfg);
int install_int_ex(void (*proc)(void), long speed);
void override_config_data(const char *data, int len);
void set_color_depth(int depth);
int set_gfx_mode(int card, int w, int h, int vw, int vh);
void set_pallete(const RGB *p);
void set_palette(const RGB *p);
void rest(int ms);
int poll_keyboard(void);
int poll_mouse(void);
int keypressed(void);
int readkey(void);
void clear_keybuf(void);
void text_mode(int mode);

/* --- bitmaps / drawing --------------------------------------------------- */
BITMAP *create_bitmap(int w, int h);
void destroy_bitmap(BITMAP *b);
void acquire_bitmap(BITMAP *b);
void release_bitmap(BITMAP *b);
int makecol(int r, int g, int b);
void clear_to_color(BITMAP *b, int color);
void putpixel(BITMAP *b, int x, int y, int color);
int getpixel(BITMAP *b, int x, int y);
void blit(BITMAP *src, BITMAP *dst, int sx, int sy, int dx, int dy, int w, int h);
void draw_sprite(BITMAP *dst, BITMAP *src, int x, int y);
void draw_sprite_h_flip(BITMAP *dst, BITMAP *src, int x, int y);
void draw_sprite_v_flip(BITMAP *dst, BITMAP *src, int x, int y);
void rotate_sprite(BITMAP *dst, BITMAP *src, int x, int y, fixed angle);
void draw_trans_sprite(BITMAP *dst, BITMAP *src, int x, int y);
void set_add_blender(int r, int g, int b, int a);
void circlefill(BITMAP *b, int x, int y, int radius, int color);
void rectfill(BITMAP *b, int x1, int y1, int x2, int y2, int color);
void line(BITMAP *b, int x1, int y1, int x2, int y2, int color);
void vline(BITMAP *b, int x, int y1, int y2, int color);
void floodfill(BITMAP *b, int x, int y, int color);

/* --- text ---------------------------------------------------------------- */
int text_length(const FONT *f, const char *s);
int text_height(const FONT *f);
void textprintf(BITMAP *b, const FONT *f, int x, int y, int color, const char *fmt, ...);
void textprintf_centre(BITMAP *b, const FONT *f, int x, int y, int color, const char *fmt, ...);
void textprintf_right(BITMAP *b, const FONT *f, int x, int y, int color, const char *fmt, ...);

/* --- datafile / sound ---------------------------------------------------- */
DATAFILE *load_datafile(const char *path);
void unload_datafile(DATAFILE *d);
int play_sample(const SAMPLE *s, int vol, int pan, int freq, int loop);
void stop_sample(const SAMPLE *s);
void adjust_sample(const SAMPLE *s, int vol, int pan, int freq, int loop);

/* --- web loop bridge (called from main.cpp) ------------------------------ */
void al_web_pump(void);   /* pump SDL events, refresh input + tick_counter */

#ifdef __cplusplus
}
#endif
#endif
