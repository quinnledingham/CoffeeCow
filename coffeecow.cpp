#include <gl.h>
#include <gl.c>
#include <SDL.h>

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_RESIZE_IMPLEMENTATION
#define STB_TRUETYPE_IMPLEMENTATION
#include <stb_image.h>
#include <stb_image_resize.h>
#include <stb_truetype.h>

#include <qlib.h>
#include <qlib_assets.h>
#include <qlib_opengl_debug.cpp>
#include <qlib_assets.cpp>
#include "coffeecow.h"
#include <qlib_application.h>

global_variable m4x4 orthographic_matrix;
global_variable Shader *color_shader;
global_variable Shader *tex_shader;

inline void
draw_rect(v2 coords, v2 dim, v4 color)
{
    draw_rect({coords.x, coords.y, 0}, get_rotation(0, {1, 0, 0}), {dim.x, dim.y, 1}, color,
              orthographic_matrix, identity_m4x4());
}

inline void
draw_rect(v2 coords, v2 dim, Bitmap *bitmap)
{
    draw_rect({coords.x, coords.y, 0}, get_rotation(0, {1, 0, 0}), {dim.x, dim.y, 1}, bitmap,
              orthographic_matrix, identity_m4x4());
}

inline void
draw_string(Font *font, const char *string, v2 coords, f32 pixel_height, v4 color)
{
    draw_string(font, string, coords, pixel_height, color,
                orthographic_matrix, identity_m4x4());
}

enum Game_Modes
{
    IN_GAME,
    MAIN_MENU,
    PAUSED,
    GAME_OVER,
};

enum Shader_Tags
{
    SHADER_COLOR_RECT_2D,
    SHADER_TEX_RECT_2D,
};

enum Bitmap_Tags
{
    BITMAP_MENU_BACKGROUND,
    BITMAP_LOGO,
};

enum Font_Tags
{
    FONT_RUBIK
};

#include "menu.h"

function b32
menu_button(Menu *menu, v2 coords, const char *text, u32 index, u32 active, u32 press)
{
    Menu_Button *button = &menu->button;
    b32 button_pressed = false;
    
    v4 b_color = button->back_color;
    v4 t_color = button->text_color;
    
    if (index == active)
    {
        b_color = button->active_back_color;
        t_color = button->active_text_color;
        if (press)
            button_pressed = true;
    }
    
    // drawing
    draw_rect(coords, button->dim, b_color);
    
    v2 text_dim = get_string_dim(menu->font, text, button->pixel_height, t_color);
    f32 text_x_coord = coords.x + (button->dim.x / 2.0f) - (text_dim.x / 2.0f);
    f32 text_y_coord = coords.y + (button->dim.y / 2.0f) + (text_dim.y / 2.0f);
    
    draw_string(menu->font, text, { text_x_coord, text_y_coord }, button->pixel_height, t_color);
    
    return button_pressed;
}

