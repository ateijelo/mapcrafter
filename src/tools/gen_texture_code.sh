#!/bin/sh -x

SRCDIR=$(realpath "$(dirname "$0")/../")

python "$SRCDIR/tools/gen_texture_code.py" \
   --header "$SRCDIR/data/textures/blocks" \
   > "$SRCDIR/mapcraftercore/renderer/blocktextures.h"

python "$SRCDIR/tools/gen_texture_code.py" \
   --source "$SRCDIR/data/textures/blocks" \
   > "$SRCDIR/mapcraftercore/renderer/blocktextures.cpp"
