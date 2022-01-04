#!/bin/bash
mkdir -p ../../build
pushd ../../build
CommonFlags="-Wall -Werror -Wno-write-strings -Wno-unused-variable -Wno-unused-function -Wno-sign-compare -std=gnu++11 -fno-rtti -fno-exceptions -DSNAKE_INTERNAL=1 -DSNAKE_SLOW=1 -DSNAKE_SDL=1"

# Build a 64-bit version
c++ $CommonFlags ../game/code/sdl_snake.cpp -o snake.x86_64 -g `../game/code/sdl2-64/bin/sdl2-config --cflags --libs` -Wl,-rpath,'$ORIGIN/x86_64'
# Build a 32-bit version
c++ -m32 $CommonFlags ../game/code/sdl_snake.cpp -o snake.x86 -g `../game/code/sdl2-32/bin/sdl2-config --cflags --libs` -Wl,-rpath,'$ORIGIN/x86'

#Copy SDL into the right directory.
mkdir -p x86_64
cp ../game/code/sdl2-64/lib/libSDL2-2.0.so.0 x86_64/
mkdir -p x86
cp ../game/code/sdl2-32/lib/libSDL2-2.0.so.0 x86/
popd
