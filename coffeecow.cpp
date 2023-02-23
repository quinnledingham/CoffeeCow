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

#define QLIB_CONTROLLER
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
        };
        Button buttons[5];
    };
};

#define QLIB_STORAGE
struct Storage
{
    Mesh rect;
    
    u32 game_mode;
    s32 active;
};

#include <qlib_application.h>

global_variable Mesh rect = {};
global_variable m4x4 orthographic_matrix;
global_variable Shader *color_shader;
global_variable Shader *tex_shader;

inline void
draw_rect(v2 coords, v2 dim, v4 color)
{
    draw_rect({coords.x, coords.y, 0}, get_rotation(0, {1, 0, 0}), {dim.x, dim.y, 1}, color,
              &rect, color_shader, orthographic_matrix, identity_m4x4());
}

inline void
draw_rect(v2 coords, v2 dim, Bitmap *bitmap)
{
    draw_rect({coords.x, coords.y, 0}, get_rotation(0, {1, 0, 0}), {dim.x, dim.y, 1}, bitmap,
              &rect, tex_shader, orthographic_matrix, identity_m4x4());
}

inline void
draw_string(Font *font, const char *string, v2 coords, f32 pixel_height, v4 color)
{
    draw_string(font, string, coords, pixel_height, color,
                &rect, tex_shader, orthographic_matrix, identity_m4x4());
}

enum Game_Modes
{
    IN_GAME,
    MAIN_MENU,
    PAUSED,
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

// returns true if the button is pressed
function b32
menu_button(v2 coords, v2 dim, v4 back_color, v4 active_back_color,
            const char *text, Font *font, f32 pixel_height, v4 text_color, v4 active_text_color,
            u32 index, u32 active, u32 press)
{
    b32 button_pressed = false;
    
    v4 b_color = back_color;
    v4 t_color = text_color;
    
    if (index == active)
    {
        b_color = active_back_color;
        t_color = active_text_color;
        
        if (press)
            button_pressed = true;
    }
    
    // drawing
    draw_rect(coords, dim, b_color);
    
    v2 text_dim = get_string_dim(font, text, pixel_height, t_color);
    f32 text_x_coord = coords.x + (dim.x / 2.0f) - (text_dim.x / 2.0f);
    f32 text_y_coord = coords.y + (dim.y / 2.0f) + (text_dim.y / 2.0f);
    
    draw_string(font, text, { text_x_coord, text_y_coord }, pixel_height, t_color);
    
    return button_pressed;
}

function void
do_one_frame(Application *app)
{
    Controller *controller = &app->input.controller;
    Assets *assets = &app->assets;
    Storage *storage = &app->storage;
    
    if (on_down(controller->left))
        printf("yo\n");
    
    orthographic_matrix = orthographic_projection(0.0f, (r32)app->window_dim.Width, (r32)app->window_dim.Height,
                                                  0.0f, -3.0f, 3.0f);
    
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    
    draw_rect({0, 0}, {100, 100}, {0, 255, 0, 1});
    
    
    if (storage->game_mode == MAIN_MENU)
    {
        v2s components = {0, 1};
        v2 menu_padding = {10, 10};
        
        v2 menu_logo_dim = {550, 274};
        
        v2 menu_button_dim = {550, 75};
        v4 menu_button_back_color = {0, 0, 0, 1};
        v4 menu_button_active_back_color = {222, 201, 179, 1};
        v4 menu_button_text_color = {255, 255, 255, 1};
        v4 menu_button_active_text_color = {0, 0, 0, 1};
        f32 button_pixel_height = 50;
        
        Font *menu_font = &assets->fonts[FONT_RUBIK];
        
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
        
        f32 menu_dim_x = menu_logo_dim.x;
        f32 menu_dim_y = menu_logo_dim.y + (menu_button_dim.y * 2.0f) + (menu_padding.y * 2.0f);
        v2 menu_coords = 
        { 
            ((f32)app->window_dim.x/2.0f) - (menu_dim_x / 2.0f), 
            ((f32)app->window_dim.y/2.0f) - (menu_dim_y / 2.0f)
        };
        
        draw_rect({0, 0}, cv2(app->window_dim), &assets->bitmaps[BITMAP_MENU_BACKGROUND]);
        draw_rect(menu_coords, menu_logo_dim, &assets->bitmaps[BITMAP_LOGO]);
        
        menu_coords.y += menu_logo_dim.y + menu_padding.y;
        
        if (menu_button(menu_coords, menu_button_dim, menu_button_back_color, menu_button_active_back_color,
                        "Play", menu_font, button_pixel_height, menu_button_text_color, menu_button_active_text_color,
                        0, storage->active, on_down(controller->select)))
        {
            
        }
        
        menu_coords.y += menu_button_dim.y + menu_padding.y;
        
        if (menu_button(menu_coords, menu_button_dim, menu_button_back_color, menu_button_active_back_color,
                        "Quit", menu_font, button_pixel_height, menu_button_text_color, menu_button_active_text_color,
                        1, storage->active, on_down(controller->select)))
        {
            app->quit = TRUE;
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
    
    create_rect_mesh(&rect);
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