/* Allegro-4 -> SDL3 compatibility shim for the Emscripten build. See allegro.h.
 * Everything the game draws goes into 32-bit memory BITMAPs (software raster);
 * SDL3 only provides the window (present the `screen` bitmap), input, audio. */
#include "allegro.h"
#include <SDL3/SDL.h>
#include <emscripten.h>
#include <emscripten/html5.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <math.h>

#define MASK 0xFF00FFu

/* --- globals ------------------------------------------------------------- */
extern "C" {
volatile char key[KEY_MAX];
volatile int mouse_x, mouse_y, mouse_b, mouse_z;
BITMAP *screen;
FONT *font;
int al_scr_w = 640, al_scr_h = 480;
}
extern volatile int tick_counter;   /* owned by the game (fps.h) */

static SDL_Window *g_win;
static SDL_Renderer *g_ren;
static SDL_Texture *g_tex;
static int g_add_alpha = 255;       /* current add-blender alpha */

/* --- bitmaps ------------------------------------------------------------- */
BITMAP *create_bitmap(int w, int h)
{
    if (w < 1) w = 1; if (h < 1) h = 1;
    BITMAP *b = (BITMAP *)malloc(sizeof(BITMAP));
    b->w = w; b->h = h;
    b->dat = (uint32_t *)malloc((size_t)w * h * 4);
    b->line = (unsigned char **)malloc((size_t)h * sizeof(unsigned char *));
    for (int y = 0; y < h; y++) b->line[y] = (unsigned char *)(b->dat + (size_t)y * w);
    memset(b->dat, 0, (size_t)w * h * 4);
    return b;
}
void destroy_bitmap(BITMAP *b) { if (!b) return; free(b->dat); free(b->line); free(b); }
void acquire_bitmap(BITMAP *b) { (void)b; }
void release_bitmap(BITMAP *b) { (void)b; }

int makecol(int r, int g, int b) { return ((r & 0xff) << 16) | ((g & 0xff) << 8) | (b & 0xff); }

static inline uint32_t *px(BITMAP *b, int x, int y) { return (uint32_t *)b->line[y] + x; }
static inline int in(BITMAP *b, int x, int y) { return x >= 0 && y >= 0 && x < b->w && y < b->h; }

void putpixel(BITMAP *b, int x, int y, int color) { if (in(b, x, y)) *px(b, x, y) = (uint32_t)color; }
int getpixel(BITMAP *b, int x, int y) { return in(b, x, y) ? (int)*px(b, x, y) : -1; }

void clear_to_color(BITMAP *b, int color)
{
    uint32_t c = (uint32_t)color, n = (uint32_t)b->w * b->h;
    for (uint32_t i = 0; i < n; i++) b->dat[i] = c;
}

/* --- present ------------------------------------------------------------- */
static void present(void)
{
    if (!g_tex) return;
    SDL_UpdateTexture(g_tex, NULL, screen->dat, screen->w * 4);
    SDL_RenderClear(g_ren);
    SDL_RenderTexture(g_ren, g_tex, NULL, NULL);
    SDL_RenderPresent(g_ren);
}

void blit(BITMAP *src, BITMAP *dst, int sx, int sy, int dx, int dy, int w, int h)
{
    for (int y = 0; y < h; y++)
        for (int x = 0; x < w; x++) {
            if (!in(src, sx + x, sy + y) || !in(dst, dx + x, dy + y)) continue;
            *px(dst, dx + x, dy + y) = *px(src, sx + x, sy + y);
        }
    if (dst == screen) present();
}

