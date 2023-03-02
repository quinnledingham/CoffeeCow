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
#include "assets.h"
#include "assets.cpp"

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

global_variable m4x4 orthographic_matrix;
global_variable v2s global_window_dim;

function void
init_rect_indices(u32 *indices, 
                  u32 top_left, 
                  u32 top_right,
                  u32 bottom_left,
                  u32 bottom_right)
{
    indices[0] = top_left;
    indices[1] = bottom_left;
    indices[2] = bottom_right;
    indices[3] = top_left;
    indices[4] = bottom_right;
    indices[5] = top_right;
}

function void
init_rect_mesh(Mesh *rect)
{
    rect->vertices_count = 4;
    rect->vertices = (Vertex*)SDL_malloc(sizeof(Vertex) * rect->vertices_count);
    /*
    rect->vertices[0] = { {0, 0, 0}, {0, 0, 1}, {0, 0} };
    rect->vertices[1] = { {0, 1, 0}, {0, 0, 1}, {0, 1} };
    rect->vertices[2] = { {1, 0, 0}, {0, 0, 1}, {1, 0} };
    rect->vertices[3] = { {1, 1, 0}, {0, 0, 1}, {1, 1} };
    */
    rect->vertices[0] = { {-0.5, -0.5, 0}, {0, 0, 1}, {0, 0} };
    rect->vertices[1] = { {-0.5, 0.5, 0}, {0, 0, 1}, {0, 1} };
    rect->vertices[2] = { {0.5, -0.5, 0}, {0, 0, 1}, {1, 0} };
    rect->vertices[3] = { {0.5, 0.5, 0}, {0, 0, 1}, {1, 1} };
    
    rect->indices_count = 6;
    rect->indices = (u32*)SDL_malloc(sizeof(u32) * rect->indices_count);
    init_rect_indices(rect->indices, 1, 3, 0, 2);
    
    init_mesh(rect);
}

function void
draw_rect(v3 coords, quat rotation, v3 dim, u32 handle, m4x4 projection_matrix, m4x4 view_matrix)
{
    m4x4 model = create_transform_m4x4(coords, rotation, dim);
    glUniformMatrix4fv(glGetUniformLocation(handle, "model"), (GLsizei)1, false, (float*)&model);
    glUniformMatrix4fv(glGetUniformLocation(handle, "projection"), (GLsizei)1, false, (float*)&projection_matrix);
    glUniformMatrix4fv(glGetUniformLocation(handle, "view"), (GLsizei)1, false, (float*)&view_matrix);
    
    local_persist Mesh rect = {};
    if (rect.vertices_count == 0)
        init_rect_mesh(&rect);
    
    draw_mesh(&rect);
}

function void
draw_rect(v3 coords, quat rotation, v3 dim, v4 color,
          m4x4 projection_matrix, m4x4 view_matrix)
{
    // it makes sense to have the shader local because these functions are tailored to
    // this shaders.
    local_persist Shader shader = {};
    if (!shader.compiled)
    {
        shader.vs_file = basic_vs;
        shader.fs_file = color_fs;
        compile_shader(&shader);
    }
    
    u32 handle = use_shader(&shader);
    glUniform4fv(glGetUniformLocation(handle, "user_color"), (GLsizei)1, (float*)&color);
    draw_rect(coords, rotation, dim, handle, projection_matrix, view_matrix);
}

function void
draw_rect(v3 coords, quat rotation, v3 dim, Bitmap *bitmap,
          m4x4 projection_matrix, m4x4 view_matrix)
{
    local_persist Shader shader = {};
    if (!shader.compiled)
    {
        shader.vs_file = basic_vs;
        shader.fs_file = tex_fs;
        compile_shader(&shader);
    }
    
    u32 handle = use_shader(&shader);
    
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, bitmap->handle);
    glUniform1i(glGetUniformLocation(handle, "tex0"), 0);
    
    draw_rect(coords, rotation, dim, handle, projection_matrix, view_matrix);
}

function void
draw_rect(v2 coords, r32 rotation, v2 dim, v4 color)
{
    coords += dim / 2.0f;
    draw_rect({ coords.x, coords.y, 0 }, get_rotation(rotation, { 0, 0, 1 }), 
              { dim.x, dim.y, 1 }, color, orthographic_matrix, identity_m4x4());
}

