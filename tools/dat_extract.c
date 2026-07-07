/* Extract the assets from Zombie Slaughter's Allegro datafile.
 *
 *   dat_extract <data.dat> <output_dir>
 *
 * Bitmaps  -> <name>.bmp   (using the datafile's own palette)
 * Samples  -> <name>.wav   (PCM, written by hand — Allegro has no WAV writer)
 * Fonts    -> skipped (Allegro fonts don't round-trip to a normal file format)
 * Palettes -> skipped (implicitly used for the bitmap exports)
 */
#include <allegro.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

static void sanitize(char *s)
{
    for (; *s; s++)
        if (*s == '/' || *s == '\\' || *s == ' ') *s = '_';
}

/* Minimal PCM WAV writer. Allegro keeps sample data unsigned regardless of
 * depth (16-bit is centred on 0x8000); WAV wants unsigned 8-bit but signed
 * 16-bit, so we convert the 16-bit case. */
static void save_wav(const char *path, SAMPLE *s)
{
    int channels = s->stereo ? 2 : 1;
    int bytes_ps = s->bits / 8;
    long frames  = s->len;
    long data_bytes = frames * channels * bytes_ps;

    FILE *f = fopen(path, "wb");
    if (!f) { printf("  ! cannot write %s\n", path); return; }

    int byte_rate   = s->freq * channels * bytes_ps;
    int block_align = channels * bytes_ps;

#define W32(v) do{ unsigned int _v=(v); fputc(_v&0xff,f);fputc((_v>>8)&0xff,f);fputc((_v>>16)&0xff,f);fputc((_v>>24)&0xff,f);}while(0)
#define W16(v) do{ unsigned int _v=(v); fputc(_v&0xff,f);fputc((_v>>8)&0xff,f);}while(0)
    fwrite("RIFF", 1, 4, f); W32(36 + data_bytes); fwrite("WAVE", 1, 4, f);
    fwrite("fmt ", 1, 4, f); W32(16); W16(1); W16(channels);
    W32(s->freq); W32(byte_rate); W16(block_align); W16(s->bits);
    fwrite("data", 1, 4, f); W32(data_bytes);
#undef W32
#undef W16

    if (s->bits == 8) {
        fwrite(s->data, 1, data_bytes, f);        /* already unsigned */
    } else {
        unsigned short *src = (unsigned short *)s->data;
        long n = frames * channels;
        for (long i = 0; i < n; i++) {
            short v = (short)(src[i] - 0x8000); /* unsigned -> signed */
            fputc(v & 0xff, f); fputc((v >> 8) & 0xff, f);
        }
    }
    fclose(f);
}

int main(int argc, char *argv[])
{
    if (argc < 3) {
        printf("usage: %s <data.dat> <output_dir>\n", argv[0]);
        return 1;
    }
    if (allegro_init() != 0) { printf("allegro_init failed\n"); return 1; }
    set_color_depth(24);

    DATAFILE *df = load_datafile(argv[1]);
    if (!df) { printf("cannot load datafile %s\n", argv[1]); return 1; }

    /* the palette the paletted bitmaps were grabbed with */
    PALETTE pal;
    get_palette(pal);
    for (int i = 0; df[i].type != DAT_END; i++)
        if (df[i].type == DAT_PALETTE) { memcpy(pal, df[i].dat, sizeof(PALETTE)); break; }
    /* select it so 8-bit -> 24-bit blits map indices to real RGB, with the
     * correct 6-bit -> 8-bit scaling (save_bitmap on a paletted bitmap writes
     * the raw 6-bit values and comes out ~4x too dark). */
    select_palette(pal);

    /* Regenerate data.h — the checked-in one is out of sync with this datafile
     * (indices shifted by objects added after it was last exported). */
    char hpath[512];
    snprintf(hpath, sizeof hpath, "%s/data.h", argv[2]);
    FILE *hf = fopen(hpath, "w");
    if (hf) fprintf(hf, "/* Regenerated from %s by dat_extract. Do not hand edit. */\n\n", argv[1]);

    int n_bmp = 0, n_wav = 0, n_skip = 0;
    for (int i = 0; df[i].type != DAT_END; i++) {
        char name[64];
        char *prop = get_datafile_property(&df[i], DAT_NAME);
        snprintf(name, sizeof name, "%s", (prop && *prop) ? prop : "unnamed");
        sanitize(name);

        if (hf)
            fprintf(hf, "#define %-28s %-4d /* %c%c%c%c */\n", name, i,
                    (df[i].type>>24)&0xff,(df[i].type>>16)&0xff,(df[i].type>>8)&0xff,df[i].type&0xff);

        char path[512];
        if (df[i].type == DAT_BITMAP) {
            BITMAP *src = (BITMAP *)df[i].dat;
            snprintf(path, sizeof path, "%s/%s.bmp", argv[2], name);
            /* bake the palette into 24-bit so colours are correct in any viewer */
            BITMAP *out = create_bitmap_ex(24, src->w, src->h);
            blit(src, out, 0, 0, 0, 0, src->w, src->h);
            if (save_bitmap(path, out, NULL) == 0) { printf("  bmp  %s\n", path); n_bmp++; }
            else printf("  ! failed %s\n", path);
            destroy_bitmap(out);
        } else if (df[i].type == DAT_SAMPLE) {
            snprintf(path, sizeof path, "%s/%s.wav", argv[2], name);
            save_wav(path, (SAMPLE *)df[i].dat); printf("  wav  %s\n", path); n_wav++;
        } else {
            printf("  skip %s (type '%c%c%c%c')\n", name,
                   (df[i].type>>24)&0xff,(df[i].type>>16)&0xff,(df[i].type>>8)&0xff,df[i].type&0xff);
            n_skip++;
        }
    }

    if (hf) { fclose(hf); printf("  gen  %s\n", hpath); }
    unload_datafile(df);
    printf("\ndone: %d bitmaps, %d samples, %d skipped\n", n_bmp, n_wav, n_skip);
    return 0;
}
END_OF_MAIN()
