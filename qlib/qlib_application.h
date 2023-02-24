#ifndef QLIB_APPLICATION_H
#define QLIB_APPLICATION_H

// What structs must be defined before including this file:
// Controller
// Storage

struct Input
{
#ifdef QLIB_CONTROLLER
    Controller controller;
#endif
    
    v2 mouse_coords;
    v2 mouse_rel_coords;
};

struct Application
{
    Input input;
    Assets assets;
    
#ifdef QLIB_STORAGE
    Storage storage;
#endif
    
    v2s window_dim;
    
    f32 run_time_s;
    f32 frame_time_s;
    
    b32 quit;
};

function void do_one_frame(Application *app);
function void init_app(Application *app);

#include "qlib_sdl_application.cpp"

#endif //QLIB_APPLICATION_H
