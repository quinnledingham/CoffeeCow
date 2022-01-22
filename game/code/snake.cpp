/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Casey Muratori $
   $Notice: (C) Copyright 2014 by Molly Rocket, Inc. All Rights Reserved. $
   ======================================================================== */

#include "stdio.h"
#include "snake.h"

#include "socketq.cpp"
#include "socketq.h"

#include "gui.cpp"
#include "gui.h"

#include "memorymanager.cpp"
#include "memorymanager.h"

internal void
GameOutputSound(game_sound_output_buffer *SoundBuffer, int ToneHz)
{
    local_persist real32 tSine;
    int16 ToneVolume = 3000;
    int WavePeriod = SoundBuffer->SamplesPerSecond/ToneHz;
    
    int16 *SampleOut = SoundBuffer->Samples;
    for(int SampleIndex = 0;
        SampleIndex < SoundBuffer->SampleCount;
        ++SampleIndex)
    {
        // TODO(casey): Draw this out for people
        real32 SineValue = sinf(tSine);
        int16 SampleValue = (int16)(SineValue * ToneVolume);
        *SampleOut++ = SampleValue;
        *SampleOut++ = SampleValue;
        
        tSine += 2.0f*Pi32*1.0f/(real32)WavePeriod;
    }
}

internal void
RenderWeirdGradient(game_offscreen_buffer *Buffer, int BlueOffset, int GreenOffset)
{
    // TODO(casey): Let's see what the optimizer does
    
    uint8 *Row = (uint8 *)Buffer->Memory;    
    for(int Y = 0;
        Y < Buffer->Height;
        ++Y)
    {
        uint32 *Pixel = (uint32 *)Row;
        for(int X = 0;
            X < Buffer->Width;
            ++X)
        {
            uint8 Blue = (uint8)(X + BlueOffset);
            uint8 Green = (uint8)(Y + GreenOffset);
            
            *Pixel++ = ((Green << 8) | Blue);
        }
        
        Row += Buffer->Pitch;
    }
}

internal void 
RenderRect(game_offscreen_buffer *Buffer, Rect *S, int fill, uint32 color)
{
    uint8 *EndOfBuffer = (uint8 *)Buffer->Memory + Buffer->Pitch*Buffer->Height;
    uint32 Color = color;
    
    for(int X = S->x;
        X < (S->x + S->width);
        ++X)
    {
        uint8 *Pixel = ((uint8 *)Buffer->Memory +
                        X*Buffer->BytesPerPixel +
                        S->y*Buffer->Pitch);
        
        for(int Y = S->y;
            Y < (S->y + S->height);
            ++Y)
        {
            // Check if the pixel exists
            if((Pixel >= Buffer->Memory) &&
               ((Pixel + 4) <= EndOfBuffer))
            {
                if (fill == FILL)
                {
                    *(uint32 *)Pixel = Color;
                }
                else if (fill == NOFILL)
                {
                    // Only draw border
                    if ((X == S->x) ||
                        (Y == S->y) ||
                        (X == (S->x + S->width) - 1) ||
                        (Y == (S->y + S->height) - 1))
                    {
                        *(uint32 *)Pixel = Color;
                    }
                }
            }
            
            Pixel += Buffer->Pitch;
        }
    }
}

unsigned long createRGBA(int r, int g, int b, int a)
{   
    return ((a & 0xff) << 24) + ((r & 0xff) << 16) + ((g & 0xff) << 8) + ((b & 0xff));
}

unsigned long createRGB(int r, int g, int b)
{   
    return ((r & 0xff) << 16) + ((g & 0xff) << 8) + (b & 0xff);
}

#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include "stb/stb_image_resize.h"

