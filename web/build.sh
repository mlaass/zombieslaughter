#!/usr/bin/env bash
# Build the WASM version. Run from repo root: ./web/build.sh
# Needs emsdk on PATH (source emsdk_env.sh) and web/assets/data.pak baked by pak_build.
set -euo pipefail
cd "$(dirname "$0")/.."

[ -f web/assets/data.pak ] || { echo "web/assets/data.pak missing — run: build/pak_build original/ZombieSlaughter/data.dat web/assets/data.pak"; exit 1; }

mkdir -p web/dist
emcc \
  src/entity.cpp src/game.cpp src/grafics.cpp src/hiscore.cpp src/land.cpp src/main.cpp src/vec2.cpp \
  web/alleg_compat.cpp \
  third_party/ppcol/ppcol.c third_party/ppcol/ppamount.c \
  -Iweb -Isrc -Ithird_party/ppcol -Ithird_party/fblend \
  --use-port=sdl3 \
  -O2 -sALLOW_MEMORY_GROWTH=1 -sINITIAL_MEMORY=64MB \
  --preload-file web/assets/data.pak@/data.pak \
  --shell-file web/shell.html \
  -o web/dist/index.html

echo "built web/dist/  ->  zip its contents for itch.io"
