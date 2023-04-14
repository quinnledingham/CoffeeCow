#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_RESIZE_IMPLEMENTATION
#define STB_TRUETYPE_IMPLEMENTATION
#include <stb_image.h>
#include <stb_image_resize.h>
#include <stb_truetype.h>
#include <gl.h>
#include <gl.c>
#include <SDL.h>
#include <ctime>

#include "log.h"
#include "types.h"
#include "assets.h"
#include "application.h"
#include "coffee_cow.h"

#include "assets.cpp"
#include "rect.cpp"
#include "menu.cpp"
#include "coffee_cow.cpp"

enum Game_Modes
{
    MAIN_MENU,
    IN_GAME,
    PAUSED,
    GAME_OVER,
    
    NUM_OF_GAME_MODES
};

struct Game_Data
{
    v2s grid_dim;
    Coffee_Cow_Design designs[4];
    Coffee_Cow players[4];
    u32 num_of_players;
    //v2s coffees[10];
    Coffee coffees[10];
    u32 num_of_coffees;
    
    u32 game_mode = MAIN_MENU;
    s32 active;
    Menu default_menu;
    
    Menu menus[NUM_OF_GAME_MODES];
};

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
init_controllers(Input *input)
{
    input->active_controller = &input->controllers[0];
    
    Controller *keyboard = &input->controllers[0];
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
    
    input->num_of_controllers = 1; // keyboard
    
    log("Game Controllers:");
    SDL_Joystick **joysticks = input->joysticks;
    SDL_GameController **game_controllers = input->game_controllers;
    input->num_of_joysticks = SDL_NumJoysticks();
    for (u32 i = 0; i < input->num_of_joysticks; i++)
    {
        joysticks[i] = SDL_JoystickOpen(i);
        if (SDL_IsGameController(i))
        {
            log("%s", SDL_JoystickName(joysticks[i]));
            
            game_controllers[i] = SDL_GameControllerOpen(i);
            
            Controller *c = &input->controllers[i + 1];
            set(&c->right, SDL_CONTROLLER_BUTTON_DPAD_RIGHT);
            set(&c->up, SDL_CONTROLLER_BUTTON_DPAD_UP);
            set(&c->left, SDL_CONTROLLER_BUTTON_DPAD_LEFT);
            set(&c->down, SDL_CONTROLLER_BUTTON_DPAD_DOWN);
            set(&c->select, SDL_CONTROLLER_BUTTON_A);
            set(&c->pause, SDL_CONTROLLER_BUTTON_START);
            
            input->num_of_controllers++;
        }
    }
}

function void*
init_game_data(Assets *assets)
{
    Game_Data *data = (Game_Data*)malloc(sizeof(Game_Data));
    *data = {};
    data->grid_dim = { 10, 10 };
    
    Coffee_Cow_Design *designs = data->designs;
    designs[0].bitmaps[ASSET_COW_HEAD] = find_bitmap(assets, "COW1_HEAD");
    designs[0].bitmaps[ASSET_COW_HEAD_OUTLINE] = find_bitmap(assets, "COW1_HEAD_OUTLINE");
    designs[0].bitmaps[ASSET_COW_CIRCLE] = find_bitmap(assets, "COW1_CIRCLE");
    designs[0].bitmaps[ASSET_COW_CIRCLE_OUTLINE] = find_bitmap(assets, "COW1_CIRCLE_OUTLINE");
    designs[0].color = { 255, 255, 255, 1 };
    designs[0].outline_color = { 0, 0, 0, 1 };
    
    // default menu
    Menu *default_menu = &data->default_menu;
    default_menu->font = find_font(assets, "RUBIK");
    
    default_menu->button.back_color = {0, 0, 0, 1};
    default_menu->button.active_back_color = {222, 201, 179, 1};
    default_menu->button.text_color = {255, 255, 255, 1};
    default_menu->button.active_text_color = {0, 0, 0, 1};
    
    default_menu->window_percent = 0.7f;
    default_menu->padding_percent = 0.0179f;
    default_menu->button_percent = 0.15f;
    default_menu->pixel_height_percent = 0.6f;
    
    return (void*)data;
}