function void
draw_rect(v2 coords, r32 rotation, v2 dim, Bitmap *bitmap)
{
    coords += dim / 2.0f;
    draw_rect({ coords.x, coords.y, 0 }, get_rotation(-rotation, { 0, 0, 1 }), 
              { dim.x, dim.y, 1 }, bitmap, orthographic_matrix, identity_m4x4());
}

struct Rect
{
    v2 coords;
    v2 dim;
};
function void draw_rect(Rect rect, Bitmap *bitmap) { draw_rect(rect.coords, 0.0f, rect.dim, bitmap); }

function void
draw_string(Font *font, const char *string, v2 coords, f32 pixel_height, v4 color)
{
    f32 scale = stbtt_ScaleForPixelHeight(&font->info, pixel_height);
    f32 string_x_coord = coords.x;
    
    u32 i = 0;
    while (string[i] != 0)
    {
        Font_Char *font_char = load_font_char(font, string[i], scale, color);
        
        f32 y = coords.y + font_char->c_y1;
        f32 x = string_x_coord + (font_char->lsb * scale);
        v2 dim = { f32(font_char->c_x2 - font_char->c_x1), f32(font_char->c_y2 - font_char->c_y1) };
        
        draw_rect({ x, y }, 0, {dim.x, dim.y}, &font_char->bitmap);
        
        int kern = stbtt_GetCodepointKernAdvance(&font->info, string[i], string[i + 1]);
        string_x_coord += ((kern + font_char->ax) * scale);
        
        i++;
    }
}

struct Menu_Button
{
    v2 dim;
    v4 back_color;
    v4 active_back_color;
    v4 text_color;
    v4 active_text_color;
    f32 pixel_height;
};

struct Menu
{
    Menu_Button button;
    
    Menu_Button buttons[10];
    u32 num_of_buttons;
    
    v2 padding;
    v2 dim;
    Font *font;
};

function void
menu_update_active(s32 *active, s32 lower, s32 upper, Button increase, Button decrease)
{
    if (on_down(increase))
    {
        (*active)++;
        if (*active > upper)
            *active = upper;
    }
    if (on_down(decrease))
    {
        (*active)--;
        if (*active < lower)
            *active = lower;
    }
}

function v2
get_center_menu_coords(v2 menu_dim, v2 window_dim)
{
    return
    { 
        (window_dim.x/2.0f) - (menu_dim.x / 2.0f), 
        (window_dim.y/2.0f) - (menu_dim.y / 2.0f)
    };
}

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
    draw_rect(coords, 0, button->dim, b_color);
    
    v2 text_dim = get_string_dim(menu->font, text, button->pixel_height, t_color);
    f32 text_x_coord = coords.x + (button->dim.x / 2.0f) - (text_dim.x / 2.0f);
    f32 text_y_coord = coords.y + (button->dim.y / 2.0f) + (text_dim.y / 2.0f);
    
    draw_string(menu->font, text, { text_x_coord, text_y_coord }, button->pixel_height, t_color);
    
    return button_pressed;
}



function Rect
get_centered_coords(r32 percent, v2 window_dim)
{
    Rect rect = {};
    r32 size = 0.0f;
    if (window_dim.y <= window_dim.x)
        size = window_dim.y * percent;
    if (window_dim.y > window_dim.x)
        size = window_dim.x * percent;
    rect.dim = { size, size };
    rect.coords = get_center_menu_coords(rect.dim, window_dim);
    return rect;
}

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
    ASSET_COW_STRAIGHT,
    ASSET_COW_STRAIGHT_OUTLINE,
};

struct Coffee_Cow_Node
{
    v2s coords;
    v2s direction;
    v2s last_direction;
};

struct Coffee_Cow
{
    Coffee_Cow_Node nodes[400];  // 0 = head
    u32 num_of_nodes;
    r32 transition; // 0.0f - 1.0f
    v2s direction;
    
    v2s inputs[5];
    u32 num_of_inputs;
    
    Bitmap bitmaps[6];
};

function void
add_node(Coffee_Cow *cow, v2s grid_coords)
{
    Coffee_Cow_Node *new_node = &cow->nodes[cow->num_of_nodes++];
    new_node->coords = grid_coords;
    new_node->direction = cow->direction;
}

function b8
coffee_cow_check_bounds(Coffee_Cow *cow, v2s grid_dim)
{
    Coffee_Cow_Node *head_node = &cow->nodes[0];
    v2s head = head_node->coords;
    head += cow->direction;
    if (head.x < 0 || head.x > (grid_dim.x - 1))
        return false;
    else if (head.y < 0 || head.y > (grid_dim.y - 1))
        return false;
    
    for (u32 i = 1; i < cow->num_of_nodes; i++)
    {
        Coffee_Cow_Node *node = &cow->nodes[i];
        if (head == node->coords)
            return false;
    }
    
    return true;
}

