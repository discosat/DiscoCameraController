#!/bin/sh
rm -rf build
ninja -t clean
meson setup build
ninja -C build
