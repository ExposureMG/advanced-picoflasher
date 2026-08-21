#!/bin/bash
set -e

echo "Building PicoFlasher for standard Pico..."
cmake -B build_pico -DFLASHER_BOARD="pico"
cmake --build build_pico -j$(nproc)
cp build_pico/PicoFlasher.uf2 PicoFlasher_Standard.uf2

echo "Building PicoFlasher for RP2040 Zero..."
cmake -B build_zero -DFLASHER_BOARD="waveshare_rp2040_zero"
cmake --build build_zero -j$(nproc)
cp build_zero/PicoFlasher.uf2 PicoFlasher_RP2040_Zero.uf2

echo "Builds complete! Outputs:"
echo "- PicoFlasher_Standard.uf2"
echo "- PicoFlasher_RP2040_Zero.uf2"
