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
        SDL_Log("ERROR: Cannot open file %s.\n", filename);
    
    return result;
}

//
// Bitmap
//

function Bitmap
load_bitmap(const char *filename)
{
    Bitmap bitmap = {};
    bitmap.memory = stbi_load(filename, &bitmap.width, &bitmap.height, &bitmap.channels, 0);
    if (bitmap.memory == 0)
        error("load_bitmap() could not load bitmap");
    
    printf("file: %s %d %d %d\n", filename, bitmap.width, bitmap.height, bitmap.channels);
    
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
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, bitmap->width, bitmap->height, 0, GL_RGB, GL_UNSIGNED_BYTE, bitmap->memory);
    }
    else if (bitmap->channels == 4)
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, bitmap->width, bitmap->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, bitmap->memory);
    
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

function bool
compile_shader(u32 handle, const char *filename, int type)
{
    const char *file = load_shader_file(filename);
    if (file == 0)
        return false;
    
    u32 s =  glCreateShader((GLenum)type);
    glShaderSource(s, 1, &file, NULL);
    glCompileShader(s);
    
    GLint compiled_s = 0;
    glGetShaderiv(s, GL_COMPILE_STATUS, &compiled_s);
    if (!compiled_s)
    {
#ifdef opengl_debug
        opengl_debug(GL_SHADER, s);
#endif
    }
    else
        glAttachShader(handle, s);
    
    glDeleteShader(s);
    free((void*)file);
    
    return compiled_s;
}

function void
load_shader(Shader *shader)
{
    shader->compiled = false;
    if (shader->handle != 0)
        glDeleteProgram(shader->handle);
    shader->handle = glCreateProgram();
    
    // Compile
    if (!compile_shader(shader->handle, shader->vs_filename, GL_VERTEX_SHADER))
    {
        printf("vs: %s\n", shader->vs_filename);
        error("load_opengl_shader() compiling vertex shader failed");
        return;
    }
    if (shader->tcs_filename != 0)
    {
        if (!compile_shader(shader->handle, shader->tcs_filename, GL_TESS_CONTROL_SHADER))
            error("load_opengl_shader() compiling tess control shader failed");
    }
    if (shader->tes_filename != 0)
    {
        if (!compile_shader(shader->handle, shader->tes_filename, GL_TESS_EVALUATION_SHADER))
            error("load_opengl_shader() compiling tess_evaluation shader failed");
    }
    if (shader->gs_filename != 0)
    {
        if (!compile_shader(shader->handle, shader->gs_filename, GL_GEOMETRY_SHADER))
            error("load_opengl_shader() compiling geometry shader failed");
    }
    if (shader->fs_filename != 0)
    {
        if (!compile_shader(shader->handle, shader->fs_filename, GL_FRAGMENT_SHADER))
            error("load_opengl_shader() compiling fragment shader failed");
    }
    
    // Link
    glLinkProgram(shader->handle);
    GLint linked_program = 0;
    glGetProgramiv(shader->handle, GL_LINK_STATUS, &linked_program);
    if (!linked_program)
    {
#ifdef opengl_debug
        opengl_debug(GL_PROGRAM, shader->handle);
#endif
        error("load_opengl_shader() link failed");
        return;
    }
    
    shader->compiled = true;
}

function Shader
load_shader(const char *vs_filename,
            const char *tcs_filename,
            const char *tes_filename,
            const char *gs_filename,
            const char *fs_filename)
{
    Shader shader = {};
    
    shader.vs_filename = copy(vs_filename);
    shader.tcs_filename = copy(tcs_filename);
    shader.tes_filename = copy(tes_filename);
    shader.gs_filename = copy(gs_filename);
    shader.fs_filename = copy(fs_filename);
    
    load_shader(&shader);
    
    return shader;
}

function void
reload_shader(Shader *shader)
{
    load_shader(shader);
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
create_rect_mesh(Mesh *rect)
{
    rect->vertices_count = 4;
    rect->vertices = (Vertex*)SDL_malloc(sizeof(Vertex) * rect->vertices_count);
    rect->vertices[0] = { {0, 0, 0}, {0, 0, 1}, {0, 0} };
    rect->vertices[1] = { {0, 1, 0}, {0, 0, 1}, {0, 1} };
    rect->vertices[2] = { {1, 0, 0}, {0, 0, 1}, {1, 0} };
    rect->vertices[3] = { {1, 1, 0}, {0, 0, 1}, {1, 1} };
    
    rect->indices_count = 6;
    rect->indices = (u32*)SDL_malloc(sizeof(u32) * rect->indices_count);
    init_rect_indices(rect->indices, 1, 3, 0, 2);
    
    init_mesh(rect);
}

function void
draw_rect(v3 coords, quat rotation, v3 dim, u32 handle,
          Mesh *rect, Shader *shader, m4x4 projection_matrix, m4x4 view_matrix)
{
    m4x4 model = create_transform_m4x4(coords, rotation, dim);
    glUniformMatrix4fv(glGetUniformLocation(handle, "model"), (GLsizei)1, false, (float*)&model);
    glUniformMatrix4fv(glGetUniformLocation(handle, "projection"), (GLsizei)1, false, (float*)&projection_matrix);
    glUniformMatrix4fv(glGetUniformLocation(handle, "view"), (GLsizei)1, false, (float*)&view_matrix);
    
    //local_persist Mesh rect = {};
    
    draw_mesh(rect);
}

function void
draw_rect(v3 coords, quat rotation, v3 dim, v4 color,
          Mesh *rect, Shader *shader, m4x4 projection_matrix, m4x4 view_matrix)
{
    u32 handle = use_shader(shader);
    glUniform4fv(glGetUniformLocation(handle, "user_color"), (GLsizei)1, (float*)&color);
    draw_rect(coords, rotation, dim, handle, rect, shader, projection_matrix, view_matrix);
}

function void
draw_rect(v3 coords, quat rotation, v3 dim, Bitmap *bitmap,
          Mesh *rect, Shader *shader, m4x4 projection_matrix, m4x4 view_matrix)
{
    u32 handle = use_shader(shader);
    
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, bitmap->handle);
    glUniform1i(glGetUniformLocation(handle, "tex0"), 0);
    
    draw_rect(coords, rotation, dim, handle, rect, shader, projection_matrix, view_matrix);
}

