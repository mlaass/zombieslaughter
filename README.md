# Zombie Slaughter — Linux resurrection

Moritz Laass' 2004 Ludum Dare 48h game, rebuilt for modern Linux with CMake.
The original Windows/Allegro-4.1 source is preserved untouched under `original/`.

## Layout

```
original/       untouched 2004 release (exe, dlls, data.dat, Src/)
src/            ported game source (Linux/g++/Allegro-4.4 fixes)
third_party/
  ppcol/        pixel-perfect collision (original 1998 sources, see below)
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
./zombieslaughter -windowed -fps 30      # slower sim (default 60)
```

Controls: Left/Right or A/D steer, Up/W/Space jump (double-jump!), Mouse/LCtrl
shoot, 1/2 switch weapon, M/P mute/play music, Esc back to menu.

`-fps N` sets the fixed simulation rate — movement speed scales with it (see
"Game speed" below). Sound plays through PulseAudio/PipeWire automatically.

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
- **PPCol** (Ivan Baldo, 1998): the game only calls the two bitmap-form functions,
  `check_pp_collision` and `check_pp_collision_amount`. These use the **original
  1998 sources** — `third_party/ppcol/ppcol.c` (`PPCOL.C`) and `ppamount.c`
  (`PPAMOUNT.C`), copied byte-for-byte from `third_party/ppcol/original/SRC/`. They
  are plain ANSI C against the Allegro `BITMAP` API and compile unchanged on Linux.
  The bitmask API (`PPMASKC.C`/`PPMAMOUN.C`) is unused and not compiled.
- **Sound**: Allegro 4's ALSA driver opens the raw `default` PCM (dmix), which
  fails on a PipeWire desktop ("unable to create IPC semaphore" — the display
  manager already holds the card). `CGame::Init` injects `[sound] alsa_device=pulse`
  via `override_config_data` before `install_sound`, routing audio through
  PulseAudio/PipeWire.
- **Game speed / fixed timestep**: the loop was uncapped and the Verlet integrator
  carries velocity as `(pos-lpos)`, unscaled by dt, so it's only correct at a
  constant step. Uncapped on modern hardware it ran thousands of fps → movement far
  too fast. `CGame::Loop` now advances the sim in fixed `step`-ms slices (default
  16ms / 60Hz, `-fps N` to retune) using `tick_counter` as a free-running 1ms clock,
  and `rest(1)` to yield the CPU.
- **FBlend**: replaced its single `fblend_add` call with Allegro's `set_add_blender`.
- **data.h was stale.** The shipped `data.dat` has 4 bitmaps (`BMP_DYNAMITE`,
  `BMP_ITEM_D/F/H`) added after the checked-in `data.h` was last exported, shifting
  every later index — so `FNT_BIG` pointed at a BITMAP and the game crashed casting
  it to a FONT. Fixed by regenerating `data.h` from the actual datafile.
- Minor g++ fixes: Windows `.\` include paths, lowercase filenames (case-sensitive
  FS), C++-scoped `for` loop variables, missing `<string>`/`<cstring>`/`<ctime>`.
