function void
init_opengl(SDL_Window *window)
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
    gladLoadGL((GLADloadfunc) SDL_GL_GetProcAddress);
    SDL_Log("OpenGL loaded\n");
    SDL_Log("Vendor:   %s", glGetString(GL_VENDOR));
    SDL_Log("Renderer: %s", glGetString(GL_RENDERER));
    SDL_Log("Version:  %s", glGetString(GL_VERSION));
}

function void
get_time(Application *app, u32 *last_frame_run_time_ms)
{
    u32 run_time_ms = SDL_GetTicks();
    app->run_time_s = (f32)run_time_ms / 1000.0f;
    u32 frame_time_ms = run_time_ms - *last_frame_run_time_ms;
    app->frame_time_s = (f32)frame_time_ms / 1000.0f;
    *last_frame_run_time_ms = run_time_ms;
}

function void
main_loop(SDL_Window *window)
{
    Application app = {};
    SDL_GetWindowSize(window, &app.window_dim.Width, &app.window_dim.Height);
    init_app(&app);
    
    Controller *controller = &app.input.controller;
    Input *input = &app.input;
    u32 last_frame_run_time_ms = 0;
    while(1)
    {
        input->mouse_rel_coords = {};
        for (u32 i = 0; i < array_count(controller->buttons); i++)
            controller->buttons[i].previous_state = controller->buttons[i].current_state;
        
        SDL_Event event;
        while(SDL_PollEvent(&event))
        {
            switch(event.type)
            {
                case SDL_QUIT: return;
                
                case SDL_WINDOWEVENT:
                {
                    SDL_WindowEvent *window_event = &event.window;
                    
                    switch(window_event->event)
                    {
                        case SDL_WINDOWEVENT_RESIZED:
                        case SDL_WINDOWEVENT_SIZE_CHANGED:
                        {
                            app.window_dim.Width = window_event->data1;
                            app.window_dim.Height = window_event->data2;
                            glViewport(0, 0, app.window_dim.Width, app.window_dim.Height);
                        } break;
                    }
                } break;
                
                case SDL_MOUSEMOTION:
                {
                    input->mouse_coords.x = event.motion.x;
                    input->mouse_coords.y = event.motion.y;
                    input->mouse_rel_coords.x = event.motion.xrel;
                    input->mouse_rel_coords.y = event.motion.yrel;
                } break;
                
                case SDL_KEYDOWN:
                case SDL_KEYUP:
                {
                    SDL_KeyboardEvent *keyboard_event = &event.key;
                    s32 key_id = keyboard_event->keysym.sym;
                    b32 state = FALSE;
                    if (keyboard_event->state == SDL_PRESSED)
                        state = TRUE;
                    
                    for (u32 i = 0; i < array_count(controller->buttons); i++)
                    {
                        if (key_id == controller->buttons[i].id)
                        {
                            controller->buttons[i].current_state = state;
                            break;
                        }
                    }
                } break;
            }
        }
        
        get_time(&app, &last_frame_run_time_ms);
        
        do_one_frame(&app);
        
        SDL_GL_SwapWindow(window);
    }
}

int main(int argc, char *argv[])
{
    
    
    SDL_Init(SDL_INIT_VIDEO | 
             SDL_INIT_GAMECONTROLLER | 
             SDL_INIT_HAPTIC | 
             SDL_INIT_AUDIO);
    SDL_Window *window = SDL_CreateWindow("Coffee Cow", 
                                          SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 
                                          800, 800, 
                                          SDL_WINDOW_RESIZABLE | SDL_WINDOW_OPENGL);
    init_opengl(window);
    main_loop(window);
}