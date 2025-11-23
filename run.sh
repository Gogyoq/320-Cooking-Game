#!/usr/bin/env bash
set -e
cxx=${CXX:-clang++}
$cxx -std=c++17 -Iframeworks/include \
  src/*.cpp src/minigames/*.cpp \
  -L/usr/local/lib -L/opt/homebrew/lib \
  -lSDL3 -lSDL3_ttf -lSDL3_image \
  -o cooking_mama_clone
./cooking_mama_clone
