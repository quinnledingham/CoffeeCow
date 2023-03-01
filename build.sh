mkdir -p build
g++ -I./glad -I./stb -I./qlib game.cpp -o build/coffeecow $(sdl2-config --cflags --libs)
