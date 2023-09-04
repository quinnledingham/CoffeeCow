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
        
        result.memory = SDL_malloc(result.size);
        fread(result.memory, result.size, 1, in);
        fclose(in);
    }
    else error(0, "Cannot open file %s", filename);
    
    return result;
}

// copies the next n = length of chars from the input file
// and returns them in a string
function const char*
copy_from_file(FILE *input_file, int length)
{
    char *string = (char*)SDL_malloc(length + 1);
    memset(string, 0, length + 1);

    for (int i = 0; i < length; i++)
    {
        int ch = fgetc(input_file);
        if (ch == EOF)
        {
            warning(0, "copy_from_file hit the EOF");
            break;
        }
        string[i] = ch;
    }
    
    return string;
}


//
// Bitmap
//

function Bitmap
load_bitmap(const char *filename)
{
    Bitmap bitmap = {};
    bitmap.memory = stbi_load(filename, &bitmap.dim.width, &bitmap.dim.height, &bitmap.channels, 0);
    if (bitmap.memory == 0) error("load_bitmap() could not load bitmap %s", filename);
    //log("file: %s %d %d %d\n", filename, bitmap.dim.width, bitmap.dim.height, bitmap.channels);
    bitmap.pitch = bitmap.dim.width * bitmap.channels;
    return bitmap;
}