internal void 
RenderRectImage(game_offscreen_buffer *Buffer, Rect *S, Image *image)
{
    Image re = {};
    re.data = image->data;
    re.x = S->width;
    re.y = S->height;
    re.n = image->n;
    //RenderImage(Buffer, &re);
    
    
    uint8 *EndOfBuffer = (uint8 *)Buffer->Memory + Buffer->Pitch*Buffer->Height;
    
    for(int X = S->x; X < (S->x + S->width); ++X)
    {
        uint8 *Pixel = ((uint8 *)Buffer->Memory + X * Buffer->BytesPerPixel +S->y*Buffer->Pitch);
        uint8 *Color = ((uint8 *)re.data + (X - S->x) * re.n);
        
        for(int Y = S->y; Y < (S->y + S->height); ++Y)
        {
            // Check if the pixel exists
            if((Pixel >= Buffer->Memory) && ((Pixel + 4) <= EndOfBuffer))
            {
                uint32 c = *Color;
                
                int r = *Color++;
                int g = *Color++;
                int b = *Color;
                Color--;
                Color--;
                
                c = createRGB(r, g, b);
                *(uint32 *)Pixel =c;
                Color += (re.n * re.x);
            }
            Pixel += Buffer->Pitch;
        }
    }
    
}

internal void
RenderBackgroundGrid(game_offscreen_buffer *Buffer, int GridX, int GridY, 
                     int GridWidth, int GridHeight, int GridSize, Image *image)
{
    for (int i = 0;
         i < GridWidth;
         i++)
    {
        for (int j = 0;
             j < GridHeight;
             j++)
        {
            Rect newRect = {GridX + (i * GridSize), GridY + (j * GridSize), GridSize, GridSize};
            //RenderRect(Buffer, &newRect, NOFILL, 0xFF000000);
            RenderRectImage(Buffer, &newRect, image);
        }
    }
}


// Paints the screen white
internal void
ClearScreen(game_offscreen_buffer *Buffer)
{
    uint8 *Column = (uint8 *)Buffer->Memory;    
    for(int X = 0;
        X< Buffer->Width;
        ++X)
    {
        uint8 *Pixel = ((uint8 *)Buffer->Memory +
                        X*Buffer->BytesPerPixel);
        for(int Y = 0;
            Y < Buffer->Height;
            ++Y)
        {
            *(uint32 *)Pixel = 0xFFFFFFFF;
            Pixel += Buffer->Pitch;
        }
    }
}

global_variable int GameInitialized = false;
global_variable Snake player;
global_variable GUI menu = {};

internal void 
AddSnakeNode(Snake *s, int x, int y)
{
    if (s->head == 0)
    {
        SnakeNode part1 = {};
        part1.x = x;
        part1.y = y;
        part1.nextx = x + 1;
        part1.nexty = y;
        
        s->head = (SnakeNode*)PermanentStorageAssign(&part1, sizeof(SnakeNode));
        return;
    }
    
    SnakeNode* cursor = s->head;
    while(cursor->next != 0)
    {
        cursor = cursor->next;
    }
    SnakeNode partnew = {};
    partnew.x = x;
    partnew.y = y;
    partnew.nextx = cursor->x;
    partnew.nexty = cursor->y;
    partnew.previous = cursor;
    
    cursor->next = (SnakeNode*)PermanentStorageAssign(&partnew, sizeof(SnakeNode));
}

internal void
InitializeSnake(Snake *s)
{
    s->direction = RIGHT;
    s->length = 5;
    
    AddSnakeNode(s, 10, 2);
    AddSnakeNode(s, 9, 2);
    AddSnakeNode(s, 8, 2);
    AddSnakeNode(s, 7, 2);
    AddSnakeNode(s, 6, 2);
    AddSnakeNode(s, 5, 2);
    AddSnakeNode(s, 4, 2);
    AddSnakeNode(s, 3, 2);
    AddSnakeNode(s, 2, 2);
    AddSnakeNode(s, 1, 2);
    AddSnakeNode(s, 0, 2);
}

