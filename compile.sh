#!/bin/bash
set -euo pipefail

COMPILER="gcc"

HEADER_DIR="headers"
SOURCE_DIR="src"
OBJ_DIR=".obj"
OUT="main"

LINKS="-lm"
CARGS="-std=c2x -O0 -g3 -fno-omit-frame-pointer -Wall -Wextra -Wpedantic -Wshadow -Wconversion -Wsign-conversion -Wcast-qual -Wcast-align -Wstrict-prototypes -Wundef -Wmissing-prototypes -Wold-style-definition -Wwrite-strings -Wformat=2 -fsanitize=address,undefined -Wdouble-promotion -Wvla -Wpointer-arith -Wnull-dereference -fno-sanitize-recover=all -fno-common"

mkdir -p "$OBJ_DIR"

OBJS=()
for src in "$SOURCE_DIR"/*.c; do
        obj="$OBJ_DIR/$(basename "${src%.c}").o"
        echo "CC  $src -> $obj"
        $COMPILER $CARGS -I"$HEADER_DIR" -I"$SOURCE_DIR" -c "$src" -o "$obj"
        OBJS+=("$obj")
done

echo "LD  -> $OUT"
$COMPILER $CARGS "${OBJS[@]}" $LINKS -o "$OUT"
echo "Built: $OUT"
