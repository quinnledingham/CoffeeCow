//
// File
//

function File
read_file(const char *filename)
{
    File result = {};
    
    FILE *in = fopen(filename, "rb");
    if(in) 
    {
        fseek(in, 0, SEEK_END);
        result.size = ftell(in);
        fseek(in, 0, SEEK_SET);
        
        result.memory = malloc(result.size);
        fread(result.memory, result.size, 1, in);
        fclose(in);
    }
    else 
        log("ERROR: Cannot open file %s.\n", filename);
    
    return result;
}

//
// Bitmap
//

function Bitmap
load_bitmap(const char *filename)
{
    Bitmap bitmap = {};
    bitmap.memory = stbi_load(filename, &bitmap.dim.width, &bitmap.dim.height, &bitmap.channels, 0);
    if (bitmap.memory == 0) error("load_bitmap() could not load bitmap");
    //log("file: %s %d %d %d\n", filename, bitmap.dim.width, bitmap.dim.height, bitmap.channels);
    return bitmap;
}

function void
init_bitmap_handle(Bitmap *bitmap)
{
    glGenTextures(1, &bitmap->handle);
    glBindTexture(GL_TEXTURE_2D, bitmap->handle);
    
    if (bitmap->channels == 3)
    {
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, bitmap->dim.width, bitmap->dim.height, 0, GL_RGB, GL_UNSIGNED_BYTE, bitmap->memory);
    }
    else if (bitmap->channels == 4)
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, bitmap->dim.width, bitmap->dim.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, bitmap->memory);
    
    glGenerateMipmap(GL_TEXTURE_2D);
    
    // Tile
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
    glBindTexture(GL_TEXTURE_2D, 0);
}

function Bitmap
load_and_init_bitmap(const char *filename)
{
    Bitmap bitmap = load_bitmap(filename);
    init_bitmap_handle(&bitmap);
    return bitmap;
}

//
// Shader
//

function const char*
load_shader_file(const char* filename)
{
    FILE *file = fopen(filename, "rb");
    if (file == 0)
    {
        error("load_shader_file() could not open file");
        return 0;
    }
    
    fseek(file, 0, SEEK_END);
    u32 size = ftell(file);
    fseek(file, 0, SEEK_SET);
    
    char* shader_file = (char*)malloc(size + 1);
    fread(shader_file, size, 1, file);
    shader_file[size] = 0;
    fclose(file);
    
    return shader_file;
}

function void
load_shader(Shader *shader)
{
    // Load files
    if (shader->vs_filename != 0)
    {
        if (shader->vs_file != 0)
            free((void*)shader->vs_file);
        shader->vs_file = load_shader_file(shader->vs_filename);
    }
    else
    {
        error("load_opengl_shader() must have a vertex shader");
        return;
    }
    
    if (shader->tcs_filename != 0)
    {
        if (shader->tcs_file != 0)
            free((void*)shader->tcs_file);
        shader->tcs_file = load_shader_file(shader->tcs_filename);
    }
    if (shader->tes_filename != 0)
    {
        if (shader->tes_file != 0)
            free((void*)shader->tes_file);
        shader->tes_file = load_shader_file(shader->tes_filename);
    }
    if (shader->gs_filename != 0)
    {
        if (shader->gs_file != 0)
            free((void*)shader->gs_file);
        shader->gs_file = load_shader_file(shader->gs_filename);
    }
    if (shader->fs_filename != 0)
    {
        if (shader->fs_file != 0)
            free((void*)shader->fs_file);
        shader->fs_file = load_shader_file(shader->fs_filename);
    }
}

function bool
compile_shader(u32 handle, const char *file, int type)
{
    u32 s =  glCreateShader((GLenum)type);
    glShaderSource(s, 1, &file, NULL);
    glCompileShader(s);
    
    GLint compiled_s = 0;
    glGetShaderiv(s, GL_COMPILE_STATUS, &compiled_s);
    if (!compiled_s)
    {
        opengl_debug(GL_SHADER, s);
        error("compile_shader() could not compile %s", glGetString(type));
    }
    else
        glAttachShader(handle, s);
    
    glDeleteShader(s);
    
    return compiled_s;
}

function void
compile_shader(Shader *shader)
{
    shader->compiled = false;
    if (shader->handle != 0)
        glDeleteProgram(shader->handle);
    shader->handle = glCreateProgram();
    
    if (shader->vs_file != 0)
        compile_shader(shader->handle, shader->vs_file, GL_VERTEX_SHADER);
    if (shader->tcs_file != 0)
        compile_shader(shader->handle, shader->tcs_file, GL_TESS_CONTROL_SHADER);
    if (shader->tes_file != 0)
        compile_shader(shader->handle, shader->tes_file, GL_TESS_EVALUATION_SHADER);
    if (shader->gs_file != 0)
        compile_shader(shader->handle, shader->gs_file, GL_GEOMETRY_SHADER);
    if (shader->fs_file != 0)
        compile_shader(shader->handle, shader->fs_file,GL_FRAGMENT_SHADER);
    
    // Link
    glLinkProgram(shader->handle);
    GLint linked_program = 0;
    glGetProgramiv(shader->handle, GL_LINK_STATUS, &linked_program);
    if (!linked_program)
    {
        opengl_debug(GL_PROGRAM, shader->handle);
        error("load_opengl_shader() link failed");
        return;
    }
    
    shader->compiled = true;
    
}

function u32
use_shader(Shader *shader)
{
    glUseProgram(shader->handle);
    return shader->handle;
}

//
// Mesh
//

