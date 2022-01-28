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

#define STB_TRUETYPE_IMPLEMENTATION
#include "stb/stb_truetype.h"
#include "stdio.h"

#include "text.h"
#include "gui.h"

#include "text.cpp"


inline int32
RoundReal32ToInt32(real32 Real32)
{
    int32 Result = (int32)roundf(Real32);
    return(Result);
}

#include "gui.cpp"


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
            RenderRect(Buffer, &newRect, NOFILL, 0xFF000000);
            //RenderRectImage(Buffer, &newRect, image);
        }
    }
}



// Paints the screen white
internal void
ClearScreen(game_offscreen_buffer *Buffer)
{
    memset(Buffer->Memory, 0xFF, (Buffer->Width * Buffer->Height) * Buffer->BytesPerPixel);
    /*
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
*/
}

global_variable int GameInitialized = false;
global_variable Snake player;
global_variable GUI menu = {};
global_variable NewGUI MainMenu = {};
global_variable NewGUI LoseMenu = {};

internal void 
AddSnakeNode(Snake *s, int x, int y, int transitiondirection)
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
    partnew.transitiondirection = transitiondirection;
    
    cursor->next = (SnakeNode*)PermanentStorageAssign(&partnew, sizeof(SnakeNode));
}

internal void
InitializeSnake(Snake *s)
{
    s->direction = RIGHT;
    s->length = 3;
    
    
    AddSnakeNode(s, 2, 2, 0);
    AddSnakeNode(s, 1, 2, 0);
    AddSnakeNode(s, 0, 2, 0);
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

internal void
RenderApple(game_offscreen_buffer *Buffer, Apple* A, int GridX, int GridY, 
            int GridWidth, int GridHeight, int GridSize)
{
    Rect newSquare =
    {
        GridX + (A->X * GridSize), 
        GridY + (A->Y * GridSize), 
        GridSize, GridSize
    };
    RenderRect(Buffer, &newSquare, FILL, 0xFFff4040);
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
    
    snake->LastDirection = snake->direction;
    return 1;
}

int frameSkip = 10;
int framesToSkip = 10;

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


internal void
CheckGetApple(Snake* S, Apple* A)
{
    if(S->head->x == A->X && S->head->y == A->Y)
    {
        A->Score++;
        S->length++;
        
        SnakeNode* Cursor = S->head;
        while(Cursor->next != 0)
        {
            Cursor = Cursor->next;
        }
        
        if (Cursor->transitiondirection == RIGHT)
            AddSnakeNode(S, Cursor->x - 1, Cursor->y, Cursor->transitiondirection);
        if (Cursor->transitiondirection == UP)
            AddSnakeNode(S, Cursor->x, Cursor->y + 1, Cursor->transitiondirection);
        if (Cursor->transitiondirection == LEFT)
            AddSnakeNode(S, Cursor->x + 1, Cursor->y, Cursor->transitiondirection);
        if (Cursor->transitiondirection == DOWN)
            AddSnakeNode(S, Cursor->x, Cursor->y - 1, Cursor->transitiondirection);
        
        int Good = 0;
        while(!Good)
        {
            A->X = rand() % 17;
            A->Y = rand() % 17;
            Good = 1;
            
            Cursor = S->head;
            while(Cursor != 0)
            {
                if (Cursor->x == A->X && Cursor->y == A->Y)
                {
                    Good = 0;
                }
                Cursor = Cursor->next;
            }
        }
    }
}

internal int
CheckCollision(Snake* S)
{
    SnakeNode* Cursor = S->head;
    Cursor = Cursor->next;
    while(Cursor->next != 0)
    {
        if(Cursor->x == S->head->x && Cursor->y == S->head->y)
        {
            return 0;
        }
        Cursor = Cursor->next;
    }
    
    return 1;
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
SaveImageToHeaderFile(char* filename, Image* image)
{
    char* fullDir = StringConcat("imagesaves/", filename);
    
    // Header file
    FILE *newfile = fopen(fullDir, "w");
    
    char f[sizeof(filename)];
    FilenameSearchModify(filename, f);
    char fcapital[sizeof(filename)];
    FilenameCapitalize(filename, fcapital);
    
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
    char* filenamecpp = (char*)PermanentStorageAssign(fullDir, StringLength(fullDir) + 2);
    
    int j = 0;
    int extension = 0;
    char* cursor = fullDir;
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




internal void
RenderBitmap(game_offscreen_buffer *Buffer, loaded_bitmap *Bitmap, real32 RealX, real32 RealY)
{
    int32 MinX = RoundReal32ToInt32(RealX);
    int32 MinY = RoundReal32ToInt32(RealY);
    int32 MaxX = MinX + Bitmap->Width;
    int32 MaxY = MinY + Bitmap->Height;
    
    if(MinX < 0)
    {
        MinX = 0;
    }
    
    if(MinY < 0)
    {
        MinY = 0;
    }
    
    if(MaxX > Buffer->Width)
    {
        MaxX = Buffer->Width;
    }
    
    if(MaxY > Buffer->Height)
    {
        MaxY = Buffer->Height;
    }
    
    uint32 *SourceRow = (uint32*)Bitmap->Memory + Bitmap->Width * (Bitmap->Height - 1);
    uint8 *DestRow = ((uint8*)Buffer->Memory +
                      MinX*Buffer->BytesPerPixel +
                      MinY*Buffer->Pitch);
    
    for(int Y = MinY; Y < MaxY; ++Y)
    {
        uint32 *Dest = (uint32*)DestRow;
        uint32 *Source = SourceRow;
        
        for(int X = MinX; X < MaxX; ++X)
        {
            real32 A = (real32)((*Source >> 24) & 0xFF) / 255.0f;
            real32 SR = (real32)((*Source >> 16) & 0xFF);
            real32 SG = (real32)((*Source >> 8) & 0xFF);
            real32 SB = (real32)((*Source >> 0) & 0xFF);
            
            real32 DR = (real32)((*Dest >> 16) & 0xFF);
            real32 DG = (real32)((*Dest >> 8) & 0xFF);
            real32 DB = (real32)((*Dest >> 0) & 0xFF);
            
            real32 R = (1.0f-A)*DR + A*SR;
            real32 G = (1.0f-A)*DG + A*SG;
            real32 B = (1.0f-A)*DB + A*SB;
            
            *Dest = (((uint32)(R + 0.5f) << 16) |
                     ((uint32)(G + 0.5f) << 8) |
                     ((uint32)(B + 0.5f) << 0));
            
            ++Dest;
            ++Source;
        }
        
        DestRow += Buffer->Pitch;
        SourceRow -= Bitmap->Width;
    }
}


loaded_bitmap yo;
Client client;

real32 BackspaceTime = 0;
int Backspace = 0;

Font Faune50 = {};
Font Faune100 = {};

real32 SnakeTimeCounter = 0;

Apple A = {};

#include "../data/imagesaves/faunefifty.h"
#include "../data/imagesaves/fauneonehundred.h"

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
    Rect imageRect = {centeredX, centeredY, (GRIDSIZE * GRIDWIDTH), (GRIDSIZE * GRIDHEIGHT), 0};
    
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
        test.data = stbi_load("grass_11zon.jpg", &test.x, &test.y, &test.n, 0);
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
        //yo = LoadGlyphBitmap("../Faune-TextRegular.otf", "FauneRegular", 71, 256);
        
        
        
#if SAVE_IMAGES
        
        Faune50 = LoadEntireFont("Faune-TextRegular.otf", 50);
        Faune100 = LoadEntireFont("Faune-TextRegular.otf", 100);
        SaveFontToHeaderFile("FAUNE50", "faunefifty.h", "imagesaves/faunefifty.h" ,&Faune50);
        SaveFontToHeaderFile("FAUNE100", "fauneonehundred.h", "imagesaves/fauneonehundred.h" ,&Faune100);
        
#else
        LoadFontFromHeaderFile(&Faune50, FAUNE50stbtt, FAUNE50stbttuserdata, FAUNE50stbttdata, FAUNE50Size,
                               FAUNE50Ascent, FAUNE50Scale, FAUNE50FontChar, FAUNE50FontCharMemory);
        LoadFontFromHeaderFile(&Faune100, FAUNE100stbtt, FAUNE100stbttuserdata, FAUNE100stbttdata, FAUNE100Size,
                               FAUNE100Ascent, FAUNE100Scale, FAUNE100FontChar, FAUNE100FontCharMemory);
        
#endif
        
        
        // TODO(casey): This may be more appropriate to do in the platform layer
        Memory->IsInitialized = true;
    }
    
    int intFPS = (int)(1/Input->SecondsElapsed);
    char FPS[10]; 
    sprintf(FPS, "%d", intFPS);
    Rect FPSRect = {10, 10, 100, 50, 0};
    
    int btnPress = -1;
    int tbPress = -1;
    
    UpdateNewGUI(&MainMenu, Buffer->Width, Buffer->Height);
    
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
                if (player.LastDirection != RIGHT)
                    player.direction = LEFT;
            }
            
            if(Controller->MoveRight.EndedDown)
            {
                if (player.LastDirection != LEFT)
                    player.direction= RIGHT;
            }
            
            if(Controller->MoveDown.EndedDown)
            {
                if (player.LastDirection != UP)
                    player.direction= DOWN;
            }
            
            if(Controller->MoveUp.EndedDown)
            {
                if (player.LastDirection != DOWN)
                    player.direction= UP;
            }
            sprintf(FPS, "%d", player.direction);
            //PrintOnScreen(Buffer, &Faune50, FSP, FPSRect.x, FPSRect.y, 0xFF000000, &FPSRect);
            
            if(Input->MouseButtons[0].EndedDown)
            {
                btnPress = CheckButtonsClick(&MainMenu, Input->MouseX, Input->MouseY);
                
                tbPress = CheckTextBoxes(&MainMenu, Input->MouseX, Input->MouseY);
            }
            
            if(Controller->Zero.EndedDown)
            {
                AddCharTextBoxText(&MainMenu, "0");
            }
            if(Controller->One.EndedDown)
            {
                AddCharTextBoxText(&MainMenu, "1");
            }
            if(Controller->Two.EndedDown)
            {
                AddCharTextBoxText(&MainMenu, "2");
            }
            if(Controller->Three.EndedDown)
            {
                AddCharTextBoxText(&MainMenu, "3");
            }
            if(Controller->Four.EndedDown)
            {
                AddCharTextBoxText(&MainMenu, "4");
            }
            if(Controller->Five.EndedDown)
            {
                AddCharTextBoxText(&MainMenu, "5");
            }
            if(Controller->Six.EndedDown)
            {
                AddCharTextBoxText(&MainMenu, "6");
            }
            if(Controller->Seven.EndedDown)
            {
                AddCharTextBoxText(&MainMenu, "7");
            }
            if(Controller->Eight.EndedDown)
            {
                AddCharTextBoxText(&MainMenu, "8");
            }
            if(Controller->Nine.EndedDown)
            {
                AddCharTextBoxText(&MainMenu, "9");
            }
            if(Controller->Period.EndedDown)
            {
                AddCharTextBoxText(&MainMenu, ".");
            }
            if(Controller->Back.EndedDown)
            {
                if (Controller->Back.HalfTransitionCount == 1)
                {
                    RemoveCharTextBoxText(&MainMenu);
                }
            }
            
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
    
    if (tbPress == IP)
    {
        ChangeTextBoxShowCursor(&MainMenu, IP);
    }
    if (tbPress == PORT)
    {
        ChangeTextBoxShowCursor(&MainMenu, PORT);
    }
    if (GameState->Menu == 2)
    {
        if (LoseMenu.Initialized == 0)
        {
            LoseMenu.Padding = 10;
            
            int Y = 0;
            
            NewText txt = 
            {
                "YOU LOST",    // Text
                Btn1,       // ID
                &Faune100,   // Font
                0xFF000000, // TextColor
            };
            AddNewText(&LoseMenu, 0, Y++,  &txt);
            
            InitializeNewGUI(&LoseMenu);
            MainMenu.Initialized = 1;
        }
        
        CheckButtonsHover(&LoseMenu, Input->MouseX, Input->MouseY);
        
        ClearScreen(Buffer);
        RenderNewGUI(Buffer, &LoseMenu);
    }
    else if (GameState->Menu == 1)
    {
        
        if (MainMenu.Initialized == 0)
        {
            MainMenu.Padding = 10;
            
            int Y = 0;
            
            NewText txt = 
            {
                "SINGLEPLAYER",    // Text
                Btn1,       // ID
                &Faune100,   // Font
                0xFF000000, // TextColor
            };
            AddNewText(&MainMenu, 0, Y++,  &txt);
            
            NewButton btn = 
            {
                "START",    // Text
                &Faune100,   // Font
                GameStart,       // ID
                0,          // Color (CurrentColor)
                0xFF32a89b, // RegularColor
                0xFFeba434, // HoverColor
                0xFFFFFFFF, // TextColor
            };
            AddNewButton(&MainMenu, 0, Y++, 300, 100, &btn);
            
            txt = 
            {
                "0",    // Text
                Btn1,       // ID
                &Faune100,   // Font
                0xFFFFFFFF, // TextColor
            };
            AddNewText(&MainMenu, 0, Y++,  &txt);
            
            txt = 
            {
                "MULTIPLAYER",    // Text
                Btn1,       // ID
                &Faune100,   // Font
                0xFF000000, // TextColor
            };
            AddNewText(&MainMenu, 0, Y++,  &txt);
            
            txt = 
            {
                "IP:",    // Text
                Btn1,       // ID
                &Faune50,   // Font
                0xFF000000, // TextColor
            };
            AddNewText(&MainMenu, 0, Y,  &txt);
            
            NewTextBox tb =
            {
                "",
                &Faune50,
                IP,
                0,
                0xFFb3b3b3,
                0xFF000000
            };
            AddNewTextBox(&MainMenu, 1, Y++, 500, 50, &tb);
            
            txt = 
            {
                "PORT:",    // Text
                Btn1,       // ID
                &Faune50,   // Font
                0xFF000000, // TextColor
            };
            AddNewText(&MainMenu, 0, Y,  &txt);
            
            tb =
            {
                "",
                &Faune50,
                PORT,
                0,
                0xFFb3b3b3,
                0xFF000000
            };
            AddNewTextBox(&MainMenu, 1, Y++, 500, 50, &tb);
            
            btn = 
            {
                "JOIN",    // Text
                &Faune100,   // Font
                Btn4,       // ID
                0,          // Color (CurrentColor)
                0xFF32a89b, // RegularColor
                0xFFeba434, // HoverColor
                0xFFFFFFFF, // TextColor
            };
            AddNewButton(&MainMenu, 0, Y++, 200, 100, &btn);
            
            btn = 
            {
                "Quit",    // Text
                &Faune100,   // Font
                Quit,       // ID
                0,          // Color (CurrentColor)
                0xFF32a89b, // RegularColor
                0xFFeba434, // HoverColor
                0xFFFFFFFF, // TextColor
            };
            AddNewButton(&MainMenu, 0, Y++, 200, 100, &btn);
            
            InitializeNewGUI(&MainMenu);
            MainMenu.Initialized = 1;
        }
        
        CheckButtonsHover(&MainMenu, Input->MouseX, Input->MouseY);
        
        ClearScreen(Buffer);
        RenderNewGUI(Buffer, &MainMenu);
    }
    else if (GameState->Menu == 0)
    {
        if (GameInitialized == false)
        {
            InitializeSnake(&player);
            GameInitialized = true;
            
            A.X = rand() % 17;
            A.Y = rand() % 17;
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
        SnakeTimeCounter += Input->SecondsElapsed;
        
        if (SnakeTimeCounter > 0.008)
        {
            if(framesToSkip == frameSkip)
            {
                if (MoveSnake(&player, GRIDWIDTH, GRIDHEIGHT))
                {
                    CheckGetApple(&player, &A);
                    if(!CheckCollision(&player))
                    {
                        GameState->Menu = 1;
                        GameInitialized = false;
                        player = {};
                        A.Score = 0;
                    }
                    TransitionSnake(&player);
                    frameSkip = 0;
                    SnakeTimeCounter = 0;
                    
                }
                else
                {
                    GameState->Menu = 1;
                    GameInitialized = false;
                    player = {};
                    A.Score = 0;
                }
            }
            else
            {
                TransitionSnake(&player);
                frameSkip++;
            }
        }
        
        ClearScreen(Buffer);
        
        //Rect newRect = {centeredX, centeredY, GRIDSIZE * GRIDWIDTH, GRIDSIZE * GRIDHEIGHT};
        //RenderRectImage(Buffer, &newRect, &test);
        
        RenderBackgroundGrid(Buffer, centeredX, centeredY, GRIDWIDTH, GRIDHEIGHT, GRIDSIZE, &test);
        RenderApple(Buffer, &A, centeredX, centeredY, GRIDWIDTH, GRIDHEIGHT, GRIDSIZE);
        RenderSnake(Buffer, &player, centeredX, centeredY, GRIDWIDTH, GRIDHEIGHT, GRIDSIZE);
        
        sprintf(FPS, "%d", A.Score);
        PrintOnScreen(Buffer, &Faune50, FPS, FPSRect.x, FPSRect.y, 0xFF000000, &FPSRect);
    }
}
