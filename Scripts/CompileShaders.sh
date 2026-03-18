#!/bin/bash
set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PARENT_DIR="$(dirname "$SCRIPT_DIR")"

SOURCE_DIR="$PARENT_DIR/ShadersGLSL"
DEST_DIR="$PARENT_DIR/Data/Engine/Shaders"

mkdir -p "$DEST_DIR"

for dir in "$SOURCE_DIR"/*/; do
    shader_name="$(basename "$dir")"
    out_dir="$DEST_DIR/$shader_name"

    mkdir -p "$out_dir"

    echo "Compiling $shader_name"

    glslc "$dir/shader.vert" -o "$out_dir/vert.spv"
    glslc "$dir/shader.frag" -o "$out_dir/frag.spv"
done
