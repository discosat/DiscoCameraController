#!/bin/sh
rm -rf build
meson setup . build
ninja -C build