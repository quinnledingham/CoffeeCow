#ifndef APPLICATION_H
#define APPLICATION_H

struct Window
{
    SDL_Window *sdl;
    v2s dim;
    r32 aspect_ratio;

    b32 *update_matrices;
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

struct Button
{
    s32 id;
    
    s32 ids[3];
    u32 num_of_ids;
    
    b32 current_state; 
    b32 previous_state;
};

inline void set(Button *button, s32 id) 
{
    if (button->num_of_ids > 2)
        error("set() too many ids trying to be assigned to button");
    
    button->ids[button->num_of_ids++] = id;
    button->id = id; 
}

inline b32 is_down(Button button) 
{ 
    if (button.current_state) return true; return false; 
}

inline b32 on_down(Button button)
{
    if (button.current_state && button.current_state != button.previous_state) return true;
    return false;
}

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

    union
    {
        struct
        {
            Button show_fps;
        };
        Button debug_buttons[1];
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

struct Matrices // for rendering
{
    b32 update;

    m4x4 perspective_matrix;
    m4x4 orthographic_matrix;
    m4x4 view_matrix;
};

struct Application
{
    Window window;
    Matrices matrices;
    Time time;
    Input input;
    Assets assets;
    Audio_Player player;
    
    void *data;
};

void *platform_malloc(u32 size)
{
    return SDL_malloc(size);
}

void platform_free(void *ptr)
{
    SDL_free(ptr);
}

// block index is from glUniformBlockBinding or binding == #
u32 init_uniform_buffer_object(u32 block_size, u32 block_index)
{
    u32 uniform_buffer_object;
    glGenBuffers(1, &uniform_buffer_object);
    
    glBindBuffer(GL_UNIFORM_BUFFER, uniform_buffer_object);
    glBufferData(GL_UNIFORM_BUFFER, block_size, NULL, GL_STATIC_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
    
    glBindBufferBase(GL_UNIFORM_BUFFER, block_index, uniform_buffer_object);
    
    return uniform_buffer_object;
}

void platform_set_uniform_block_binding(u32 shader_handle, const char *tag, u32 index)
{
    u32 tag_uniform_block_index = glGetUniformBlockIndex(shader_handle, tag);
    glUniformBlockBinding(shader_handle, tag_uniform_block_index, index);
}

// functions to set matrices in uniform buffer
void orthographic(u32 ubo, Matrices *matrices)
{
    glBindBuffer(GL_UNIFORM_BUFFER, ubo);
    glBufferSubData(GL_UNIFORM_BUFFER, 0,            sizeof(m4x4), (void*)&matrices->orthographic_matrix);
    glBufferSubData(GL_UNIFORM_BUFFER, sizeof(m4x4), sizeof(m4x4), (void*)&identity_m4x4());
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}


#endif //APPLICATION_H