function void
init_bitmap_handle(Bitmap *bitmap)
{
    GLenum target = GL_TEXTURE_2D;
    
    glGenTextures(1, &bitmap->handle);
    glBindTexture(target, bitmap->handle);

    GLint internal_format;
    GLenum data_format;
    GLint pixel_unpack_alignment;
    
    switch(bitmap->channels)
    {
        case 3:
            internal_format = GL_RGB;
            data_format = GL_RGB;
            pixel_unpack_alignment = 1;
        break;
        
        case 4:
            internal_format = GL_RGBA;
            data_format = GL_RGBA;
            pixel_unpack_alignment = 0;
        break;
    }
    
    glPixelStorei(GL_UNPACK_ALIGNMENT, pixel_unpack_alignment);
    glTexImage2D(target, 0, internal_format, bitmap->dim.width, bitmap->dim.height, 0, data_format, GL_UNSIGNED_BYTE, bitmap->memory);
    glGenerateMipmap(target);
    
    // Tile
    //glTexParameteri(target, GL_TEXTURE_WRAP_S, GL_REPEAT);
    //glTexParameteri(target, GL_TEXTURE_WRAP_T, GL_REPEAT);
    
    glTexParameteri(target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(target, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    
    glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
    glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    
    glBindTexture(target, 0);
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
    if (shader->vs_filename == 0)
    {
        error("load_opengl_shader() must have a vertex shader");
        return;
    }
    
    // Free all files
    if (shader->vs_file  != 0) free((void*)shader->vs_file);
    if (shader->tcs_file != 0) free((void*)shader->tcs_file);
    if (shader->tes_file != 0) free((void*)shader->tes_file);
    if (shader->gs_file  != 0) free((void*)shader->gs_file);
    if (shader->fs_file  != 0) free((void*)shader->fs_file);
    
    // Load files
    if (shader->vs_filename  != 0) shader->vs_file  = load_shader_file(shader->vs_filename);
    if (shader->tcs_filename != 0) shader->tcs_file = load_shader_file(shader->tcs_filename);
    if (shader->tes_filename != 0) shader->tes_file = load_shader_file(shader->tes_filename);
    if (shader->gs_filename  != 0) shader->gs_file  = load_shader_file(shader->gs_filename);
    if (shader->fs_filename  != 0) shader->fs_file  = load_shader_file(shader->fs_filename);
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
    {
        glAttachShader(handle, s);
    }
    
    glDeleteShader(s);
    
    return compiled_s;
}

function void
compile_shader(Shader *shader)
{
    shader->compiled = false;
    if (shader->handle != 0) glDeleteProgram(shader->handle);
    shader->handle = glCreateProgram();
    
    if (shader->vs_file == 0) error("vertex shader required");

    if (shader->vs_file  != 0) compile_shader(shader->handle, shader->vs_file,  GL_VERTEX_SHADER);
    if (shader->tcs_file != 0) compile_shader(shader->handle, shader->tcs_file, GL_TESS_CONTROL_SHADER);
    if (shader->tes_file != 0) compile_shader(shader->handle, shader->tes_file, GL_TESS_EVALUATION_SHADER);
    if (shader->gs_file  != 0) compile_shader(shader->handle, shader->gs_file,  GL_GEOMETRY_SHADER);
    if (shader->fs_file  != 0) compile_shader(shader->handle, shader->fs_file,  GL_FRAGMENT_SHADER);
    
    // Link
    glLinkProgram(shader->handle);
    GLint linked_program = 0;
    glGetProgramiv(shader->handle, GL_LINK_STATUS, &linked_program);
    if (!linked_program)
    {
        opengl_debug(GL_PROGRAM, shader->handle);
        error("compile_shader() link failed");
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

function void
draw_mesh_instanced(Mesh *mesh)
{
    glBindVertexArray(mesh->vao);
    glDrawElementsInstanced(GL_TRIANGLES, mesh->indices_count, GL_UNSIGNED_INT, 0, 10);
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
get_string_dim(Font *font, const char *string, f32 pixel_height, v4 color)
{
    v2 dim = {};
    f32 scale = stbtt_ScaleForPixelHeight(&font->info, pixel_height);
    
    u32 i = 0;
    while (string[i] != 0)
    {
        Font_Char *font_char = load_font_char(font, string[i], scale, color);
        
        f32 y = -1 * font_char->c_y1;
        if (dim.y < y) dim.y = y;
        
        int kern = stbtt_GetCodepointKernAdvance(&font->info, string[i], string[i + 1]);
        dim.x += ((kern + font_char->ax) * scale);
        
        i++;
    }
    
    return dim;
}

//
// Asset File Reading
//

function Asset_Token
scan_asset_file(FILE *file, s32 *line_num, Asset_Token last_token)
{
    X:
    
    s32 ch;
    while((ch = fgetc(file)) != EOF && (ch == 32 || ch == 9 || ch == 13)); // remove whitespace
    
    switch(ch)
    {
        case EOF:
        {
            return { -1, 0 };
        } break;
        
        case '\n':
        {
            (*line_num)++;
            goto X;
        } break;
        
        case ':':
        case ',':
        {
            return { ATT_SEPERATOR, chtos(1, ch) };
        } break;
        
        default:
        {
            if (is_valid_start_ch(ch)) // must start with valid start ch
            {
                int length = 0;
                do
                {
                    ch = fgetc(file);
                    length++;
                } while(is_valid_body_ch(ch));
                ungetc(ch, file);
                
                fseek(file, -length, SEEK_CUR);
                const char *sequence = copy_from_file(file, length);
                
                if (is_asset_keyword(sequence))
                    return { ATT_KEYWORD, sequence };
                
                return { ATT_ID, sequence };
            }
            
            error(*line_num, "not a valid ch");
        } break;
    }
    
    return { ERROR, 0 };
}

// action is what happens when all the parts of an asset are found
function void
parse_asset_file(Assets *assets, FILE *file, void (action)(void *data, void *args))
{
    int type = 0;
    const char *tag;
    const char *filename;
    
    Asset_Token last_token = {};
    Asset_Token tok = {};
    s32 line_num = 1;
    while (tok.type != -1)
    {
        last_token = tok;
        tok = scan_asset_file(file, &line_num, tok);
        //printf("%d, %s\n", tok.type, tok.lexeme);
        
        if (tok.type == ATT_KEYWORD)
        {
            if (equal(tok.lexeme, "FONTS")) type = ASSET_TYPE_FONT;
            else if (equal(tok.lexeme, "BITMAPS")) type = ASSET_TYPE_BITMAP;
            else if (equal(tok.lexeme, "SHADERS")) type = ASSET_TYPE_SHADER;
            
            tok = scan_asset_file(file, &line_num, tok);
            if (!equal(tok.lexeme, ":")) 
            {
                error(line_num, "expected ':'");
                break;
            }
        }
        else if (tok.type == ATT_ID)
        {
            if (!equal(last_token.lexeme, ","))
            {
                tag = tok.lexeme;
                
                tok = scan_asset_file(file, &line_num, tok);
                if (!equal(tok.lexeme, ",")) 
                {
                    error(line_num, "expected ','");
                    break;
                }
            }
            else
            {
                filename = tok.lexeme;
                
                Asset_Load_Info info = { type, 0,  tag, filename };
                action((void*)assets, (void*)&info);
            }
        }
        else if (tok.type == ATT_SEPERATOR)
        {
            error(line_num, "unexpected seperator");
        }
    }
}

function void
load_assets(Assets *assets, const char *filename)
{
    FILE *file = fopen(filename, "r");
    parse_asset_file(assets, file, count_asset);
    assets->info = ARRAY_MALLOC(Asset_Load_Info, assets->num_of_assets);
    fseek(file, 0, SEEK_SET);
    parse_asset_file(assets, file, add_asset);
    fclose(file);
    
    assets->fonts = ARRAY_MALLOC(Asset, assets->num_of_fonts);
    assets->bitmaps = ARRAY_MALLOC(Asset, assets->num_of_bitmaps);
    assets->shaders = ARRAY_MALLOC(Asset, assets->num_of_shaders);
    
    for (u32 i = 0; i < assets->num_of_assets; i++)
    {
        Asset_Load_Info *info = &assets->info[i];
        //printf("asset: %d, %s, %s\n", info->type, info->tag, info->filename);
        
        Asset asset = {};
        asset.type = info->type;
        asset.tag = info->tag;
        
        switch(asset.type)
        {
            case ASSET_TYPE_FONT: 
            {
                asset.font = load_font(info->filename); 
                assets->fonts[info->index] = asset;
            } break;
            
            case ASSET_TYPE_BITMAP: 
            {
                asset.bitmap = load_and_init_bitmap(info->filename); 
                assets->bitmaps[info->index] = asset;
            } break;
        }
    }
}