internal void
RenderSnake(game_offscreen_buffer *Buffer, Snake* s, int GridX, int GridY, 
            int GridWidth, int GridHeight, int GridSize)
{
    SnakeNode* cursor = s->head;
    while(cursor != 0)
    {
        Rect newSquare =
        {
            GridX + (cursor->x * GridSize) + cursor->transitionx, 
            GridY + (cursor->y * GridSize) + cursor->transitiony, 
            GridSize, GridSize
        };
        RenderRect(Buffer, &newSquare, FILL, 0xFFFFEBC4);
        cursor = cursor->next;
    }
}


internal int
GetSnakeNodeDirection(SnakeNode* current, SnakeNode* next)
{
    if ((next->x - current->x) == 1)
    {
        return RIGHT;
    }
    else if ((next->x - current->x) == -1)
    {
        return LEFT;
    }
    else if ((next->y - current->y) == 1)
    {
        return DOWN;
    }
    else if ((next->y - current->y) == -1)
    {
        return UP;
    }
    
    return 0;
}

internal int
MoveSnake(Snake *snake, int gridwidth, int gridheight)
{
    // Make sure that all the nodes have been updated to their new spot
    SnakeNode* start = snake->head;
    while(start != 0)
    {
        start->x = start->nextx;
        start->y = start->nexty;
        start = start->next;
    }
    
    // Reset all transition values.
    SnakeNode* clear = snake->head;
    while(clear != 0)
    {
        clear->transitionx = 0;
        clear->transitiony = 0;
        clear = clear->next;
    }
    
    // Figure out where each part of the snake is moving next.
    SnakeNode* cursor = snake->head;
    // Check if head has room to move.
    // If it does have room set where it will move to next.
    if (snake->direction == RIGHT)
    {
        if (cursor->nextx + 1 < gridwidth)
            cursor->nextx = cursor->nextx + 1;
        else
            return 0;
        
        cursor->transitiondirection = RIGHT;
    }
    else if (snake->direction == UP)
    {
        if (cursor->nexty - 1 > -1)
            cursor->nexty = cursor->nexty - 1;
        else
            return 0;
        
        cursor->transitiondirection = UP;
    }
    else if (snake->direction == LEFT)
    {
        if (cursor->nextx - 1 > -1)
            cursor->nextx = cursor->nextx - 1;
        else
            return 0;
        
        cursor->transitiondirection = LEFT;
    }
    else if (snake->direction == DOWN)
    {
        if (cursor->nexty + 1 < gridheight)
            cursor->nexty = cursor->nexty + 1;
        else
            return 0;
        
        cursor->transitiondirection = DOWN;
    }
    cursor = cursor->next;
    
    // Move all the nodes behind the head.
    while(cursor != 0)
    {
        cursor->nextx = cursor->previous->x;
        cursor->nexty = cursor->previous->y;
        
        cursor->transitiondirection = GetSnakeNodeDirection(cursor, cursor->previous);
        cursor = cursor->next;
    }
    
    return 1;
}

int frameSkip = 6;
int framesToSkip = 6;

internal void
TransitionSnake(Snake* snake)
{
    SnakeNode* cursor = snake->head;
    while(cursor != 0)
    {
        if (cursor->transitiondirection == RIGHT)
        {
            cursor->transitionx += (GRIDSIZE / (framesToSkip + 1));
        }
        else if (cursor->transitiondirection == UP)
        {
            cursor->transitiony -= (GRIDSIZE / (framesToSkip + 1));
        }
        else if (cursor->transitiondirection == LEFT)
        {
            cursor->transitionx -= (GRIDSIZE / (framesToSkip + 1));
        }
        else if (cursor->transitiondirection == DOWN)
        {
            cursor->transitiony += (GRIDSIZE / (framesToSkip + 1));
        }
        cursor = cursor->next;
    }
}


#define STB_IMAGE_IMPLEMENTATION
//#define STBI_ASSERT(x)
//#define STBI_MALLOC
//#define STBI_REALLOC
//x#define STBI_FREE
#include "stb/stb_image.h"