void draw_sprite(BITMAP *dst, BITMAP *src, int x, int y)
{
    for (int sy = 0; sy < src->h; sy++)
        for (int sx = 0; sx < src->w; sx++) {
            uint32_t c = *px(src, sx, sy);
            if (c == MASK) continue;
            if (in(dst, x + sx, y + sy)) *px(dst, x + sx, y + sy) = c;
        }
}
void draw_sprite_h_flip(BITMAP *dst, BITMAP *src, int x, int y)
{
    for (int sy = 0; sy < src->h; sy++)
        for (int sx = 0; sx < src->w; sx++) {
            uint32_t c = *px(src, sx, sy);
            if (c == MASK) continue;
            if (in(dst, x + src->w - 1 - sx, y + sy)) *px(dst, x + src->w - 1 - sx, y + sy) = c;
        }
}
void draw_sprite_v_flip(BITMAP *dst, BITMAP *src, int x, int y)
{
    for (int sy = 0; sy < src->h; sy++)
        for (int sx = 0; sx < src->w; sx++) {
            uint32_t c = *px(src, sx, sy);
            if (c == MASK) continue;
            if (in(dst, x + sx, y + src->h - 1 - sy)) *px(dst, x + sx, y + src->h - 1 - sy) = c;
        }
}

void set_add_blender(int r, int g, int b, int a) { (void)r; (void)g; (void)b; g_add_alpha = a; }

void draw_trans_sprite(BITMAP *dst, BITMAP *src, int x, int y)
{
    int a = g_add_alpha; if (a < 0) a = 0; if (a > 255) a = 255;
    for (int sy = 0; sy < src->h; sy++)
        for (int sx = 0; sx < src->w; sx++) {
            uint32_t c = *px(src, sx, sy);
            if (c == MASK || !in(dst, x + sx, y + sy)) continue;
            uint32_t *d = px(dst, x + sx, y + sy);
            int sr = (c >> 16) & 0xff, sg = (c >> 8) & 0xff, sb = c & 0xff;
            int dr = (*d >> 16) & 0xff, dg = (*d >> 8) & 0xff, db = *d & 0xff;
            dr += sr * a / 255; dg += sg * a / 255; db += sb * a / 255;
            if (dr > 255) dr = 255; if (dg > 255) dg = 255; if (db > 255) db = 255;
            *d = (dr << 16) | (dg << 8) | db;
        }
}

void rotate_sprite(BITMAP *dst, BITMAP *src, int x, int y, fixed angle)
{
    double rad = (double)angle * (M_PI / 8388608.0);   /* Allegro: 256*65536 = full turn */
    double c = cos(rad), s = sin(rad);
    double cx = x + src->w / 2.0, cy = y + src->h / 2.0;
    int rad_px = (int)ceil(sqrt((double)src->w * src->w + (double)src->h * src->h) / 2.0) + 1;
    for (int oy = (int)cy - rad_px; oy <= (int)cy + rad_px; oy++)
        for (int ox = (int)cx - rad_px; ox <= (int)cx + rad_px; ox++) {
            if (!in(dst, ox, oy)) continue;
            double dxp = ox - cx, dyp = oy - cy;
            int sxp = (int)lround(c * dxp + s * dyp + src->w / 2.0);
            int syp = (int)lround(-s * dxp + c * dyp + src->h / 2.0);
            if (sxp < 0 || syp < 0 || sxp >= src->w || syp >= src->h) continue;
            uint32_t col = *px(src, sxp, syp);
            if (col == MASK) continue;
            *px(dst, ox, oy) = col;
        }
}

