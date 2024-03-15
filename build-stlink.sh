#!/usr/bin/env bash
source envsetup.sh

rm -rf set build
meson setup build --cross-file cross-file/stlink.ini
meson compile -C build
