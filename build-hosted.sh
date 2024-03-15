#!/usr/bin/env bash
source envsetup.sh

rm -rf set build
meson setup build
meson compile -C build
