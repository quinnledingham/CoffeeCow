#ifndef APPLICATION_H
#define APPLICATION_H

struct Input
{
    Controller controller;
    v2 mouse_coords;
    v2 mouse_rel_coords;
};

struct Application
{
    Controller controller; // struct must be defined before this header
    Input input;
    
    v2s window_dim;
    f32 run_time_s;
    f32 frame_time_s;
};

function void do_one_frame(Application *app);
function void init_app(Application *app);

#endif //APPLICATION_H