internal void
RenderImage(game_offscreen_buffer *Buffer, Image *image)
{
    int xt = 50;
    for(int X = 0; X < image->x; ++X)
    {
        uint8 *Pixel = ((uint8 *)Buffer->Memory + X * Buffer->BytesPerPixel);
        uint8 *Color = ((uint8 *)image->data + X * image->n);
        
        for(int Y = 0; Y < image->y; ++Y)
        {
            uint32 c = *Color;
            
            int r = *Color++;
            int g = *Color++;
            int b = *Color;
            Color--;
            Color--;
            
            c = createRGB(r, g, b);
            *(uint32 *)Pixel =c;
            Pixel += Buffer->Pitch;
            Color += (image->n * image->x);
        }
    }
}

internal void
FilenameSearchModify(char* filename, char* result)
{
    int j = 0;
    
    char* cursor = filename;
    while (*cursor != 0)
    {
        if (*cursor == '.')
        {
            result[j] = '_';
        }
        else
        {
            result[j] = *cursor;
        }
        cursor++;
        j++;
    }
    result[j] = 0;
}

internal void
FilenameCapitialize(char* filename, char* result)
{
    int j = 0;
    
    char* cursor = filename;
    while (*cursor != 0)
    {
        if (*cursor == '_' || *cursor == '.')
        {
            result[j] = '_';
        }
        else
        {
            char c = *cursor;
            result[j] = c - 32;
        }
        cursor++;
        j++;
    }
    result[j] = 0;
}

internal void
SaveImageToHeaderFile(char* filename, Image* image)
{
    // Header file
    FILE *newfile = fopen(filename, "w");
    
    char f[sizeof(filename)];
    FilenameSearchModify(filename, f);
    char fcapital[sizeof(filename)];
    FilenameCapitialize(filename, fcapital);
    
    fprintf(newfile,
            "#ifndef %s\n"
            "#define %s\n"
            "int %sx = %d;\n"
            "int %sy = %d;\n"
            "int %sn = %d;\n"
            "const unsigned char %s[%d] = \n"
            "{\n"
            ,fcapital
            ,fcapital
            ,f
            ,image->x
            ,f
            ,image->y
            ,f
            ,image->n
            ,f
            ,(image->x * image->y * image->n));
    
    
    unsigned char* imgtosave = image->data;
    for(int i = 0; i < (image->x * image->y * image->n); i++)
    {
        fprintf(newfile,
                "%d ,",
                *imgtosave);
        *imgtosave++;
    }
    fprintf(newfile, 
            "};\n"
            "\n"
            "#endif");
    fclose(newfile);
    
    // C++ file
    char filenamecpp[sizeof(filename) + 2];
    
    int j = 0;
    int extension = 0;
    char* cursor = filename;
    while (!extension)
    {
        if (*cursor == '.')
        {
            filenamecpp[j] = *cursor;
            extension = 1;
        }
        else
        {
            filenamecpp[j] = *cursor;
        }
        
        cursor++;
        j++;
    }
    filenamecpp[j] = 'c';
    filenamecpp[j + 1] = 'p';
    filenamecpp[j + 2] = 'p';
    filenamecpp[j + 3] = 0;
    
    FILE *newcppfile = fopen(filenamecpp, "w");
    fprintf(newcppfile, 
            "internal void\n"
            "LoadImageFrom%s(Image* image)\n"
            "{\n"
            "image->data = (unsigned char *)PermanentStorageAssign((void*)&%s, sizeof(%s));\n"
            "image->x = %sx;\n"
            "image->y = %sy;\n"
            "image->n = %sn;\n"
            "}\n"
            ,f
            ,f
            ,f
            ,f
            ,f
            ,f
            );
    
    fclose(newcppfile);
    
}


global_variable Image test;

#include "../data/imagesaves/image.h"
#include "../data/imagesaves/image.cpp"

#define STB_TRUETYPE_IMPLEMENTATION
#include "stb/stb_truetype.h"
#include "stdio.h"

