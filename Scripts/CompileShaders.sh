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

    # Compile vertex shader
    if [ -f "$dir/shader.vert" ]; then
        if ! glslc "$dir/shader.vert" -o "$out_dir/vert.spv"; then
            echo "Failed to compile vertex shader for $shader_name"
            exit 1
        fi
    fi

    # Compile fragment shader
    if [ -f "$dir/shader.frag" ]; then
        if ! glslc "$dir/shader.frag" -o "$out_dir/frag.spv"; then
            echo "Failed to compile fragment shader for $shader_name"
            exit 1
        fi
    fi
done

DEBUG_BUILD_DIR="$PARENT_DIR/Build/Linux/Debug/Data/Engine"

cp -rf "$DEST_DIR/" "$DEBUG_BUILD_DIR/"