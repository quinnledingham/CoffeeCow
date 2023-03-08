#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_RESIZE_IMPLEMENTATION
#define STB_TRUETYPE_IMPLEMENTATION
#include <stb_image.h>
#include <stb_image_resize.h>
#include <stb_truetype.h>
#include <gl.h>
#include <gl.c>
#include <SDL.h>

#include "log.h"
#include "types.h"

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

enum Game_Modes
{
    MAIN_MENU,
    IN_GAME,
    PAUSED,
    GAME_OVER
};

enum Asset_Tags
{
    ASSET_COW_HEAD,
    ASSET_COW_HEAD_OUTLINE,
    ASSET_COW_CIRCLE,
    ASSET_COW_CIRCLE_OUTLINE,
};

global_variable m4x4 orthographic_matrix;
global_variable v2s global_window_dim;

#include "assets.h"
#include "assets.cpp"
#include "rect.cpp"
#include "menu.cpp"
#include "coffee_cow.h"
#include "coffee_cow.cpp"

/*
 TODO:
- add coffee spots to cow
- add random cow placement
- add coffee
- add game controller support
- add multiplayer
- create 3 more cow designs
*/

function void
controller_process_input(Controller *controller, s32 id, b32 state)
{
    for (u32 i = 0; i < ARRAY_COUNT(controller->buttons); i++)
    {
        // loop through all ids associated with button
        for (u32 j = 0; j < controller->buttons[i].num_of_ids; j++)
        {
            if (id == controller->buttons[i].ids[j])
                controller->buttons[i].current_state = state;
        }
    }
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
    log("OpenGL loaded:");
    log("Vendor:   %s", glGetString(GL_VENDOR));
    log("Renderer: %s", glGetString(GL_RENDERER));
    log("Version:  %s", glGetString(GL_VERSION));
    
    // load_assets()
    Bitmap logo = load_and_init_bitmap("../assets/bitmaps/logo.png");
    Bitmap main_menu_back =  load_and_init_bitmap("../assets/bitmaps/mainmenuback.png");
    
    Bitmap game_back = load_and_init_bitmap("../assets/bitmaps/sand.png");
    Bitmap grass = load_and_init_bitmap("../assets/bitmaps/grass.png");
    Bitmap rocks = load_and_init_bitmap("../assets/bitmaps/rocks.png");
    Bitmap grid = load_and_init_bitmap("../assets/bitmaps/grid.png");
    //Bitmap test = load_and_init_bitmap("../assets/bitmaps/testimg.jpg");
    
    Font rubik = load_font("../assets/fonts/Rubik-Medium.ttf");
    
    // init_vars()
    Controller controllers[4] = {};
    u32 num_of_controllers = 0;
    Controller *menu_controller = &controllers[0];
    
    Controller *keyboard = &controllers[0];
    set(&keyboard->right, SDLK_d);
    set(&keyboard->right, SDLK_RIGHT);
    set(&keyboard->up, SDLK_w);
    set(&keyboard->up, SDLK_UP);
    set(&keyboard->left, SDLK_a);
    set(&keyboard->left, SDLK_LEFT);
    set(&keyboard->down, SDLK_s);
    set(&keyboard->down, SDLK_DOWN);
    set(&keyboard->select, SDLK_RETURN);
    set(&keyboard->pause, SDLK_ESCAPE);
    num_of_controllers++;
    
    log("Game Controllers:");
    SDL_Joystick *joysticks[4] = {};
    SDL_GameController *game_controllers[4] = {};
    u32 num_of_joysticks = SDL_NumJoysticks();
    for (u32 i = 0; i < num_of_joysticks; i++)
    {
        joysticks[i] = SDL_JoystickOpen(i);
        if (SDL_IsGameController(i))
        {
            log("%s", SDL_JoystickName(joysticks[i]));
            
            game_controllers[i] = SDL_GameControllerOpen(i);
            
            Controller *c = &controllers[i + 1];
            set(&c->right, SDL_CONTROLLER_BUTTON_DPAD_RIGHT);
            set(&c->up, SDL_CONTROLLER_BUTTON_DPAD_UP);
            set(&c->left, SDL_CONTROLLER_BUTTON_DPAD_LEFT);
            set(&c->down, SDL_CONTROLLER_BUTTON_DPAD_DOWN);
            set(&c->select, SDL_CONTROLLER_BUTTON_A);
            set(&c->pause, SDL_CONTROLLER_BUTTON_START);
            
            num_of_controllers++;
        }
    }
    
    u32 last_run_time_ms = 0;
    v2s window_dim = {};
    SDL_GetWindowSize(window, &window_dim.width, &window_dim.height);
    
    u32 game_mode = MAIN_MENU;
    s32 active = 0;
    v2s grid_dim = { 10, 10 };
    
    Coffee_Cow_Design designs[4];
    designs[0].bitmaps[ASSET_COW_HEAD] = load_and_init_bitmap("../assets/bitmaps/cow1/cowhead.png");
    designs[0].bitmaps[ASSET_COW_HEAD_OUTLINE] = load_and_init_bitmap("../assets/bitmaps/cow1/cowheadoutline.png");
    designs[0].bitmaps[ASSET_COW_CIRCLE] = load_and_init_bitmap("../assets/bitmaps/cow1/circle.png");
    designs[0].bitmaps[ASSET_COW_CIRCLE_OUTLINE] = load_and_init_bitmap("../assets/bitmaps/cow1/circleoutline.png");
    designs[0].color = { 255, 255, 255, 1 };
    designs[0].outline_color = { 0, 0, 0, 1 };
    
    Coffee_Cow players[4]; // stores the players in the game
    u32 num_of_players = 0;
    players[0].first_input_of_transition = false;
    
    Coffee_Cow cow = {};
    cow.design = designs[0];
    cow.first_input_of_transition = true;
    cow.direction = { 0, 1 };
    add_node(&cow, { 0, 4 });
    add_node(&cow, { 0, 3 });
    add_node(&cow, { 0, 2 });
    add_node(&cow, { 0, 1 });
    
    // default menu
    Menu default_menu = {};
    default_menu.font = &rubik;
    
    default_menu.button.back_color = {0, 0, 0, 1};
    default_menu.button.active_back_color = {222, 201, 179, 1};
    default_menu.button.text_color = {255, 255, 255, 1};
    default_menu.button.active_text_color = {0, 0, 0, 1};
    
    // init opengl vars
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);
    
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glViewport(0, 0, window_dim.width, window_dim.height);
    
    //glEnable(GL_DEBUG_OUTPUT);
    //glDebugMessageCallback(opengl_debug_message_callback, 0);
    
    while(1)
    {
        // input()
        for (u32 i = 0; i < num_of_controllers; i++)
        {
            for (u32 j = 0; j < ARRAY_COUNT(controllers[i].buttons); j++)
                controllers[i].buttons[j].previous_state = controllers[i].buttons[j].current_state;
        }
        
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
                
                case SDL_KEYDOWN:
                case SDL_KEYUP:
                {
                    menu_controller = &controllers[0];
                    SDL_KeyboardEvent *keyboard_event = &event.key;
                    s32 key_id = keyboard_event->keysym.sym;
                    b32 state = false;
                    if (keyboard_event->state == SDL_PRESSED)
                        state = true;
                    
                    controller_process_input(&controllers[0], key_id, state);
                } break;
                
                case SDL_CONTROLLERBUTTONUP:
                case SDL_CONTROLLERBUTTONDOWN:
                {
                    SDL_ControllerButtonEvent *button_event = &event.cbutton;
                    for (u32 i = 0; i < num_of_joysticks; i++)
                    {
                        if (SDL_JoystickInstanceID(joysticks[i]) == button_event->which)
                        {
                            s32 button_id = button_event->button;
                            b32 state = false;
                            if (button_event->state == SDL_PRESSED)
                                state = true;
                            
                            controller_process_input(&controllers[i + 1], button_id, state);
                            menu_controller = &controllers[i + 1];
                        }
                    }
                } break;
            }
        }
        
        // get_run_time()
        u32 run_time_ms = SDL_GetTicks();
        r32 run_time_s = (f32)run_time_ms / 1000.0f;
        u32 frame_time_ms = run_time_ms - last_run_time_ms;
        r32 frame_time_s = (f32)frame_time_ms / 1000.0f;
        last_run_time_ms = run_time_ms;
        
        f32 fps = 1000.0f;
        if (frame_time_s > 0.0f)
            fps = 1.0f / frame_time_s;
        //log("%f", fps);
        
        // update()
        if (game_mode == MAIN_MENU)
        {
            menu_update_active(&active, 0, 2, menu_controller->down, menu_controller->up);
        }
        else if (game_mode == PAUSED)
        {
            menu_update_active(&active, 0, 1, menu_controller->down, menu_controller->up);
            
            if (on_down(menu_controller->pause))
                game_mode = IN_GAME;
        }
        else if (game_mode == IN_GAME)
        {
            if (cow.num_of_inputs < 4)
            {
                if (on_down(menu_controller->right) && cow.inputs[cow.num_of_inputs] != LEFT_V)
                    cc_add_input(&cow, RIGHT_V);
                if (on_down(menu_controller->up) && cow.inputs[cow.num_of_inputs] != DOWN_V)
                    cc_add_input(&cow, UP_V);
                if (on_down(menu_controller->left) && cow.inputs[cow.num_of_inputs] != RIGHT_V)
                    cc_add_input(&cow, LEFT_V);
                if (on_down(menu_controller->down) && cow.inputs[cow.num_of_inputs] != UP_V)
                    cc_add_input(&cow, DOWN_V);
            }
            update_cc(&cow, frame_time_s, grid_dim);
            
            if (on_down(menu_controller->pause))
                game_mode = PAUSED;
        }
        
        // draw()
        u32 gl_clear_flags = GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT;
        glClear(gl_clear_flags);
        orthographic_matrix = orthographic_projection(0.0f, (r32)window_dim.width, (r32)window_dim.height,
                                                      0.0f, -3.0f, 3.0f);
        global_window_dim = window_dim;
        
        Rect window_rect = Rect{{0, 0}, cv2(window_dim)};
        
        if (game_mode == MAIN_MENU)
        {
            Menu main_menu = default_menu;
            Rect bounds = get_centered_square(window_rect, 0.7f);
            b32 select = on_down(menu_controller->select);
            
            main_menu.button.dim = { bounds.dim.width, bounds.dim.height * 0.15f };
            main_menu.button.pixel_height = main_menu.button.dim.height * 0.6f;
            main_menu.padding = bounds.dim * 0.0179f;
            v2 logo_dim = { bounds.dim.width, logo.dim.height * (bounds.dim.width / logo.dim.width) };
            
            Rect menu_rect = {};
            menu_rect.dim.x = main_menu.button.dim.width;
            menu_rect.dim.y = logo_dim.y + (main_menu.button.dim.y * 3.0f) + (main_menu.padding.y * 3.0f);
            menu_rect.coords = get_centered(menu_rect, window_rect);
            
            draw_rect(window_rect, &main_menu_back);
            draw_rect(menu_rect.coords, 0, logo_dim, &logo);
            
            menu_rect.coords.y += logo_dim.y + main_menu.padding.y;
            
            u32 index = 0;
            if (menu_button(&main_menu, menu_rect.coords, "Play", index++, active, select))
            {
                game_mode = IN_GAME;
                active = 0;
            }
            menu_rect.coords.y += main_menu.button.dim.y + main_menu.padding.y;
            
            if (menu_button(&main_menu, menu_rect.coords, "Multiplayer", index++, active, select))
            {
                game_mode = IN_GAME;
                active = 0;
            }
            menu_rect.coords.y += main_menu.button.dim.y + main_menu.padding.y;
            
            if (menu_button(&main_menu, menu_rect.coords, "Quit", index++, active, select))
            {
                return 0;
            }
        }
        else if (game_mode == IN_GAME || game_mode == PAUSED)
        {
            draw_rect(window_rect, &game_back);
            
            Rect rocks_rect = get_centered_square(window_rect, 0.9f);
            Rect grass_rect = get_centered_rect(rocks_rect, 0.75265f, 0.75265f);
            
            draw_rect(grass_rect, &grass);
            
            v2 grid_size = grass_rect.dim / cv2(grid_dim);
            
            // drawing grid
            for (s32 i = 0; i < grid_dim.x; i++)
            {
                for (s32 j = 0; j < grid_dim.y; j++) 
                {
                    draw_rect({ grass_rect.coords.x + (i * grid_size.x), grass_rect.coords.y + (j * grid_size.y)}, 
                              0, grid_size, &grid);
                }
            }
            
            draw_rect(rocks_rect, &rocks);
            draw_coffee_cow(&cow, grass_rect.coords, grid_size.x);
            
            if (game_mode == PAUSED)
            {
                Menu pause_menu = default_menu;
                Rect bounds = get_centered_square(window_rect, 0.6f);
                b32 select = on_down(menu_controller->select);
                
                pause_menu.button.dim = { bounds.dim.width, bounds.dim.height * 0.15f };
                pause_menu.button.pixel_height = pause_menu.button.dim.height * 0.6f;
                pause_menu.padding = bounds.dim * 0.0179f;
                pause_menu.button.back_color = { 0, 0, 0, 0.5f };
                pause_menu.button.active_back_color = { 0, 0, 0, 1.0f };
                pause_menu.button.text_color = { 255, 255, 255, 1 };
                pause_menu.button.active_text_color = { 255, 255, 255, 1 };
                
                Rect menu_rect = {};
                menu_rect.dim.x = pause_menu.button.dim.x;
                menu_rect.dim.y = (pause_menu.button.dim.y * 2.0f) + (pause_menu.padding.y * 1.0f);
                menu_rect.coords = get_centered(menu_rect, window_rect);
                
                Rect back_rect = get_centered_rect_pad(menu_rect, pause_menu.padding * 2);
                draw_rect(back_rect, { 0, 0, 0, 0.5f });
                
                u32 index = 0;
                
                if (menu_button(&pause_menu, menu_rect.coords, "Restart", index++, active, select))
                {
                    
                }
                
                menu_rect.coords.y += pause_menu.button.dim.y + pause_menu.padding.y;
                
                if (menu_button(&pause_menu, menu_rect.coords, "Menu", index++, active, select))
                {
                    game_mode = MAIN_MENU;
                    active = 0;
                }
            }
        }
        
        SDL_GL_SwapWindow(window);
    }
    
    return 0;
}

int main(int argc, char *argv[]) { return game(); }
