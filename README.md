# Zombie Slaughter — Linux resurrection

Moritz Laass' 2004 Ludum Dare 48h game, rebuilt for modern Linux with CMake.
The original Windows/Allegro-4.1 source is preserved untouched under `original/`.

## Layout

```
original/       untouched 2004 release (exe, dlls, data.dat, Src/)
src/            ported game source (Linux/g++/Allegro-4.4 fixes)
third_party/
  ppcol/        pixel-perfect collision (reimplemented, see below)
  fblend/       fblend_add shim over Allegro's built-in add blender
tools/
  dat_extract.c extracts data.dat -> BMP/WAV and regenerates data.h
assets/         extracted assets (bitmaps, sounds) + generated data.h
CMakeLists.txt
```

## Build

Needs Allegro 4:

```sh
sudo apt install liballegro4-dev      # Ubuntu 24.04: gives -lalleg, headers in /usr/include/allegro
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
```

Produces `build/zombieslaughter` (with `data.dat` copied beside it) and `build/dat_extract`.

## Run

```sh
cd build && ./zombieslaughter            # fullscreen
./zombieslaughter -windowed              # windowed
./zombieslaughter -windowed -debug       # + fps/coords overlay
```

Controls: Left/Right or A/D steer, Up/W/Space jump (double-jump!), Mouse/LCtrl
shoot, 1/2 switch weapon, M/P mute/play music, Esc back to menu.

## Extract original assets

```sh
./build/dat_extract original/ZombieSlaughter/data.dat assets
```

Dumps 23 bitmaps (24-bit `.bmp`, datafile palette baked in) and 9 samples
(`.wav`). Allegro fonts are skipped (no sane export target). It also regenerates
`data.h`.

> Allegro ships its own `dat` CLI (installed at `/usr/bin/dat` by
> `liballegro4-dev`) which is the canonical extractor: `dat -x -o assets -pal
> AAAPAL data.dat '*'`. On a normal desktop that's the tool to use. It hung when
> reading this datafile in the headless build session here, so `dat_extract`
> does the same job via the same Allegro datafile API. Colours must be baked to
> 24-bit: `save_bitmap` on an 8-bit bitmap writes Allegro's raw 6-bit palette
> values and comes out ~4x too dark, so we blit through the palette to a 24-bit
> bitmap first.

## Porting notes

- **Allegro 4, not 5.** The code is Allegro 4.1; Ubuntu ships Allegro 4.4
  (`liballegro4-dev`), API-compatible, so it builds almost as-is. A raylib/Allegro-5
  port would be a full rewrite (datafiles, fonts, samples, blitting, input) and
  isn't needed to run the game.
- **PPCol** (Ivan Baldo, 1998): only `check_pp_collision` and
  `check_pp_collision_amount` (bitmap forms) were ever used. Rather than hunt the
  27-year-old library, both are reimplemented in `third_party/ppcol/ppcol.c` as a
  naive per-pixel overlap scan — fine for this game's scale.
- **FBlend**: replaced its single `fblend_add` call with Allegro's `set_add_blender`.
- **data.h was stale.** The shipped `data.dat` has 4 bitmaps (`BMP_DYNAMITE`,
  `BMP_ITEM_D/F/H`) added after the checked-in `data.h` was last exported, shifting
  every later index — so `FNT_BIG` pointed at a BITMAP and the game crashed casting
  it to a FONT. Fixed by regenerating `data.h` from the actual datafile.
- Minor g++ fixes: Windows `.\` include paths, lowercase filenames (case-sensitive
  FS), C++-scoped `for` loop variables, missing `<string>`/`<cstring>`/`<ctime>`.
```