/* --- primitives ---------------------------------------------------------- */
void vline(BITMAP *b, int x, int y1, int y2, int color)
{
    if (y1 > y2) { int t = y1; y1 = y2; y2 = t; }
    for (int y = y1; y <= y2; y++) putpixel(b, x, y, color);
}
void rectfill(BITMAP *b, int x1, int y1, int x2, int y2, int color)
{
    if (x1 > x2) { int t = x1; x1 = x2; x2 = t; }
    if (y1 > y2) { int t = y1; y1 = y2; y2 = t; }
    for (int y = y1; y <= y2; y++)
        for (int x = x1; x <= x2; x++) putpixel(b, x, y, color);
}
void circlefill(BITMAP *b, int cx, int cy, int r, int color)
{
    for (int y = -r; y <= r; y++)
        for (int x = -r; x <= r; x++)
            if (x * x + y * y <= r * r) putpixel(b, cx + x, cy + y, color);
}
void line(BITMAP *b, int x1, int y1, int x2, int y2, int color)
{
    int dx = abs(x2 - x1), dy = -abs(y2 - y1);
    int sx = x1 < x2 ? 1 : -1, sy = y1 < y2 ? 1 : -1, err = dx + dy;
    for (;;) {
        putpixel(b, x1, y1, color);
        if (x1 == x2 && y1 == y2) break;
        int e2 = 2 * err;
        if (e2 >= dy) { err += dy; x1 += sx; }
        if (e2 <= dx) { err += dx; y1 += sy; }
    }
}
void floodfill(BITMAP *b, int x, int y, int color)
{
    if (!in(b, x, y)) return;
    uint32_t target = *px(b, x, y), repl = (uint32_t)color;
    if (target == repl) return;
    /* scanline stack flood */
    int cap = 1024, top = 0;
    int *stk = (int *)malloc(cap * 2 * sizeof(int));
    stk[top * 2] = x; stk[top * 2 + 1] = y; top++;
    while (top > 0) {
        top--; int px0 = stk[top * 2], py = stk[top * 2 + 1];
        if (!in(b, px0, py) || *px(b, px0, py) != target) continue;
        int xl = px0, xr = px0;
        while (xl > 0 && *px(b, xl - 1, py) == target) xl--;
        while (xr < b->w - 1 && *px(b, xr + 1, py) == target) xr++;
        for (int xx = xl; xx <= xr; xx++) {
            *px(b, xx, py) = repl;
            for (int ny = py - 1; ny <= py + 1; ny += 2) {
                if (in(b, xx, ny) && *px(b, xx, ny) == target) {
                    if (top >= cap) { cap *= 2; stk = (int *)realloc(stk, cap * 2 * sizeof(int)); }
                    stk[top * 2] = xx; stk[top * 2 + 1] = ny; top++;
                }
            }
        }
    }
    free(stk);
}

/* --- text ---------------------------------------------------------------- */
int text_height(const FONT *f) { return f ? f->height : 8; }
int text_length(const FONT *f, const char *s)
{
    if (!f) return 0;
    int len = 0;
    for (; *s; s++) { int c = (unsigned char)*s; if (c >= 32 && c < 128) len += f->glyph[c - 32].w; }
    return len;
}
static void draw_text(BITMAP *b, const FONT *f, int x, int y, int color, const char *s)
{
    if (!f) return;
    for (; *s; s++) {
        int c = (unsigned char)*s;
        if (c < 32 || c >= 128) continue;
        const FONT_GLYPH *g = &f->glyph[c - 32];
        for (int gy = 0; gy < g->h; gy++)
            for (int gx = 0; gx < g->w; gx++) {
                uint32_t col = g->px[gy * g->w + gx];
                if (col == MASK || !in(b, x + gx, y + gy)) continue;
                *px(b, x + gx, y + gy) = (color == -1) ? col : (uint32_t)color;
            }
        x += g->w;
    }
}
void textprintf(BITMAP *b, const FONT *f, int x, int y, int color, const char *fmt, ...)
{
    char buf[512]; va_list a; va_start(a, fmt); vsnprintf(buf, sizeof buf, fmt, a); va_end(a);
    draw_text(b, f, x, y, color, buf);
}
void textprintf_centre(BITMAP *b, const FONT *f, int x, int y, int color, const char *fmt, ...)
{
    char buf[512]; va_list a; va_start(a, fmt); vsnprintf(buf, sizeof buf, fmt, a); va_end(a);
    draw_text(b, f, x - text_length(f, buf) / 2, y, color, buf);
}
void textprintf_right(BITMAP *b, const FONT *f, int x, int y, int color, const char *fmt, ...)
{
    char buf[512]; va_list a; va_start(a, fmt); vsnprintf(buf, sizeof buf, fmt, a); va_end(a);
    draw_text(b, f, x - text_length(f, buf), y, color, buf);
}

/* --- datafile (reads /data.pak baked by tools/pak_build) ----------------- */
static const unsigned char *g_pak;
static size_t g_cur;
static uint32_t rd8(void)  { return g_pak[g_cur++]; }
static uint32_t rd16(void) { uint32_t v = g_pak[g_cur] | (g_pak[g_cur + 1] << 8); g_cur += 2; return v; }
static uint32_t rd32(void) { uint32_t v = rd16(); return v | (rd16() << 16); }