function void
update_coffee_cow(Coffee_Cow *cow, r32 frame_time_s, v2s grid_dim)
{
    if (cow->num_of_inputs != 0)
    {
        cow->direction = cow->inputs[0];
        for (u32 i = 0; i < cow->num_of_inputs - 2; i++)
        {
            cow->inputs[i] = cow->inputs[i + 1];
        }
        cow->num_of_inputs--;
    }
    
    if (!coffee_cow_check_bounds(cow, grid_dim))
        return;
    
    cow->transition += 4.0f * frame_time_s;
    if (cow->transition >= 1.0f)
    {
        cow->nodes[0].direction = cow->direction;
        
        cow->transition = 0.0f;
        for (u32 i = 0; i < cow->num_of_nodes; i++)
        {
            cow->nodes[i].coords += cow->nodes[i].direction;
            cow->nodes[i].last_direction = cow->nodes[i].direction;
            if (i != 0)
                cow->nodes[i].direction = cow->nodes[i - 1].last_direction;
        }
    }
}

function r32
v2toDeg(v2s v)
{
    return tanf((r32)v.y/(r32)v.x);
}

enum Direction
{
    RIGHT,
    UP,
    LEFT,
    DOWN
};

#define RIGHT_V v2s{ 1, 0 }
#define UP_V v2s{ 0, -1 }
#define LEFT_V v2s{ -1, 0 }
#define DOWN_V v2s{ 0, 1 }

function u32
get_direction(v2s dir)
{
    if (dir.x == 1 && dir.y == 0)
        return RIGHT;
    else if (dir.x == 0 && dir.y == -1)
        return UP;
    else if (dir.x == -1 && dir.y == 0)
        return LEFT;
    else 
        return DOWN;
}

function v2s
get_direction(u32 dir)
{
    if (dir == RIGHT)
        return { 1, 0 };
    if (dir == UP)
        return { 0, 1 };
    if (dir == LEFT)
        return { -1, 0 };
    else // DOWN
        return { 0, -1 };
}

function r32
get_rot(u32 dir)
{
    r32 rotation = 0.0f;
    if (dir == RIGHT)
        rotation = 90.0f;
    else if (dir == UP)
        rotation = 180.0f;
    else if (dir == LEFT)
        rotation = 270.0f;
    else if (dir == DOWN)
        rotation = 0.0f;
    return rotation * DEG2RAD;
}

function Rect
get_centered_rect(r32 percent_x, r32 percent_y, v2 dim)
{
    Rect rect = {};
    rect.dim.x = dim.x * percent_x;
    rect.dim.y = dim.y * percent_y;
    rect.coords = get_center_menu_coords(rect.dim, dim);
    return rect;
}

function Rect
get_body_rect(u32 dir, r32 min, r32 max, v2 grid_s, v2 coords)
{
    Rect cow_node = {};
    if (dir == RIGHT)
        cow_node = get_centered_rect(max, min, grid_s);
    else if (dir == UP)
        cow_node = get_centered_rect(min, max, grid_s);
    else if (dir == LEFT)
        cow_node = get_centered_rect(max, min, grid_s);
    else if (dir == DOWN)
        cow_node = get_centered_rect(min, max, grid_s);
    cow_node.coords += coords;
    return cow_node;
}

