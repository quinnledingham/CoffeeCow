#ifndef ASSETS_H
#define ASSETS_H

struct File
{
    u32 size;
    void *memory;
};

struct Bitmap
{
    u8 *memory;
    v2s dim;
    s32 pitch;
    s32 channels;

    u32 handle; // opengl handle
};

struct Shader
{
    const char *vs_filename;  //.vs vertex_shader
    const char *tcs_filename; //.tcs tessellation control shader
    const char *tes_filename; //.tes tessellation evaluation shader
    const char *gs_filename;  //.gs geometry shader
    const char *fs_filename;  //.fs fragment shader
    
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

struct Audio
{
    SDL_AudioSpec spec;
    u8 *buffer;
    u32 length;
};

struct Playing_Audio
{
    u8 *position;
    u32 length_remaining;
};

struct Audio_Player
{
    b32 playing;
    Playing_Audio audios[10];
    u32 audios_count;

    u8 *buffer;
    u32 length;

    SDL_AudioStream *audio_stream;
    SDL_AudioDeviceID device_id;
};

//
// Storing Assets
//

enum asset_types
{
    ASSET_TYPE_FONT,
    ASSET_TYPE_BITMAP,
    ASSET_TYPE_SHADER,
    ASSET_TYPE_AUDIO,
};

struct Asset
{
    u32 type;
    const char *tag;
    union
    {
        Font font;
        Bitmap bitmap;
        Shader shader;
        Audio audio;
    };
};

struct Asset_Load_Info
{
    int type;
    int index;
    const char *tag;
    const char *filename;
};

struct Assets
{
    u32 num_of_assets;
    
    Asset_Load_Info *info;
    u32 num_of_info_loaded;
    
    // Storage of assets
    Asset *fonts;
    u32 num_of_fonts;
    
    Asset *bitmaps;
    u32 num_of_bitmaps;
    
    Asset *shaders;
    u32 num_of_shaders;

    Asset *audios;
    u32 num_of_audios;
};

function void
add_asset(void *data, void *args)
{
    Assets *assets = (Assets*)data;
    Asset_Load_Info *info = (Asset_Load_Info*)args;
    
    if      (info->type == ASSET_TYPE_FONT)   info->index = assets->num_of_fonts;
    else if (info->type == ASSET_TYPE_BITMAP) info->index = assets->num_of_bitmaps;
    else if (info->type == ASSET_TYPE_SHADER) info->index = assets->num_of_shaders;
    else if (info->type == ASSET_TYPE_AUDIO)  info->index = assets->num_of_audios;
    
    if      (info->type == ASSET_TYPE_FONT)   assets->num_of_fonts++;
    else if (info->type == ASSET_TYPE_BITMAP) assets->num_of_bitmaps++;
    else if (info->type == ASSET_TYPE_SHADER) assets->num_of_shaders++;
    else if (info->type == ASSET_TYPE_AUDIO)  assets->num_of_audios++;
    
    assets->info[assets->num_of_info_loaded++] = *info;
}

function void
count_asset(void *data, void *args)
{
    Assets *assets = (Assets*)data;
    assets->num_of_assets++;
}

function Font*
find_font(Assets *assets, const char *tag)
{
    for (u32 i = 0; i < assets->num_of_fonts; i++)
        if (equal(tag, assets->fonts[i].tag)) return &assets->fonts[i].font;
    
    warning(0, "Could not find font with tag: %s", tag);
    
    return 0;
}

function Bitmap*
find_bitmap(Assets *assets, const char *tag)
{
    for (u32 i = 0; i < assets->num_of_bitmaps; i++)
        if (equal(tag, assets->bitmaps[i].tag)) return &assets->bitmaps[i].bitmap;
    
    warning(0, "Could not find bitmap with tag: %s", tag);
    
    return 0;
}

function Audio*
find_audio(Assets *assets, const char *tag)
{
    for (u32 i = 0; i < assets->num_of_audios; i++)
        if (equal(tag, assets->audios[i].tag)) return &assets->audios[i].audio;
    
    warning(0, "Could not find audio with tag: %s", tag);
    
    return 0;
}

//
// Parsing Asset File
//

function bool
is_ascii_digit(int ch)
{
    if (isdigit(ch)) return true;
    return false;
}

function bool
is_ascii_letter(int ch)
{
    if (ch >= 'A' && ch <= 'Z') return true; // uppercase
    else if (ch >= 'a' && ch <= 'z') return true; // lowercase
    else return false;
}

function b32
is_file_path_ch(s32 ch)
{
    if (ch == '.' || ch == '/' || ch == '-' || ch == '_') return true;
    else return false;
}

function b32
is_valid_body_ch(s32 ch)
{
    if (is_ascii_letter(ch) || is_ascii_digit(ch) || is_file_path_ch(ch)) return true;
    else return false;
}

function b32
is_valid_start_ch(s32 ch)
{
    if (is_ascii_letter(ch) || is_file_path_ch(ch)) return true;
    else return false;
}

enum Asset_Token_Type
{
    ATT_KEYWORD,
    ATT_ID,
    ATT_SEPERATOR,
    ATT_ERROR,
    ATT_WARNING,
    ATT_END
};

struct Asset_Token
{
    s32 type;
    const char *lexeme;
};

const char *asset_keywords[4] = { "FONTS", "BITMAPS", "SHADERS", "AUDIOS" };

function b32
is_asset_keyword(const char *word)
{
    for (u32 i = 0; i < ARRAY_COUNT(asset_keywords); i++) 
        if (equal(word, asset_keywords[i])) return true;
    return false;
}

#endif //ASSETS_H