function b32
update(Application *app)
{
    Game_Data *data = (Game_Data*)app->data;
    Controller *menu_controller = app->input.active_controller;
    
    u32 num_of_players = data->num_of_players;
    Coffee_Cow *players = data->players;
    v2s window_dim = app->window.dim;
    
    // what to update
    switch(data->game_mode)
    {
        case MAIN_MENU:
        {
            menu_update_active(&data->active, 0, 2, menu_controller->down, menu_controller->up);
        } break;
        
        case PAUSED:
        case GAME_OVER:
        {
            menu_update_active(&data->active, 0, 1, menu_controller->down, menu_controller->up);
            
            if (on_down(menu_controller->pause))
                data->game_mode = IN_GAME;
        } break;
        
        case IN_GAME:
        {
            update_coffee_cows(players, num_of_players, app->time.frame_time_s, data->grid_dim);
            
            for (u32 i = 0; i < num_of_players; i++) 
            {
                if (players[i].dead) 
                {
                    data->game_mode = GAME_OVER;
                    players[i].dead = false;
                }
            }
            
            update_coffees(data->coffees, data->num_of_coffees);
            coffee_cows_collsions(players, num_of_players, data->coffees, data->num_of_coffees, data->grid_dim);
            
            if (on_down(menu_controller->pause))
                data->game_mode = PAUSED;
        } break;
    }
    
    u32 gl_clear_flags = 
        GL_COLOR_BUFFER_BIT | 
        GL_DEPTH_BUFFER_BIT | 
        GL_STENCIL_BUFFER_BIT;
    
    glClear(gl_clear_flags);
    
    Rect window_rect = Rect{{0, 0}, cv2(window_dim)};
    
    switch (data->game_mode)
    {
        case MAIN_MENU:
        {
            Bitmap *logo = find_bitmap(&app->assets, "LOGO");
            Bitmap *main_menu_back = find_bitmap(&app->assets, "MAIN_MENU_BACK");
            
            Menu main_menu = data->default_menu;
            b32 select = on_down(menu_controller->select);
            
            resize_menu(&main_menu, window_rect, logo->dim, 1, 3);
            
            draw_rect(window_rect, main_menu_back);
            
            draw_rect(main_menu.rect.coords, 0, main_menu.bitmap.dim, logo);
            main_menu.rect.coords.y += main_menu.bitmap.dim.y + main_menu.padding.y;
            
            u32 index = 0;
            if (menu_button(&main_menu, "Play", index++, data->active, select))
            {
                data->game_mode = IN_GAME;
                data->active = 0;
                
                init_cc(&players[0], players, data->designs[0], &app->input.controllers[0], data->grid_dim);
                data->num_of_players = 1;
                
                random_coffee_locaton(&data->coffees[0].coords, data->grid_dim, players, num_of_players);
                data->num_of_coffees = 1;
            }
            
            if (menu_button(&main_menu, "Multiplayer", index++, data->active, select))
            {
                data->game_mode = IN_GAME;
                data->active = 0;
            }
            
            if (menu_button(&main_menu, "Quit", index++, data->active, select))
            {
                return true;
            }
            
        } break;
        
        case IN_GAME:
        case PAUSED:
        case GAME_OVER:
        {
            Bitmap *game_back = find_bitmap(&app->assets, "GAME_BACK");
            Bitmap *grass = find_bitmap(&app->assets, "GRASS");
            Bitmap *grid = find_bitmap(&app->assets, "GRID");
            Bitmap *rocks = find_bitmap(&app->assets, "ROCKS");
            
            draw_rect(window_rect, game_back);
            
            Rect rocks_rect = get_centered_square(window_rect, 0.9f);
            Rect grass_rect = get_centered_rect(rocks_rect, 0.75265f, 0.75265f);
            
            draw_rect(grass_rect, grass);
            
            v2 grid_size = grass_rect.dim / cv2(data->grid_dim);
            
            // drawing grid
            for (s32 i = 0; i < data->grid_dim.x; i++)
            {
                for (s32 j = 0; j < data->grid_dim.y; j++) 
                {
                    draw_rect({ grass_rect.coords.x + (i * grid_size.x), grass_rect.coords.y + (j * grid_size.y)}, 0, grid_size, grid);
                }
            }
            
            draw_rect(rocks_rect, rocks);
            
            r32 score_pixel_height = window_dim.y * 0.1f;
            r32 score_pixel_height_outline  = score_pixel_height + 5.0f; 
            Font *rubik = find_font(&app->assets, "RUBIK");
            draw_string(rubik, u32_to_string(players[0].score), { 5, score_pixel_height_outline - 10 }, score_pixel_height_outline, { 0, 0, 0, 1 });
            draw_string(rubik, u32_to_string(players[0].score), { 10, score_pixel_height - 10 }, score_pixel_height, { 255, 255, 255, 1 });
            
            for (u32 i = 0; i < data->num_of_coffees; i++) draw_rect(grass_rect.coords + (cv2(data->coffees[i].coords) * grid_size), DEG2RAD * data->coffees[i].rotation, grid_size, find_bitmap(&app->assets, "COFFEE"));
            for (u32 i = 0; i < num_of_players; i++) draw_coffee_cow(&players[i], grass_rect.coords, grid_size.x);
            
            if (data->game_mode == PAUSED || data->game_mode == GAME_OVER)
            {
                Menu pause_menu = data->default_menu;
                b32 select = on_down(menu_controller->select);
                
                pause_menu.button.back_color = { 0, 0, 0, 0.5f };
                pause_menu.button.active_back_color = { 0, 0, 0, 1.0f };
                pause_menu.button.text_color = { 255, 255, 255, 1 };
                pause_menu.button.active_text_color = { 255, 255, 255, 1 };
                
                pause_menu.window_percent = 0.6f;
                
                resize_menu(&pause_menu, window_rect, {1,1}, 0, 2);
                
                Rect back_rect = get_centered_rect_pad(pause_menu.rect, pause_menu.padding * 2);
                draw_rect(back_rect, { 0, 0, 0, 0.5f });
                
                u32 index = 0;
                
                if (menu_button(&pause_menu, "Restart", index++, data->active, select))
                {
                    data->game_mode = IN_GAME;
                    data->active = 0;
                    
                    init_cc(&players[0], players, data->designs[0], &app->input.controllers[0], data->grid_dim);
                    
                    random_coffee_locaton(&data->coffees[0].coords, data->grid_dim, players, num_of_players);
                    data->num_of_coffees = 1;
                }
                
                if (menu_button(&pause_menu, "Menu", index++, data->active, select))
                {
                    data->game_mode = MAIN_MENU;
                    data->active = 0;
                    
                    players[0] = {};
                    
                    init_cc(&players[0], players, data->designs[0], &app->input.controllers[0], data->grid_dim);
                    
                    random_coffee_locaton(&data->coffees[0].coords, data->grid_dim, players, num_of_players);
                    data->num_of_coffees = 1;
                }
            }
        } break;
    }
    
    //Font *rubik = find_font(&app->assets, "RUBIK");
    //draw_string(rubik, ftos(app->time.frames_per_s), { 100, 100 }, 50, { 255, 150, 0, 1 });
    
    return false;
}

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

