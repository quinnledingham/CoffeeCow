mkdir -p build
g++ -Wall -Wno-unused-function -Wno-unused-variable -I./glad -I./stb -I./qlib game.cpp -o build/coffeecow $(sdl2-config --cflags --libs)
