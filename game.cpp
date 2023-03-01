#include <gl.h>
#include <gl.c>
#include <SDL.h>

#include "log.h"
#include "types.h"

struct Button
{
    s32 id;
    b32 current_state; 
    b32 previous_state;
};
inline void set(Button *button, s32 id) { button->id = id; }
inline b32 is_down(Button button) { if (button.current_state) return true; return false; }
inline b32 on_down(Button button)
{
    if (button.current_state && button.current_state != button.previous_state) return true;
    return false;
}

function u32
game()
{
    // window()
    u32 sdl_init_flags = SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER | SDL_INIT_HAPTIC | SDL_INIT_AUDIO;
    SDL_Init(sdl_init_flags);
    u32 sdl_window_flags = SDL_WINDOW_RESIZABLE | SDL_WINDOW_OPENGL;
    SDL_Window *window = SDL_CreateWindow("Coffee Cow", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 800, 800, sdl_window_flags);
    
    // init_opengl()
    SDL_GL_LoadLibrary(NULL);
    
    // Request an OpenGL 4.6 context (should be core)
    SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 6);
    
    // Also request a depth buffer
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    
    SDL_GLContext Context = SDL_GL_CreateContext(window);
    SDL_GL_SetSwapInterval(0);
    
    // Check OpenGL properties
    gladLoadGL((GLADloadfunc) SDL_GL_GetProcAddress);
    log("OpenGL loaded\n");
    log("Vendor:   %s", glGetString(GL_VENDOR));
    log("Renderer: %s", glGetString(GL_RENDERER));
    log("Version:  %s", glGetString(GL_VERSION));
    // load_assets()
    
    
    // init_vars()
    u32 last_run_time_ms = 0;
    v2s window_dim = {};
    SDL_GetWindowSize(window, &window_dim.width, &window_dim.height);
    
    // init opengl vars
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glViewport(0, 0, window_dim.width, window_dim.height);
    
    Str test = "yoooo";
    SDL_Log("%d\n", get_length(test));
    
    while(1)
    {
        // input()
        SDL_Event event;
        while(SDL_PollEvent(&event))
        {
            switch(event.type)
            {
                case SDL_QUIT: return 0;
                
                case SDL_WINDOWEVENT:
                {
                    SDL_WindowEvent *window_event = &event.window;
                    
                    switch(window_event->event)
                    {
                        case SDL_WINDOWEVENT_RESIZED:
                        case SDL_WINDOWEVENT_SIZE_CHANGED:
                        {
                            window_dim.width = window_event->data1;
                            window_dim.height = window_event->data2;
                            glViewport(0, 0, window_dim.width, window_dim.height);
                        } break;
                    }
                } break;
            }
        }
        
        // update()
        
        // get_run_time()
        u32 run_time_ms = SDL_GetTicks();
        r32 run_time_s = (f32)run_time_ms / 1000.0f;
        u32 frame_time_ms = run_time_ms - last_run_time_ms;
        r32 frame_time_s = (f32)frame_time_ms / 1000.0f;
        last_run_time_ms = run_time_ms;
        
        // draw()
        u32 gl_clear_flags = GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT;
        glClear(gl_clear_flags);
        
        SDL_GL_SwapWindow(window);
    }
    
    return 0;
}

int main(int argc, char *argv[]) { return game(); }