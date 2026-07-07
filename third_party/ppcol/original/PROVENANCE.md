# PPCol — original sources

Pixel-perfect collision detection library for Allegro sprites. This folder holds
the **complete, untouched original release**, recovered for reference.

## Version

**PPCol v1.4 "Final version"** — released 11 September 1998, the last release.
(v1.0 was announced 8 April 1998 on the DJGPP newsgroup / mailing list.)

## Where it came from

The original download URLs are all dead:

- `http://members.xoom.com/baldo/ppcol10.zip` (author's homepage — gone)
- `http://ibaldo.codigolibre.net/ppcol14.zip` (gone)
- `ftp://ftp.simtel.net/.../allegro/ppcol14.zip` (404)
- `https://www.delorie.com/pub/simtelnet/.../allegro/ppcol14.zip` (404)

The only surviving live copy was the UK academic mirror of ftp.delorie.com:

    http://www.mirrorservice.org/sites/ftp.delorie.com/pub/djgpp/current/v2tk/allegro/ppcol14.zip

`ppcol14.zip` in this folder is that file, byte-for-byte (37,903 bytes,
mirror-dated 1998-09-12). Everything else here is its extracted contents.

## Contents

- `ppcol14.zip` — the original archive, unmodified
- `SRC/` — `PPMASKC.C` (bitmask algorithm), `PPCOL.C` (pixel-by-pixel),
  `PPAMOUNT.C` / `PPMAMOUN.C` (collision-amount), `PPCOL.H`
- `EXAMPLES/` — EX1/EX2/EX3 demo programs
- `TESTS/BMARK.C` — benchmark
- `DOCS/` — manual (`PPCOL._TX`) and changelog (`CHANGES._TX`), Allegro Makedoc format
- `MAKEFILE`, `SPRITES.DAT`, `SPRITES.H`, `MANIFEST/`, `FILE_ID.DIZ`

## Authors & credit chain

- **Neil Chinnery** — wrote the original 32×32 bitmask collision routine
  (see the `// original from Neil` comment in `SRC/PPMASKC.C`).
- **Michael de la Pena (aka Vox)** — extended it to handle sprites wider than 32px.
- **Ivan Baldo** — made the routines dynamic, added the pixel-by-pixel and
  collision-amount routines, packaged and maintained the library.
- **Peter Wang** — benchmark program, alpha/beta testing, found a bug.
- **Grzegorz Adam Hankiewicz** — EX2 example, documentation reformatting.
- **Eduardo Roldan** — sprites.
- Thanks (per docs) to Salvador Eduardo Tropea, Ove Kaaven, Eli Zaretskii,
  Shawn Hargreaves, DJ Delorie.

## License

Freeware (per `DOCS/PPCOL._TX`):

- Free use in commercial or non-commercial programs; your programs are not
  encumbered by using it.
- Free redistribution without charge (except copying costs), provided you keep
  a link/credit to Ivan Baldo's page.
- You may modify and maintain an unofficial version, but must label it as
  unofficial and point users to the official library.

## Notes for reuse

This is DJGPP/DOS-era code targeting **Allegro 3.x**: it uses `getpixel`,
`bitmap_mask_color`, and 8/15/16/24/32bpp `BITMAP`s. It needs light porting for
modern **Allegro 5**. RLE and COMPILED sprites are not supported.