static uint32_t *read_pixels(int w, int h)
{
    uint32_t *p = (uint32_t *)malloc((size_t)w * h * 4);
    for (int i = 0; i < w * h; i++) p[i] = rd32();
    return p;
}

DATAFILE *load_datafile(const char *path)
{
    (void)path;
    FILE *f = fopen("/data.pak", "rb");
    if (!f) { fprintf(stderr, "data.pak missing\n"); return NULL; }
    fseek(f, 0, SEEK_END); long n = ftell(f); fseek(f, 0, SEEK_SET);
    unsigned char *buf = (unsigned char *)malloc(n);
    fread(buf, 1, n, f); fclose(f);
    g_pak = buf; g_cur = 0;
    if (memcmp(g_pak, "ZPAK", 4) != 0) { fprintf(stderr, "bad pak\n"); return NULL; }
    g_cur = 4;
    uint32_t count = rd32();

    DATAFILE *d = (DATAFILE *)calloc(256, sizeof(DATAFILE));
    for (uint32_t e = 0; e < count; e++) {
        int type = rd8(); int idx = rd16();
        if (type == 1) {                        /* bitmap */
            int w = rd16(), h = rd16();
            BITMAP *b = (BITMAP *)malloc(sizeof(BITMAP));
            b->w = w; b->h = h; b->dat = read_pixels(w, h);
            b->line = (unsigned char **)malloc((size_t)h * sizeof(unsigned char *));
            for (int y = 0; y < h; y++) b->line[y] = (unsigned char *)(b->dat + (size_t)y * w);
            d[idx].dat = b; d[idx].type = 1;
        } else if (type == 2) {                 /* font */
            FONT *ft = (FONT *)calloc(1, sizeof(FONT));
            ft->height = rd16(); int ng = rd16();
            for (int gi = 0; gi < ng && gi < 96; gi++) {
                int gw = rd16(), gh = rd16();
                ft->glyph[gi].w = gw; ft->glyph[gi].h = gh; ft->glyph[gi].px = read_pixels(gw, gh);
            }
            if (idx == 0xFFFF) font = ft; else { d[idx].dat = ft; d[idx].type = 2; }
        } else if (type == 3) {                 /* sample */
            int freq = rd32(), frames = rd32(), stereo = rd8();
            SAMPLE *s = (SAMPLE *)malloc(sizeof(SAMPLE));
            s->freq = freq; s->stereo = stereo; s->len = frames;
            int ns = frames * (stereo ? 2 : 1);
            s->data = (int16_t *)malloc(ns * 2);
            for (int i = 0; i < ns; i++) s->data[i] = (int16_t)rd16();
            d[idx].dat = s; d[idx].type = 3;
        }
    }
    return d;
}
void unload_datafile(DATAFILE *d) { (void)d; }   /* one-shot lifetime under emscripten */

/* --- audio mixer --------------------------------------------------------- */
#define NVOICE 16
#define ARATE 44100
struct Voice { const SAMPLE *s; double pos, rate; int vol, pan, loop, active; };
static Voice g_voice[NVOICE];
static SDL_AudioStream *g_astream;

