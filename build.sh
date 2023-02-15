mkdir -p build
g++ -I./glad main.cpp -o build/Boat $(sdl2-config --cflags --libs)