function b32
process_input(v2s *window_dim, Input *input)
{
    for (u32 i = 0; i < input->num_of_controllers; i++)
    {
        for (u32 j = 0; j < ARRAY_COUNT(input->controllers[i].buttons); j++)
            input->controllers[i].buttons[j].previous_state = input->controllers[i].buttons[j].current_state;
    }
    
    SDL_Event event;
    while(SDL_PollEvent(&event))
    {
        switch(event.type)
        {
            case SDL_QUIT: return true;
            
            case SDL_WINDOWEVENT:
            {
                SDL_WindowEvent *window_event = &event.window;
                
                switch(window_event->event)
                {
                    case SDL_WINDOWEVENT_RESIZED:
                    case SDL_WINDOWEVENT_SIZE_CHANGED:
                    {
                        window_dim->width = window_event->data1;
                        window_dim->height = window_event->data2;
                        glViewport(0, 0, window_dim->width, window_dim->height);
                    } break;
                }
            } break;
            
            case SDL_KEYDOWN:
            case SDL_KEYUP:
            {
                input->active_controller = &input->controllers[0];
                SDL_KeyboardEvent *keyboard_event = &event.key;
                s32 key_id = keyboard_event->keysym.sym;
                b32 state = false;
                if (keyboard_event->state == SDL_PRESSED)
                    state = true;
                
                controller_process_input(&input->controllers[0], key_id, state);
            } break;
            
            case SDL_CONTROLLERBUTTONUP:
            case SDL_CONTROLLERBUTTONDOWN:
            {
                SDL_ControllerButtonEvent *button_event = &event.cbutton;
                for (u32 i = 0; i < input->num_of_joysticks; i++)
                {
                    if (SDL_JoystickInstanceID(input->joysticks[i]) == button_event->which)
                    {
                        s32 button_id = button_event->button;
                        b32 state = false;
                        if (button_event->state == SDL_PRESSED)
                            state = true;
                        
                        controller_process_input(&input->controllers[i + 1], button_id, state);
                        input->active_controller = &input->controllers[i + 1];
                    }
                }
            } break;
        }
    }
    
    return false;
}