function void
init_mesh(Mesh *mesh)
{
    glGenVertexArrays(1, &mesh->vao);
    glGenBuffers(1, &mesh->vbo);
    glGenBuffers(1, &mesh->ebo);
    
    glBindVertexArray(mesh->vao);
    glBindBuffer(GL_ARRAY_BUFFER, mesh->vbo);
    glBufferData(GL_ARRAY_BUFFER, mesh->vertices_count * sizeof(Vertex), &mesh->vertices[0], GL_STATIC_DRAW);  
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh->indices_count * sizeof(u32), &mesh->indices[0], GL_STATIC_DRAW);
    
    glEnableVertexAttribArray(0); // vertex positions
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
    glEnableVertexAttribArray(1); // vertex normals
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
    glEnableVertexAttribArray(2); // vertex texture coords
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texture_coordinate));
    
    glBindVertexArray(0);
}

function void
draw_mesh(Mesh *mesh)
{
    glBindVertexArray(mesh->vao);
    glDrawElements(GL_TRIANGLES, mesh->indices_count, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

//
// Assets
//

function Font
load_font(const char *filename)
{
    Font font = {};
    SDL_memset(font.font_scales, 0, sizeof(Font_Scale) * ARRAY_COUNT(font.font_scales));
    SDL_memset(font.font_chars, 0, sizeof(Font_Char) * ARRAY_COUNT(font.font_chars));
    SDL_memset(font.font_strings, 0, sizeof(Font_String) * ARRAY_COUNT(font.font_strings));
    File file = read_file(filename);
    stbtt_InitFont(&font.info, (u8*)file.memory, stbtt_GetFontOffsetForIndex((u8*)file.memory, 0));
    return font;
}

function Font_Char*
load_font_char(Font *font, u32 codepoint, f32 scale, v4 color)
{
    // search cache for font char
    for (s32 i = 0; i < font->font_chars_cached; i++)
    {
        Font_Char *font_char = &font->font_chars[i];
        if (font_char->codepoint == codepoint && font_char->color == color && font_char->scale == scale)
            return font_char;
    }
    
    // where to cache new font char
    Font_Char *font_char = &font->font_chars[font->font_chars_cached];
    memset(font_char, 0, sizeof(Font_Char));
    if (font->font_chars_cached + 1 < (s32)ARRAY_COUNT(font->font_chars))
        font->font_chars_cached++;
    else
        font->font_chars_cached = 0;
    
    font_char->codepoint = codepoint;
    font_char->scale = scale;
    font_char->color = color;
    
    // how wide is this character
    stbtt_GetCodepointHMetrics(&font->info, font_char->codepoint, &font_char->ax, &font_char->lsb);
    
    // get bounding box for character (may be offset to account for chars that dip above or below the line
    stbtt_GetCodepointBitmapBox(&font->info, font_char->codepoint, font_char->scale, font_char->scale, 
                                &font_char->c_x1, &font_char->c_y1, &font_char->c_x2, &font_char->c_y2);
    
    u8 *mono_bitmap = stbtt_GetCodepointBitmap(&font->info, 0, scale, codepoint, 
                                               &font_char->bitmap.dim.width, &font_char->bitmap.dim.height,
                                               0, 0);
    font_char->bitmap.channels = 4;
    font_char->bitmap.memory = (u8*)SDL_malloc(font_char->bitmap.dim.width * 
                                               font_char->bitmap.dim.height * 
                                               font_char->bitmap.channels);
    u32 *dest = (u32*)font_char->bitmap.memory;
    for (s32 x = 0; x < font_char->bitmap.dim.width; x++)
    {
        for (s32 y = 0; y < font_char->bitmap.dim.height; y++)
        {
            u8 alpha = *mono_bitmap++;
            u32 real_alpha = u32((f32)alpha * color.a);
            *dest++ = ((real_alpha << 24) | ((u32)color.r << 16) | ((u32)color.g <<  8) | ((u32)color.b <<  0));
        }
    }
    
    //free(mono_bitmap);
    
    init_bitmap_handle(&font_char->bitmap);
    
    return font_char;
}

function v2
get_string_dim(Font *font, const char *in, f32 pixel_height, v4 color)
{
    Font_String *string = 0;
    for (s32 i = 0; i < font->strings_cached; i++)
    {
        string = &font->font_strings[i];
        if (equal(string->memory, in) && string->pixel_height == pixel_height && string->color == color)
            return string->dim;
    }
    
    string = &font->font_strings[font->strings_cached];
    if (font->strings_cached + 1 < (s32)ARRAY_COUNT(font->font_strings))
        font->strings_cached++;
    else
        font->strings_cached = 0;
    
    f32 scale = stbtt_ScaleForPixelHeight(&font->info, pixel_height);
    string->memory = (char*)SDL_malloc(get_length(in) + 1);
    SDL_memset(string->memory, 0, get_length(in) + 1);
    
    f32 string_x_coord = 0.0f;
    f32 string_height = 0.0f;
    
    int i = 0;
    while (in[i] != 0)
    {
        string->memory[i] = in[i];
        Font_Char *font_char = load_font_char(font, string->memory[i], scale, color);
        
        f32 y = -1 * font_char->c_y1;
        //f32 x = string_x_coord + (font_char->lsb * scale);
        
        if (y > string_height)
            string_height = y;
        
        int kern = stbtt_GetCodepointKernAdvance(&font->info, string->memory[i], string->memory[i + 1]);
        string_x_coord += ((kern + font_char->ax) * scale);
        
        i++;
    }
    string->memory[i] = 0;
    
    string->pixel_height = pixel_height;
    string->dim = { string_x_coord, string_height };
    string->color = color;
    
    return string->dim;
}