struct entire_file
{
    u32 ContentsSize;
    void *Contents;
};
entire_file
ReadEntireFile(char *FileName)
{
    entire_file Result = {};
    
    FILE *In = fopen(FileName, "rb");
    if(In)
    {
        fseek(In, 0, SEEK_END);
        Result.ContentsSize = ftell(In);
        fseek(In, 0, SEEK_SET);
        
        Result.Contents = malloc(Result.ContentsSize);
        fread(Result.Contents, Result.ContentsSize, 1, In);
        fclose(In);
    }
    else
    {
        printf("ERROR: Cannot open file %s.\n", FileName);
    }
    
    return(Result);
}

internal loaded_bitmap
LoadGlyphBitmap(char *FileName, char *FontName, u32 Codepoint, float scale)
{
    loaded_bitmap Result = {};
    entire_file TTFFile = ReadEntireFile(FileName);
    if(TTFFile.ContentsSize != 0)
    {
        stbtt_fontinfo Font;
        stbtt_InitFont(&Font, (u8 *)TTFFile.Contents, stbtt_GetFontOffsetForIndex((u8 *)TTFFile.Contents, 0));
        
        int Width, Height, XOffset, YOffset;
        u8 *MonoBitmap = stbtt_GetCodepointBitmap(&Font, 0, stbtt_ScaleForPixelHeight(&Font, scale),
                                                  Codepoint, &Width, &Height, &XOffset, &YOffset);
        
        /*int x0, y0, x1, y1;
        stbtt_GetCodepointBitmapBox(&Font, Codepoint, stbtt_ScaleForPixelHeight(&Font, 128.0f), stbtt_ScaleForPixelHeight(&Font, 128.0f), &x0, &y0, &x1, &y1); 
        */
        
        Result.Width = Width;
        Result.Height = Height;
        Result.Pitch = Result.Width * BITMAP_BYTES_PER_PIXEL;
        Result.Memory = malloc(Height*Result.Pitch);
        Result.Free = Result.Memory;
        
        u8 *Source = MonoBitmap;
        u8 *DestRow = (u8 *)Result.Memory;
        for(s32 Y = 0;
            Y < Height;
            ++Y)
        {
            u32 *Dest = (u32 *)DestRow;
            for(s32 X = 0;
                X < Width;
                ++X)
            {
                u8 Gray = *Source++;
                //u8 Alpha = 0xFF;
                *Dest++ = ((Gray << 24) |
                           (Gray << 16) |
                           (Gray <<  8) |
                           (Gray <<  0));
            }
            
            DestRow += Result.Pitch;
        }
        
        stbtt_FreeBitmap(MonoBitmap, 0);
        free(TTFFile.Contents);
    }
    
    return (Result);
}

internal void
RenderBitmap(game_offscreen_buffer *Buffer, loaded_bitmap *BMP, Rect *R)
{
    RenderBitmap(Buffer, BMP, R, 0);
}

internal void
RenderBitmap(game_offscreen_buffer *Buffer, loaded_bitmap *BMP, Rect *R, uint32 ColorIn)
{
    int xt = 50;
    
    uint8 *EndOfBuffer = (uint8 *)Buffer->Memory + Buffer->Pitch*Buffer->Height;
    
    for(int X = R->x; X < (R->x + R->width); ++X)
    {
        uint8 *Pixel = ((uint8 *)Buffer->Memory + X * BITMAP_BYTES_PER_PIXEL + R->y * Buffer->Pitch );
        uint8 *Color = ((uint8 *)BMP->Memory + (X - R->x) * BITMAP_BYTES_PER_PIXEL);
        
        for(int Y = R->y; Y < (R->y + R->height); ++Y)
        {
            uint32 c = *Color;
            
            
            int r = *Color++;
            int g = *Color++;
            int b = *Color++;
            int a = *Color;
            Color--;
            Color--;
            Color--;
            
            if (a > 100)
            {
                if (ColorIn == (uint32)0)
                {
                    c = createRGB(r, g, b);
                    *(uint32 *)Pixel = c;
                }
                else
                {
                    *(uint32 *)Pixel = ColorIn;
                }
            }
            
            
            
            Color += BMP->Pitch;
            Pixel += Buffer->Pitch;
        }
    }
}