function void
update_time(Time *time)
{
    u32 last_run_time_ms = time->run_time_ms;
    
    time->run_time_ms = SDL_GetTicks();
    time->run_time_s = (f32)time->run_time_ms / 1000.0f;
    time->frame_time_ms = time->run_time_ms - last_run_time_ms;
    time->frame_time_s = (f32)time->frame_time_ms / 1000.0f;
    
    // get fps
    time->frames_per_s = 1000.0f;
    if (time->frame_time_s > 0.0f)
        time->frames_per_s = 1.0f / time->frame_time_s;
}

function int
main_loop(Application *app)
{
    init_controllers(&app->input);
    app->data = init_game_data(&app->assets);
    
    while(1)
    {
        if (process_input(&app->window.dim, &app->input)) return 0; // quit if input to quit
        
        update_time(&app->time);
        
        set_orthographic_matrix(app->window.dim);
        
        if (update(app)) return 0; // quit if update says to quit
        
        swap_window(&app->window);
    }
}

function void
init_opengl(Window *window)
{
    SDL_GL_LoadLibrary(NULL);
    
    // Request an OpenGL 4.6 context (should be core)
    SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 6);
    
    // Also request a depth buffer
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    
    SDL_GLContext Context = SDL_GL_CreateContext(window->sdl);
    SDL_GL_SetSwapInterval(0);
    
    // Check OpenGL properties
    gladLoadGL((GLADloadfunc) SDL_GL_GetProcAddress);
    log("OpenGL loaded:");
    log("Vendor:   %s", glGetString(GL_VENDOR));
    log("Renderer: %s", glGetString(GL_RENDERER));
    log("Version:  %s", glGetString(GL_VERSION));
    
    SDL_GetWindowSize(window->sdl, &window->dim.width, &window->dim.height);
    glViewport(0, 0, window->dim.width, window->dim.height);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
}

function void
init_window(Window *window)
{
    u32 sdl_init_flags = 
        SDL_INIT_VIDEO | 
        SDL_INIT_GAMECONTROLLER | 
        SDL_INIT_HAPTIC | 
        SDL_INIT_AUDIO;
    
    u32 sdl_window_flags = 
        SDL_WINDOW_RESIZABLE | 
        SDL_WINDOW_OPENGL;
    
    SDL_Init(sdl_init_flags);
    
    window->sdl = SDL_CreateWindow("Coffee Cow", 
                                   SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                                   800, 800, 
                                   sdl_window_flags);
    
    init_opengl(window);
    
    SDL_GetWindowSize(window->sdl, &window->dim.width, &window->dim.height);
}

function int
application()
{
    Application app = {};
    init_window(&app.window);
    load_assets(&app.assets, "../assets.ethan");
    return main_loop(&app);
}

int main(int argc, char *argv[]) { return application();}

