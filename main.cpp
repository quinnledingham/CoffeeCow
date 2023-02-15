#include <glad.h>
#include <glad.c>
#include <SDL.h>

#include "types.h"

struct Bitmap
{
    void *memory;
    s32 width;
    s32 height;
    s32 pitch;
    s32 channels;
    
    void *texture_handle;
};

struct Shader
{
    const char *vs_filename; //.vs vertex_shader
    const char *tcs_filename; //.tcs tessellation control shader
    const char *tes_filename; //.tes tessellation evaluation shader
    const char *gs_filename; //.gs geometry shader
    const char *fs_filename; //.fs fragment shader
    
    b32 compiled;
    u32 handle;
};

struct Assets
{
    Bitmap *bitmaps;
    u32 num_of_bitmaps;
    
    Shader *shaders;
    u32 num_of_shaders;
};

struct Button
{
    u32 id;
    b32 current_state;
    b32 previous_state;
};

struct Controller
{
    Button left;
    Button up;
    Button right;
    Button down;
    
    Button select;
    Button pause;
};

struct Dev_Controller
{
    Button toggle_fps;
    Button toggle_wireframe;
};

struct Application
{
    v2s window_dim;
    f32 run_time_s;
    f32 frame_time_s;
    Assets assets;
    Controller controller;
    Dev_Controller dev_controller;
};

function void
load_assets(Assets *assets)
{
    
}

function void
do_one_frame(Application *app)
{
    
}

function void
main_loop(SDL_Window *window)
{
    Application app = {};
    SDL_GetWindowSize(window, &app.window_dim.Width, &app.window_dim.Height);
    SDL_SetRelativeMouseMode(SDL_TRUE);
    
    // setup playing controller
    Controller *controller = &app.controller;
    controller->right.id = SDLK_d;
    controller->up.id = SDLK_w;
    controller->left.id = SDLK_a;
    controller->down.id = SDLK_s;
    
    controller->select.id = SDLK_RETURN;
    controller->pause.id = SDLK_ESCAPE;
    
    //
    // DEV
    //
    
    Dev_Controller *dev_controller = &app.dev_controller;
    dev_controller->toggle_fps.id = SDLK_F5;
    dev_controller->toggle_wireframe.id = SDLK_F6;
    
    //
    // End of DEV
    //
    
    load_assets(&app.assets);
    
    u32 last_frame_run_time_ms = 0;
    while(1)
    {
        SDL_Event event;
        while(SDL_PollEvent(&event))
        {
            switch(event.type)
            {
                case SDL_QUIT: return;
            }
        }
        
        // Getting time since program started and time of frame
        u32 run_time_ms = SDL_GetTicks();
        app.run_time_s = (f32)run_time_ms / 1000.0f;
        u32 frame_time_ms = run_time_ms - last_frame_run_time_ms; 
        app.frame_time_s = (f32)frame_time_ms / 1000.0f;
        last_frame_run_time_ms = run_time_ms;
        
        do_one_frame(&app);
        
        SDL_GL_SwapWindow(window);
    }
}

function void
sdl_init_opengl(SDL_Window *window)
{
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
    gladLoadGLLoader((GLADloadproc) SDL_GL_GetProcAddress);
    SDL_Log("OpenGL loaded\n");
    SDL_Log("Vendor:   %s", glGetString(GL_VENDOR));
    SDL_Log("Renderer: %s", glGetString(GL_RENDERER));
    SDL_Log("Version:  %s", glGetString(GL_VERSION));
    
    // Default settings
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);
    
    glEnable(GL_DEBUG_OUTPUT);
    //glDebugMessageCallback(opengl_debug_message_callback, 0);
    
    v2s window_dim = {};
    SDL_GetWindowSize(window, &window_dim.Width, &window_dim.Height);
    glViewport(0, 0, window_dim.Width, window_dim.Height);
}

int main(int argc, char *argv[])
{
    SDL_Init(SDL_INIT_VIDEO |
             SDL_INIT_GAMECONTROLLER |
             SDL_INIT_AUDIO);
    SDL_Window *window = SDL_CreateWindow("Coffee Cow", 
                                          SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 
                                          800, 800, 
                                          SDL_WINDOW_RESIZABLE | SDL_WINDOW_OPENGL);
}