internal void
ChangeBitmapColor(loaded_bitmap* BMP, uint32 Color)
{
    for (int i = 0; i < BMP->Width; i++)
    {
        uint8 *Pixel = ((uint8 *)BMP->Memory + i * BITMAP_BYTES_PER_PIXEL );
        
        for (int j = 0; j < BMP->Height; j++)
        {
            int r = *Pixel++;
            int g = *Pixel++;
            int b = *Pixel++;
            int a = *Pixel;
            Pixel--;
            Pixel--;
            Pixel--;
            
            if (a == 0xFF)
            {
                uint32 c = createRGBA(0, 255, 0, 255);
                *(uint32*)Pixel = c;
                
            }
            Pixel += BMP->Pitch;
        }
    }
}

internal LinkedListNode
CreateLinkedListNode(void* D)
{
    LinkedListNode Result = {};
    
    Result.Data = D;
    
    return(Result);
}

#define MAXSTRINGSIZE 1000

internal void
PrintOnScreen(game_offscreen_buffer *Buffer, char* text, int xin, int yin, float scalein, 
              uint32 color, Rect* alignRect)
{
    entire_file File = ReadEntireFile("../Faune-TextRegular.otf");
    
    stbtt_fontinfo info;
    stbtt_InitFont(&info, (u8 *)File.Contents, stbtt_GetFontOffsetForIndex((u8 *)File.Contents, 0));
    //stbtt_InitFont(&Font, (u8 *)TTFFile.Contents, stbtt_GetFontOffsetForIndex((u8 *)TTFFile.Contents, 0));
    
    float scale = stbtt_ScaleForPixelHeight(&info, scalein);
    
    int x = xin;
    int ascent, descent, lineGap;
    stbtt_GetFontVMetrics(&info, &ascent, &descent, &lineGap);
    
    ascent = (int)roundf(ascent * scale);
    descent = (int)roundf(descent * scale);
    
    int stringSize = (int)strlen(text);
    int stringWidth = 0;
    int stringHeight = 0;
    //LinkedList string = {maxstringsize};
    //string.Head = CreateLinkedListNode();
    
    loaded_bitmap string[MAXSTRINGSIZE];
    
    for (int i = 0; i < stringSize; ++i)
    {
        // how wide is this character
        int ax;
        int lsb;
        stbtt_GetCodepointHMetrics(&info, text[i], &ax, &lsb);
        
        // get bounding box for character (may be offset to account for chars that dip above or below the line
        int c_x1, c_y1, c_x2, c_y2;
        stbtt_GetCodepointBitmapBox(&info, text[i], scale, scale, &c_x1, &c_y1, &c_x2, &c_y2);
        
        // compute y (different characters have different heights)
        int y = ascent + c_y1 + yin;
        
        // render character
        //int byteOffset = x + roundf(lsb * scale) + (y * b_w);
        string[i] = LoadGlyphBitmap("../Faune-TextRegular.otf", "FauneRegular", text[i], scalein);
        
        // advance x 
        x += (int)roundf(ax * scale);
        
        // add kerning
        int kern;
        kern = stbtt_GetCodepointKernAdvance(&info, text[i], text[i + 1]);
        x += (int)roundf(kern * scale);
        
        //stringWidth += x;
    }
    
    stringWidth = (x - xin);
    x = xin;
    for (int i = 0; i < stringSize; i++)
    {
        // how wide is this character
        int ax;
        int lsb;
        stbtt_GetCodepointHMetrics(&info, text[i], &ax, &lsb);
        
        int c_x1, c_y1, c_x2, c_y2;
        stbtt_GetCodepointBitmapBox(&info, text[i], scale, scale, &c_x1, &c_y1, &c_x2, &c_y2);
        int y = ascent + c_y1 + yin - ((alignRect->height - ascent)/2);
        
        Rect PrintLocation = {};
        PrintLocation.x =  x + ((alignRect->width - stringWidth)/2);
        PrintLocation.y = y;
        PrintLocation.width = string[i].Width;
        PrintLocation.height = string[i].Height;
        RenderBitmap(Buffer, &string[i], &PrintLocation, color);
        
        // advance x 
        x += (int)roundf(ax * scale);
        
        // add kerning
        int kern;
        kern = stbtt_GetCodepointKernAdvance(&info, text[i], text[i + 1]);
        x += (int)roundf(kern * scale);
    }
    
}

