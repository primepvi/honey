#!/usr/bin/env bash
set -euo pipefail

BUILD_DIR="build"
mkdir -p "$BUILD_DIR"

# GCC FLAGS
CFLAGS="-std=c11 -Wextra"

echo "[1/2] compiling HASM..."
gcc $CFLAGS \
    hasm/main.c hasm/lexer.c hasm/parser.c \
    -o "$BUILD_DIR/hasm"

echo "[2/2] compiling HVM..."
gcc $CFLAGS \
    hvm/main.c hvm/honey.c \
    -o "$BUILD_DIR/hvm"

echo "log -> build completed"
