#!/bin/sh -x

SRCDIR=$(realpath "$(dirname "$0")/../")

python3 "$SRCDIR/tools/gen_texture_code.py" \
   --header "$SRCDIR/data/textures/blocks" \
   > "$SRCDIR/mapcraftercore/renderer/blocktextures.h"

python3 "$SRCDIR/tools/gen_texture_code.py" \
   --source "$SRCDIR/data/textures/blocks" \
   > "$SRCDIR/mapcraftercore/renderer/blocktextures.cpp"
