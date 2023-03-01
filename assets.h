#ifndef ASSETS_H
#define ASSETS_H

struct File
{
    u32 size;
    void *memory;
};

struct Bitmap
{
    u32 handle;
    u8 *memory;
    v2s dim;
    s32 pitch;
    s32 channels;
};

struct Shader
{
    const char *vs_filename; //.vs vertex_shader
    const char *tcs_filename; //.tcs tessellation control shader
    const char *tes_filename; //.tes tessellation evaluation shader
    const char *gs_filename; //.gs geometry shader
    const char *fs_filename; //.fs fragment shader
    
    const char *vs_file;
    const char *tcs_file;
    const char *tes_file;
    const char *gs_file;
    const char *fs_file;
    
    b32 compiled;
    u32 handle;
};
const char *basic_vs = "#version 330 core\n layout (location = 0) in vec3 position; layout (location = 1) in vec3 normal; layout (location = 2) in vec2 texture_coords; uniform mat4 model; uniform mat4 projection; uniform mat4 view; out vec2 uv; void main(void) { gl_Position = projection * view * model * vec4(position, 1.0f); uv = texture_coords;}";
const char *color_fs = "#version 330 core\n in vec2 uv; uniform vec4 user_color; out vec4 FragColor; void main() { FragColor  = vec4(user_color.x/255, user_color.y/255, user_color.z/255, user_color.w);}";
const char *tex_fs = "#version 330 core\n uniform sampler2D tex0; in vec2 uv; out vec4 FragColor; void main() { vec4 tex = texture(tex0, uv); FragColor = tex;}";

struct Vertex
{
    v3 position;
    v3 normal;
    v2 texture_coordinate;
};

struct Mesh
{
    Vertex *vertices;
    u32 vertices_count;
    
    u32 *indices;
    u32 indices_count;
    
    u32 vao;
    u32 vbo;
    u32 ebo;
};

struct Font_Scale
{
    f32 pixel_height;
    
    f32 scale;
    s32 ascent;
    s32 descent;
    s32 line_gap;
    f32 scaled_ascent;
    f32 scaled_descent;
};

struct Font_Char
{
    u32 codepoint;
    f32 scale;
    v4 color;
    
    s32 ax;
    s32 lsb;
    s32 c_x1;
    s32 c_y1;
    s32 c_x2;
    s32 c_y2;
    
    Bitmap bitmap;
};

struct Font_String
{
    char *memory;
    v2 dim;
    f32 pixel_height;
    v4 color;
};

struct Font
{
    stbtt_fontinfo info;
    
    s32 font_scales_cached;
    s32 font_chars_cached;
    s32 strings_cached;
    Font_Scale font_scales[10];
    Font_Char font_chars[300];
    Font_String font_strings[10];
};

#endif //ASSETS_H
