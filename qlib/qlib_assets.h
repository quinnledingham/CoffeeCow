#ifndef QLIB_ASSETS_H
#define QLIB_ASSETS_H

struct File
{
    u32 size;
    void *memory;
};

struct Bitmap
{
    u32 handle;
    u8 *memory;
    s32 width;
    s32 height;
    s32 pitch;
    s32 channels;
};

struct Sound
{
    SDL_AudioSpec spec;
    u8 *buffer;
    u32 length;
};

struct Playing_Sound
{
    u8 *position;
    u32 length_remaining;
    SDL_AudioDeviceID device_id;
};

struct Audio
{
    b32 playing;
    
    Playing_Sound sounds[10];
    u32 num_of_playing_sounds;
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

inline u32
use_shader(Shader *shader)
{
    glUseProgram(shader->handle);
    return shader->handle;
}

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

struct Assets
{
    Shader *shaders;
    u32 num_of_shaders;
    
    Sound *sounds;
    u32 num_of_sounds;
    
    Bitmap *bitmaps;
    u32 num_of_bitmaps;
    
    Font *fonts;
    u32 num_of_fonts;
};

#endif //QLIB_ASSETS_H
