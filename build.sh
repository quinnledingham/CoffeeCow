mkdir -p build
g++ -I./glad -I./stb  coffeecow.cpp -o build/coffeecow $(sdl2-config --cflags --libs)