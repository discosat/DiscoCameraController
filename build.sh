#!/bin/sh

# Default to native build if no arguments provided
CROSS_FILE=""
BUILD_DIR="build"

# Check for cross-compilation argument
if [ "$1" = "cross" ] || [ "$1" = "--cross" ]; then
    if [ -f "yoco_cross.ini" ]; then
        CROSS_FILE="--cross-file yoco_cross.ini"
        BUILD_DIR="build_cross"
        echo "Using cross-compilation with yoco_cross.ini"
    else
        echo "Error: yoco_cross.ini not found for cross-compilation"
        exit 1
    fi
elif [ -n "$1" ]; then
    # Allow specifying custom cross file
    if [ -f "$1" ]; then
        CROSS_FILE="--cross-file $1"
        BUILD_DIR="build_cross"
        echo "Using cross-compilation with $1"
    else
        echo "Error: Cross file $1 not found"
        exit 1
    fi
fi

echo "Building in directory: $BUILD_DIR"
rm -rf $BUILD_DIR
meson setup . $BUILD_DIR $CROSS_FILE
ninja -C $BUILD_DIR