//
// Font
//

function void
log_font_chars(Font *font)
{
    printf("start log\n");
    for (u32 i = 0; i < font->font_chars_cached; i++)
    {
        Font_Char *fc = &font->font_chars[i];
        printf("%d, %f, %f, %f, %f, %f, %d, %d\n", fc->codepoint, fc->scale, fc->color.x, fc->color.y,
               fc->color.z, fc->color.w, fc->ax, fc->lsb);
    }
    printf("end log\n");
}

function Font_Char*
load_font_char(Font *font, u32 codepoint, f32 scale, v4 color)
{
    // search cache for font char
    for (u32 i = 0; i < font->font_chars_cached; i++)
    {
        Font_Char *font_char = &font->font_chars[i];
        //printf("wow: %d\n", codepoint);
        if (font_char->codepoint == codepoint && font_char->color == color && font_char->scale == scale)
            return font_char;
    }
    
    // where to cache new font char
    Font_Char *font_char = &font->font_chars[font->font_chars_cached];
    memset(font_char, 0, sizeof(Font_Char));
    if (font->font_chars_cached + 1 < array_count(font->font_chars))
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
                                               &font_char->bitmap.width, &font_char->bitmap.height,
                                               0, 0);
    font_char->bitmap.channels = 4;
    font_char->bitmap.memory = (u8*)SDL_malloc(font_char->bitmap.width * 
                                               font_char->bitmap.height * 
                                               font_char->bitmap.channels);
    u32 *dest = (u32*)font_char->bitmap.memory;
    
    for (s32 x = 0; x < font_char->bitmap.width; x++)
    {
        for (s32 y = 0; y < font_char->bitmap.height; y++)
        {
            u8 alpha = *mono_bitmap++;
            *dest++ = ((alpha << 24) | (alpha << 16) | (alpha <<  8) | (alpha <<  0));
        }
    }
    
    init_bitmap_handle(&font_char->bitmap);
    
    return font_char;
}

function Font
load_font(const char *filename)
{
    Font font = {};
    SDL_memset(font.font_scales, 0, sizeof(Font_Scale) * array_count(font.font_scales));
    SDL_memset(font.font_chars, 0, sizeof(Font_Char) * array_count(font.font_chars));
    SDL_memset(font.font_strings, 0, sizeof(Font_String) * array_count(font.font_strings));
    File file = read_file(filename);
    stbtt_InitFont(&font.info, (u8*)file.memory, stbtt_GetFontOffsetForIndex((u8*)file.memory, 0));
    return font;
}

function void
draw_string(Font *font, const char *string, v2 coords, f32 pixel_height, v4 color,
            Mesh *rect, Shader *shader, m4x4 projection_matrix, m4x4 view_matrix)
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
        
        draw_rect({x, y, 0}, get_rotation(0, {1, 0, 0}), {dim.x, dim.y}, &font_char->bitmap,
                  rect, shader, projection_matrix, view_matrix);
        
        //printf("char: %c\n", font_char->codepoint);
        int kern = stbtt_GetCodepointKernAdvance(&font->info, string[i], string[i + 1]);
        string_x_coord += ((kern + font_char->ax) * scale);
        
        i++;
    }
}

function v2
get_string_dim(Font *font, const char *in, f32 pixel_height, v4 color)
{
    Font_String *string = 0;
    for (u32 i = 0; i < font->strings_cached; i++)
    {
        string = &font->font_strings[i];
        if (equal(string->memory, in) && string->pixel_height == pixel_height)
            return string->dim;
    }
    
    string = &font->font_strings[font->strings_cached];
    if (font->strings_cached + 1 < array_count(font->font_strings))
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
        f32 x = string_x_coord + (font_char->lsb * scale);
        
        if (y > string_height)
            string_height = y;
        
        int kern = stbtt_GetCodepointKernAdvance(&font->info, string->memory[i], string->memory[i + 1]);
        string_x_coord += ((kern + font_char->ax) * scale);
        
        i++;
    }
    string->memory[i] = 0;
    
    string->pixel_height = pixel_height;
    string->dim = { string_x_coord, string_height };
    
    return string->dim;
}