static void audio_cb(void *ud, SDL_AudioStream *stream, int need, int total)
{
    (void)ud; (void)total;
    static int16_t out[4096 * 2];
    while (need > 0) {
        int frames = need / 4; if (frames > 4096) frames = 4096;
        for (int i = 0; i < frames; i++) {
            float l = 0, r = 0;
            for (int v = 0; v < NVOICE; v++) {
                Voice *vo = &g_voice[v];
                if (!vo->active) continue;
                long idx = (long)vo->pos;
                if (idx >= (long)vo->s->len) {
                    if (vo->loop) { vo->pos = 0; idx = 0; } else { vo->active = 0; continue; }
                }
                float ls, rs;
                if (vo->s->stereo) { ls = vo->s->data[idx * 2] / 32768.0f; rs = vo->s->data[idx * 2 + 1] / 32768.0f; }
                else ls = rs = vo->s->data[idx] / 32768.0f;
                float g = vo->vol / 255.0f;
                float lf = (255 - vo->pan) / 128.0f; if (lf > 1) lf = 1;
                float rf = vo->pan / 128.0f; if (rf > 1) rf = 1;
                l += ls * g * lf; r += rs * g * rf;
                vo->pos += vo->rate;
            }
            if (l > 1) l = 1; if (l < -1) l = -1;
            if (r > 1) r = 1; if (r < -1) r = -1;
            out[i * 2] = (int16_t)(l * 32767); out[i * 2 + 1] = (int16_t)(r * 32767);
        }
        SDL_PutAudioStreamData(stream, out, frames * 4);
        need -= frames * 4;
    }
}

int install_sound(int digi, int midi, const char *cfg)
{
    (void)digi; (void)midi; (void)cfg;
    SDL_AudioSpec spec; spec.freq = ARATE; spec.format = SDL_AUDIO_S16; spec.channels = 2;
    g_astream = SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &spec, audio_cb, NULL);
    if (g_astream) SDL_ResumeAudioStreamDevice(g_astream);
    return 0;
}
int play_sample(const SAMPLE *s, int vol, int pan, int freq, int loop)
{
    int slot = -1;
    for (int v = 0; v < NVOICE; v++) if (!g_voice[v].active) { slot = v; break; }
    if (slot < 0) slot = 0;
    Voice *vo = &g_voice[slot];
    vo->s = s; vo->pos = 0; vo->rate = (double)s->freq * freq / 1000.0 / ARATE;
    vo->vol = vol; vo->pan = pan; vo->loop = loop; vo->active = 1;
    return slot;
}
void adjust_sample(const SAMPLE *s, int vol, int pan, int freq, int loop)
{
    for (int v = 0; v < NVOICE; v++)
        if (g_voice[v].active && g_voice[v].s == s) {
            g_voice[v].vol = vol; g_voice[v].pan = pan; g_voice[v].loop = loop;
            g_voice[v].rate = (double)s->freq * freq / 1000.0 / ARATE;
        }
}
void stop_sample(const SAMPLE *s)
{
    for (int v = 0; v < NVOICE; v++) if (g_voice[v].s == s) g_voice[v].active = 0;
}

/* --- system / gfx / input ------------------------------------------------ */
/* keyboard event queue for readkey()/keypressed() */
static int g_kq[64], g_khead, g_ktail;

/* DOM KeyboardEvent.code -> our KEY_* (SDL's emscripten keyboard capture is
 * focus-dependent and unreliable, so we handle keys via the HTML5 API on the
 * document instead). */
static int code_to_key(const char *c)
{
    if (c[0] == 'K' && c[1] == 'e' && c[2] == 'y' && c[3] && !c[4]) return KEY_A + (c[3] - 'A');
    if (c[0] == 'D' && !strncmp(c, "Digit", 5) && c[5] && !c[6]) {
        int d = c[5] - '0'; return d == 0 ? KEY_0 : KEY_1 + (d - 1);
    }
    if (!strcmp(c, "ArrowLeft")) return KEY_LEFT;
    if (!strcmp(c, "ArrowRight")) return KEY_RIGHT;
    if (!strcmp(c, "ArrowUp")) return KEY_UP;
    if (!strcmp(c, "ArrowDown")) return KEY_DOWN;
    if (!strcmp(c, "Enter") || !strcmp(c, "NumpadEnter")) return KEY_ENTER;
    if (!strcmp(c, "Space")) return KEY_SPACE;
    if (!strcmp(c, "Escape")) return KEY_ESC;
    if (!strcmp(c, "Backspace")) return KEY_BACKSPACE;
    if (!strcmp(c, "Tab")) return KEY_TAB;
    if (!strcmp(c, "ShiftLeft")) return KEY_LSHIFT;
    if (!strcmp(c, "ShiftRight")) return KEY_RSHIFT;
    if (!strcmp(c, "ControlLeft")) return KEY_LCONTROL;
    if (!strcmp(c, "ControlRight")) return KEY_RCONTROL;
    if (!strcmp(c, "AltLeft") || !strcmp(c, "AltRight")) return KEY_ALT;
    return 0;
}