function void
do_one_frame(Application *app)
{
    Controller *controller = &app->input.controller;
    Assets *assets = &app->assets;
    Storage *storage = &app->storage;
    
    // Update
    if (storage->game_mode == MAIN_MENU)
    {
        v2s components = {0, 1};
        if (on_down(controller->down))
        {
            storage->active++;
            if (storage->active > components.y)
                storage->active = components.y;
        }
        if (on_down(controller->up))
        {
            storage->active--;
            if (storage->active < components.x)
                storage->active = components.x;
        }
    }
    else if (storage->game_mode == IN_GAME)
    {
        
    }
    
    // Draw
    
    orthographic_matrix = orthographic_projection(0.0f, (r32)app->window_dim.Width, (r32)app->window_dim.Height,
                                                  0.0f, -3.0f, 3.0f);
    
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    
    draw_rect({0, 0}, {100, 100}, {0, 255, 0, 1});
    
    if (storage->game_mode == MAIN_MENU)
    {
        draw_rect({0, 0}, cv2(app->window_dim), &assets->bitmaps[BITMAP_MENU_BACKGROUND]);
        
        Menu main_menu = {};
        v2 menu_logo_dim = {550, 274};
        main_menu.padding = {10, 10};
        main_menu.font = &assets->fonts[FONT_RUBIK];
        
        main_menu.button.dim = {550, 75};
        main_menu.button.back_color = {0, 0, 0, 1};
        main_menu.button.active_back_color = {222, 201, 179, 1};
        main_menu.button.text_color = {255, 255, 255, 1};
        main_menu.button.active_text_color = {0, 0, 0, 1};
        main_menu.button.pixel_height = 50;
        
        main_menu.dim.x = menu_logo_dim.x;
        main_menu.dim.y = menu_logo_dim.y + (main_menu.button.dim.y * 2.0f) + (main_menu.padding.y * 2.0f);
        v2 menu_coords = 
        { 
            ((f32)app->window_dim.x/2.0f) - (main_menu.dim.x / 2.0f), 
            ((f32)app->window_dim.y/2.0f) - (main_menu.dim.y / 2.0f)
        };
        
        draw_rect(menu_coords, menu_logo_dim, &assets->bitmaps[BITMAP_LOGO]);
        
        menu_coords.y += menu_logo_dim.y + main_menu.padding.y;
        
        if (menu_button(&main_menu, menu_coords, "Play", 0, storage->active, on_down(controller->select)))
        {
            
        }
        
        menu_coords.y += main_menu.button.dim.y + main_menu.padding.y;
        
        if (menu_button(&main_menu, menu_coords, "Quit", 1, storage->active, on_down(controller->select)))
        {
            app->quit = TRUE;
        }
    }
    else if (storage->game_mode == IN_GAME)
    {
        // draw game
        
        if (storage->game_mode == PAUSED)
        {
            
        }
        else if (storage->game_mode == GAME_OVER)
        {
            
        }
    }
}

function void
init_storage(Storage *storage)
{
    create_rect_mesh(&storage->rect);
    storage->game_mode = MAIN_MENU;
    storage->active = 0;
}

function void
load_assets(Assets *assets)
{
    assets->num_of_shaders = 2;
    assets->shaders = (Shader*)SDL_malloc(sizeof(Shader) * assets->num_of_shaders);
    assets->shaders[SHADER_COLOR_RECT_2D] = load_shader("../assets/shaders/color3D.vs",
                                                        0, 0, 0, "../assets/shaders/color.fs");
    assets->shaders[SHADER_TEX_RECT_2D] = load_shader("../assets/shaders/color3D.vs",
                                                      0, 0, 0, "../assets/shaders/tex.fs");
    
    assets->num_of_bitmaps = 2;
    assets->bitmaps = (Bitmap*)SDL_malloc(sizeof(Bitmap) * assets->num_of_bitmaps);
    assets->bitmaps[BITMAP_MENU_BACKGROUND] = load_bitmap("../assets/bitmaps/mainmenuback.png");
    init_bitmap_handle(&assets->bitmaps[BITMAP_MENU_BACKGROUND]);
    assets->bitmaps[BITMAP_LOGO] = load_bitmap("../assets/bitmaps/logo.png");
    init_bitmap_handle(&assets->bitmaps[BITMAP_LOGO]);
    
    assets->num_of_fonts = 1;
    assets->fonts = (Font*)SDL_malloc(sizeof(Font) * assets->num_of_fonts);
    assets->fonts[FONT_RUBIK] = load_font("../assets/fonts/Rubik-Medium.ttf");
}

function void
init_controller(Controller *controller)
{
    set(&controller->right, KEY_D);
    set(&controller->up, KEY_W);
    set(&controller->left, KEY_A);
    set(&controller->down, KEY_S);
    set(&controller->select, KEY_RETURN);
}

function void
init_app(Application *app)
{
    init_controller(&app->input.controller);
    load_assets(&app->assets);
    init_storage(&app->storage);
    
    color_shader = &app->assets.shaders[SHADER_COLOR_RECT_2D];
    tex_shader = &app->assets.shaders[SHADER_TEX_RECT_2D];
    
    // Default OpenGL Settings
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);
    glPointSize(5.0f);
    glPatchParameteri(GL_PATCH_VERTICES, 4);
    
    glEnable(GL_DEBUG_OUTPUT);
    glDebugMessageCallback(opengl_debug_message_callback, 0);
    
    glViewport(0, 0, app->window_dim.Width, app->window_dim.Height);
}