#ifndef APPLICATION_H
#define APPLICATION_H

struct Window
{
    SDL_Window *sdl;
    v2s dim;
};

function void swap_window(Window *window) { SDL_GL_SwapWindow(window->sdl); }

struct Time
{
    u32 run_time_ms;
    r32 run_time_s;
    u32 frame_time_ms;
    r32 frame_time_s;
    r32 frames_per_s;
};

struct Controller
{
    union
    {
        struct
        {
            Button right;
            Button up;
            Button left;
            Button down;
            Button select;
            Button pause;
        };
        Button buttons[6];
    };
};

struct Input
{
    Controller controllers[5];
    u32 num_of_controllers;
    Controller *active_controller;
    
    SDL_Joystick *joysticks[4];
    SDL_GameController *game_controllers[4];
    u32 num_of_joysticks;
};

struct Application
{
    Window window;
    Time time;
    Input input;
    Assets assets;
    
    void *data;
};

#endif //APPLICATION_H