static EM_BOOL on_key(int type, const EmscriptenKeyboardEvent *e, void *ud)
{
    (void)ud;
    int k = code_to_key(e->code);
    if (type == EMSCRIPTEN_EVENT_KEYUP) { if (k) key[k] = 0; return EM_TRUE; }
    if (k) key[k] = 1;
    int ascii = (e->key[0] && !e->key[1]) ? (unsigned char)e->key[0] : 0;   /* single char */
    int nh = (g_khead + 1) & 63;
    if (nh != g_ktail) { g_kq[g_khead] = (k << 8) | ascii; g_khead = nh; }
    return EM_TRUE;   /* consume so arrows/space don't scroll the page */
}

int allegro_init(void)
{
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
    emscripten_set_keydown_callback(EMSCRIPTEN_EVENT_TARGET_DOCUMENT, NULL, 1, on_key);
    emscripten_set_keyup_callback(EMSCRIPTEN_EVENT_TARGET_DOCUMENT, NULL, 1, on_key);
    return 0;
}
void install_keyboard(void) {}
void install_mouse(void) {}
void install_timer(void) {}
int install_int_ex(void (*proc)(void), long speed) { (void)proc; (void)speed; return 0; }
void override_config_data(const char *data, int len) { (void)data; (void)len; }
void set_color_depth(int depth) { (void)depth; }         /* always 32-bit internally */
void set_pallete(const RGB *p) { (void)p; }
void set_palette(const RGB *p) { (void)p; }
void rest(int ms) { (void)ms; }                          /* main loop yields to browser */
int poll_keyboard(void) { return 0; }
int poll_mouse(void) { return 0; }

int set_gfx_mode(int card, int w, int h, int vw, int vh)
{
    (void)card; (void)vw; (void)vh;
    al_scr_w = w; al_scr_h = h;
    SDL_CreateWindowAndRenderer("Zombie Slaughter", w, h, 0, &g_win, &g_ren);
    SDL_SetRenderLogicalPresentation(g_ren, w, h, SDL_LOGICAL_PRESENTATION_LETTERBOX);
    g_tex = SDL_CreateTexture(g_ren, SDL_PIXELFORMAT_XRGB8888, SDL_TEXTUREACCESS_STREAMING, w, h);
    SDL_SetTextureScaleMode(g_tex, SDL_SCALEMODE_NEAREST);
    screen = create_bitmap(w, h);
    return 0;
}

int keypressed(void) { return g_khead != g_ktail; }
int readkey(void) { if (g_khead == g_ktail) return 0; int v = g_kq[g_ktail]; g_ktail = (g_ktail + 1) & 63; return v; }
void clear_keybuf(void) { g_khead = g_ktail = 0; }
void text_mode(int mode) { (void)mode; }   /* text bg is always transparent (magenta-keyed) */

void al_web_pump(void)
{
    static int audio_kicked = 0;
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        switch (e.type) {
        case SDL_EVENT_MOUSE_MOTION:
            SDL_ConvertEventToRenderCoordinates(g_ren, &e);
            mouse_x = (int)e.motion.x; mouse_y = (int)e.motion.y; break;
        case SDL_EVENT_MOUSE_BUTTON_DOWN:
        case SDL_EVENT_MOUSE_BUTTON_UP: {
            int bit = (e.button.button == SDL_BUTTON_LEFT) ? 1 : (e.button.button == SDL_BUTTON_RIGHT) ? 2 : 0;
            if (e.type == SDL_EVENT_MOUSE_BUTTON_DOWN) mouse_b |= bit; else mouse_b &= ~bit;
            break;
        }
        default: break;
        }
        if (!audio_kicked && g_astream) { SDL_ResumeAudioStreamDevice(g_astream); audio_kicked = 1; }
    }
    tick_counter = (int)emscripten_get_now();
}
