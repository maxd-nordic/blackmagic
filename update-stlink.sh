#!/usr/bin/env bash
sudo dfu-util -d 1d50:6018,:6017 -s 0x08002000:leave -D build/blackmagic_stlink_firmware.bin