function void
draw_coffee_cow(Coffee_Cow *cow, v2 grid_coords, r32 grid_size)
{
    v2 grid_s = { grid_size, grid_size };
    
    u32 head_index = 0;
    u32 tail_index =  cow->num_of_nodes - 1;
    for (s32 i = tail_index; i >= 0; i--)
    {
        Coffee_Cow_Node *node = &cow->nodes[i];
        v2 coords = { node->coords.x * grid_size, node->coords.y * grid_size };
        coords += grid_coords;
        
        
        if (i == head_index)
        {
            r32 rot = get_rot(get_direction(cow->direction));
            draw_rect(coords, rot, grid_s, &cow->bitmaps[ASSET_COW_HEAD_OUTLINE]);
            draw_rect(coords, rot, grid_s, { 0, 0, 0, 1 });
            Rect cow_node = get_body_rect(get_direction(cow->direction), 0.9f, 1.0f, grid_s, coords);
            draw_rect(cow_node.coords, 0, cow_node.dim, { 255, 255, 255, 1 });
            draw_rect(coords, rot, grid_s, &cow->bitmaps[ASSET_COW_HEAD]);
        }
        else if (i == tail_index)
        {
            draw_rect(coords, 0, grid_s, &cow->bitmaps[ASSET_COW_CIRCLE_OUTLINE]);
            draw_rect(coords, 0, grid_s, &cow->bitmaps[ASSET_COW_CIRCLE]);
        }
        else
        {
            r32 rot = get_rot(get_direction(node->direction));
            if (cow->nodes[i + 1].direction != node->direction)
            {
                draw_rect(coords, rot, grid_s, &cow->bitmaps[ASSET_COW_CIRCLE_OUTLINE]);
                draw_rect(coords, rot, grid_s, &cow->bitmaps[ASSET_COW_CIRCLE]);
            }
            else
            {
                draw_rect(coords, rot, grid_s, { 0, 0, 0, 1 });
                Rect cow_node = get_body_rect(get_direction(node->direction), 0.9f, 1.0f, grid_s, coords);
                draw_rect(cow_node.coords, 0, cow_node.dim, { 255, 255, 255, 1 });
            }
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
    log("OpenGL loaded");
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
    Bitmap test = load_and_init_bitmap("../assets/bitmaps/testimg.jpg");
    
    Font rubik = load_font("../assets/fonts/Rubik-Medium.ttf");
    
    // init_vars()
    Controller controller = {};
    controller.right.id = SDLK_d;
    controller.up.id = SDLK_w;
    controller.left.id = SDLK_a;
    controller.down.id = SDLK_s;
    controller.select.id = SDLK_RETURN;
    controller.pause.id = SDLK_ESCAPE;
    
    u32 last_run_time_ms = 0;
    v2s window_dim = {};
    SDL_GetWindowSize(window, &window_dim.width, &window_dim.height);
    
    u32 game_mode = MAIN_MENU;
    s32 active = 0;
    
    v2s grid_dim = { 10, 10 };
    Coffee_Cow cow = {};
    cow.direction = { 0, 1 };
    add_node(&cow, { 0, 3 });
    add_node(&cow, { 0, 2 });
    add_node(&cow, { 0, 1 });
    add_node(&cow, { 0, 0 });
    
    cow.bitmaps[ASSET_COW_HEAD] = load_and_init_bitmap("../assets/bitmaps/cow1/cowhead.png");
    cow.bitmaps[ASSET_COW_HEAD_OUTLINE]= load_and_init_bitmap("../assets/bitmaps/cow1/cowheadoutline.png");
    cow.bitmaps[ASSET_COW_CIRCLE]= load_and_init_bitmap("../assets/bitmaps/cow1/circle.png");
    cow.bitmaps[ASSET_COW_CIRCLE_OUTLINE]= load_and_init_bitmap("../assets/bitmaps/cow1/circleoutline.png");
    cow.bitmaps[ASSET_COW_STRAIGHT]= load_and_init_bitmap("../assets/bitmaps/cow1/straight.png");
    cow.bitmaps[ASSET_COW_STRAIGHT_OUTLINE]= load_and_init_bitmap("../assets/bitmaps/cow1/straightoutline.png");
    
    // init opengl vars
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);
    glPatchParameteri(GL_PATCH_VERTICES, 4);
    
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glViewport(0, 0, window_dim.width, window_dim.height);
    
    //glEnable(GL_DEBUG_OUTPUT);
    //glDebugMessageCallback(opengl_debug_message_callback, 0);
    
    while(1)
    {
        // input()
        for (u32 i = 0; i < ARRAY_COUNT(controller.buttons); i++)
            controller.buttons[i].previous_state = controller.buttons[i].current_state;
        
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
                    SDL_KeyboardEvent *keyboard_event = &event.key;
                    s32 key_id = keyboard_event->keysym.sym;
                    b32 state = false;
                    if (keyboard_event->state == SDL_PRESSED)
                        state = true;
                    
                    for (u32 i = 0; i < ARRAY_COUNT(controller.buttons); i++)
                    {
                        if (key_id == controller.buttons[i].id)
                        {
                            controller.buttons[i].current_state = state;
                            break;
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
        
        // update()
        if (game_mode == MAIN_MENU)
        {
            menu_update_active(&active, 0, 1, controller.down, controller.up);
        }
        if (game_mode == IN_GAME)
        {
            Coffee_Cow_Node *head = &cow.nodes[0];
            if (cow.num_of_inputs != 4)
            {
                if (on_down(controller.right) && head->direction != LEFT_V)
                    cow.inputs[cow.num_of_inputs++] = RIGHT_V; //cow.direction = RIGHT_V;
                if (on_down(controller.up) && head->direction != DOWN_V)
                    cow.inputs[cow.num_of_inputs++] = UP_V; //cow.direction = UP_V;
                if (on_down(controller.left) && head->direction != RIGHT_V)
                    cow.inputs[cow.num_of_inputs++] = LEFT_V; //cow.direction = LEFT_V;
                if (on_down(controller.down) && head->direction != UP_V)
                    cow.inputs[cow.num_of_inputs++] = DOWN_V; //cow.direction = DOWN_V;
            }
            update_coffee_cow(&cow, frame_time_s, grid_dim);
        }
        
        // draw()
        u32 gl_clear_flags = GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT;
        glClear(gl_clear_flags);
        orthographic_matrix = orthographic_projection(0.0f, (r32)window_dim.width, (r32)window_dim.height,
                                                      0.0f, -3.0f, 3.0f);
        global_window_dim = window_dim;
        
        Menu main_menu = {};
        v2 menu_logo_dim = {550, 274};
        main_menu.padding = {10, 10};
        main_menu.font = &rubik;
        
        main_menu.button.dim = {550, 75};
        main_menu.button.back_color = {0, 0, 0, 1};
        main_menu.button.active_back_color = {222, 201, 179, 1};
        main_menu.button.text_color = {255, 255, 255, 1};
        main_menu.button.active_text_color = {0, 0, 0, 1};
        main_menu.button.pixel_height = 50;
        if (0)
        {
            //draw_rect( { 0, 0 }, 0, { 100, 100 }, { 255, 0, 0, 1 } );
            //Rect test_rect = get_centered_coords(0.5f, cv2(window_dim));
            //draw_rect( test_rect, &test );
            //draw_rect( { (r32)window_dim.x/2.0f, (r32)window_dim.y/2.0f }, 90 * DEG2RAD, {100, 100}, &test );
            //draw_string(&rubik, "yooo", { 200, 200 }, 50, { 0, 255, 0, 1 });
        }
        else if (game_mode == MAIN_MENU)
        {
            main_menu.dim.x = menu_logo_dim.x;
            main_menu.dim.y = menu_logo_dim.y + (main_menu.button.dim.y * 2.0f) + (main_menu.padding.y * 2.0f);
            v2 menu_coords = get_center_menu_coords(main_menu.dim, cv2(window_dim));
            
            draw_rect({0, 0}, 0, cv2(window_dim), &main_menu_back);
            
            draw_rect(menu_coords, 0, menu_logo_dim, &logo);
            
            menu_coords.y += menu_logo_dim.y + main_menu.padding.y;
            
            if (menu_button(&main_menu, menu_coords, "Play", 0, active, on_down(controller.select)))
            {
                game_mode = IN_GAME;
                active = 0;
            }
            
            menu_coords.y += main_menu.button.dim.y + main_menu.padding.y;
            
            if (menu_button(&main_menu, menu_coords, "Quit", 1, active, on_down(controller.select)))
            {
                return 0;
            }
        }
        else if (game_mode == IN_GAME)
        {
            draw_rect({ 0, 0 }, 0, cv2(window_dim), &game_back);
            
            Rect rocks_rect = get_centered_coords(0.9f, cv2(window_dim));
            Rect grass_rect = get_centered_coords(0.75265f, rocks_rect.dim);
            grass_rect.coords += rocks_rect.coords;
            draw_rect(grass_rect, &grass);
            
            v2 grid_size = grass_rect.dim / cv2(grid_dim);
            
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
            
            main_menu.dim.x = menu_logo_dim.x;
            main_menu.dim.y = (main_menu.button.dim.y * 1.0f);
            v2 menu_coords = get_center_menu_coords(main_menu.dim, cv2(window_dim));
            
            if (menu_button(&main_menu, menu_coords, "Go back", 0, active, on_down(controller.select)))
            {
                game_mode = MAIN_MENU;
            }
        }
        
        SDL_GL_SwapWindow(window);
    }
    
    return 0;
}

int main(int argc, char *argv[]) { return game(); }