loaded_bitmap yo;
Client client;

internal void
GameUpdateAndRender(game_memory *Memory, game_input *Input, game_offscreen_buffer *Buffer,
                    game_sound_output_buffer *SoundBuffer)
{
    Assert((&Input->Controllers[0].Terminator - &Input->Controllers[0].Buttons[0]) ==
           (ArrayCount(Input->Controllers[0].Buttons)));
    Assert(sizeof(game_state) <= Memory->PermanentStorageSize);
    
    
    game_state *GameState = (game_state *)Memory->PermanentStorage;
    
    int centeredX = (Buffer->Width - (GRIDWIDTH * GRIDSIZE)) / 2;
    int centeredY = (Buffer->Height - (GRIDHEIGHT * GRIDSIZE)) / 2;
    //Rect imageRect = {centeredX, centeredY, (GRIDWIDTH * GRIDSIZE), (GRIDHEIGHT * GRIDSIZE), 0};
    Rect imageRect = {centeredX, centeredY, (GRIDSIZE), (GRIDSIZE), 0};
    
    if(!Memory->IsInitialized)
    {
        
#if SNAKE_INTERNAL
        char *Filename = __FILE__;
        debug_read_file_result File = DEBUGPlatformReadEntireFile(Filename);
        if(File.Contents)
        {
            DEBUGPlatformWriteEntireFile("test.out", File.ContentsSize, File.Contents);
            DEBUGPlatformFreeFileMemory(File.Contents);
        }
#endif
        
        GameState->ToneHz = 256;
        GameState->Menu = 1;
        
        manager.NextStorage = (char*)Memory->PermanentStorage + sizeof(game_state);
        
#if SAVE_IMAGES
        // Set working directory in visual studio to the image save folder
        test.data = stbi_load("../grass_11zon.jpg", &test.x, &test.y, &test.n, 0);
        SaveImageToHeaderFile("image.h", &test);
#else
        
        // Load images from imagesaves folder
        LoadImageFromimage_h(&test);
        
#endif
        //loaded_bitmap grass = LoadBMP("grass.png");
        
        // Where the top left corner of the grid should be for it to look
        // centered in the window
        
        
        //Rect imageRect = {0, 0, 200, 200, 0};
        unsigned char* resized = (unsigned char *)PermanentStorageBlank(imageRect.width * imageRect.height * test.n);
        stbir_resize_uint8(test.data , test.x, test.y, 0,
                           resized, imageRect.width, imageRect.height, 0, test.n);
        unsigned char* toBeFreed = test.data;
        test.data = resized;
        
#if SAVE_IMAGES
        stbi_image_free(toBeFreed);
#endif
        yo = LoadGlyphBitmap("../Faune-TextRegular.otf", "FauneRegular", 71, 256);
        
        // TODO(casey): This may be more appropriate to do in the platform layer
        Memory->IsInitialized = true;
    }
    
    int btnPress = -1;
    int tbPress = -1;
    
    for(int ControllerIndex = 0;
        ControllerIndex < ArrayCount(Input->Controllers);
        ++ControllerIndex)
    {
        game_controller_input *Controller = GetController(Input, ControllerIndex);
        if(Controller->IsAnalog)
        {
            // NOTE(casey): Use analog movement tuning
            GameState->BlueOffset += (int)(4.0f*Controller->StickAverageX);
            GameState->ToneHz = 256 + (int)(128.0f*Controller->StickAverageY);
        }
        else
        {
            // NOTE(casey): Use digital movement tuning
            if(Controller->MoveLeft.EndedDown)
            {
                if (player.direction != RIGHT)
                    player.direction = LEFT;
            }
            
            if(Controller->MoveRight.EndedDown)
            {
                if (player.direction != LEFT)
                    player.direction = RIGHT;
            }
            
            if(Controller->MoveDown.EndedDown)
            {
                if (player.direction != UP)
                    player.direction = DOWN;
            }
            
            if(Controller->MoveUp.EndedDown)
            {
                if (player.direction != DOWN)
                    player.direction = UP;
            }
            if(Input->MouseButtons[0].EndedDown)
            {
                btnPress = CheckButtons(&menu, Input->MouseX, Input->MouseY);
                
                tbPress = CheckTextBoxes(&menu, Input->MouseX, Input->MouseY);
                
            }
        }
        
        // Input.AButtonEndedDown;
        // Input.AButtonHalfTransitionCount;
        if(Controller->ActionDown.EndedDown)
        {
            GameState->GreenOffset += 1;
        }
    }
    
    // TODO(casey): Allow sample offsets here for more robust platform options
    //GameOutputSound(SoundBuffer, GameState->ToneHz);
    
    if (btnPress == GameStart)
    {
        GameState->Menu = 0;
    }
    else if (btnPress == Quit)
    {
        Input->quit = 1;
    }
    
    if (GameState->Menu == 1)
    {
        if (menu.initialized == 0)
        {
            Button startbtn = {};
            startbtn.Shape.x = 300;
            startbtn.Shape.y = 10;
            startbtn.Shape.width = 200;
            startbtn.Shape.height = 100;
            startbtn.RegularColor = 0xFF32a89b;
            startbtn.HoverColor = 0xFFeba434;
            startbtn.Text = "Start";
            startbtn.TextColor = 0xFFFFFFFF;
            startbtn.ID = GameStart;
            
            addButton(&menu, 300, 10, 200, 100, 0xFF32a89b, 0xFFeba434, "Start", 0xFFFFFFFF, GameStart);
            addButton(&menu, 300, 120, 200, 100, 0xFF32a89b, 0xFFeba434, "Quit", 0xFFFFFFFF, Quit);
            addButton(&menu, 300, 500, 500, 200, 0xFF7d32a8, 0xFFeba434, "Menu", 0xFFFFFFFF, Join);
            //addButton(&menu, 120, 120, 100, 100, 0xFF32a89b, "yo" );
            
            addTextBox(&menu, 300, 300, 200, 100, 0xFFFFFF00, "ip", 0xFF00FFFF, IP);
            
            menu.initialized = 1;
        }
        
        CheckButtonsHover(&menu, Input->MouseX, Input->MouseY);
        
        ClearScreen(Buffer);
        RenderGUI(Buffer, &menu);
    }
    else if (GameState->Menu == 0)
    {
        if (GameInitialized == false)
        {
            InitializeSnake(&player);
            GameInitialized = true;
            
            //createClient(&client, "192.168.1.75", "10109", TCP);
        }
        
        
        //Worked to send messages to the test server
        /*
        sendq(&client, "yo", 50);
        
        char buffer[500];
        memset(buffer, 0, sizeof(buffer));
        recvq(&client, buffer, 500);
        printf("%s\n", buffer);
        */
        
        if (frameSkip == framesToSkip)
        {
            if (MoveSnake(&player, GRIDWIDTH, GRIDHEIGHT))
            {
                TransitionSnake(&player);
                frameSkip = 0;
            }
            else
            {
                
            }
        }
        else
        {
            TransitionSnake(&player);
            frameSkip++;
        }
        
        ClearScreen(Buffer);
        
        
        
        RenderBackgroundGrid(Buffer, centeredX, centeredY, GRIDWIDTH, GRIDHEIGHT, GRIDSIZE, &test);
        RenderSnake(Buffer, &player, centeredX, centeredY, GRIDWIDTH, GRIDHEIGHT, GRIDSIZE);
    }
}
