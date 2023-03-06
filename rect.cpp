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

function void draw_rect(Rect rect, v4 color) { draw_rect(rect.coords, 0.0f, rect.dim, color); }
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

// returns the coords at the center of rect
function v2 get_center(Rect rect) { return rect.coords + (rect.dim / 2.0f); }

// returns the coords of where in would be placed to
// be centered in out
function v2
get_centered(Rect in, Rect out)
{
    return
    { 
        (out.dim.x/2.0f) - (in.dim.x / 2.0f), 
        (out.dim.y/2.0f) - (in.dim.y / 2.0f)
    };
}

function Rect
get_centered_rect(Rect og, r32 x_percent, r32 y_percent)
{
    Rect rect = {};
    rect.dim.x = og.dim.x * x_percent;
    rect.dim.y = og.dim.y * y_percent;
    rect.coords = get_centered(rect, og);
    rect.coords += og.coords;
    return rect;
}

function Rect
get_centered_rect_pad(Rect og, v2 pad)
{
    Rect rect = {};
    rect.dim.x = og.dim.x + pad.x;
    rect.dim.y = og.dim.y + pad.y;
    rect.coords = get_centered(rect, og);
    rect.coords += og.coords;
    return rect;
}

function Rect
get_centered_square(Rect og, r32 percent)
{
    Rect rect = {};
    if (og.dim.x <= og.dim.y)
    {
        rect.dim.x = og.dim.x * percent;
        rect.dim.y = rect.dim.x;
    }
    else
    {
        rect.dim.y = og.dim.y * percent;
        rect.dim.x = rect.dim.y;
    }
    
    //log("%f %f\n", rect.dim.x, rect.dim.y);
    rect.coords = get_centered(rect, og);
    rect.coords += og.coords;
    return rect;
}