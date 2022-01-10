/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Casey Muratori $
   $Notice: (C) Copyright 2014 by Molly Rocket, Inc. All Rights Reserved. $
   ======================================================================== */

#include "stdio.h"
#include "Windows.h"
#include "snake.h"

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

// Set if the shape should be filled in
#define NOFILL 0
#define FILL 1

internal void 
RenderSquare(game_offscreen_buffer *Buffer, Square *S, int fill, uint32 color)
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
#define GRIDWIDTH 50
#define GRIDHEIGHT 20
internal void
RenderBackgroundGrid(game_offscreen_buffer *Buffer, int GridX, int GridY, 
                     int GridWidth, int GridHeight, int GridSize)
{
    for (int i = 0;
         i < GridWidth;
         i++)
    {
        for (int j = 0;
             j < GridHeight;
             j++)
        {
            Square newSquare = {GridX + (i * GridSize), GridY + (j * GridSize), GridSize, GridSize};
            RenderSquare(Buffer, &newSquare, NOFILL, 0xFF000000);
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

global_variable MemoryManager manager;
global_variable int GameInitialized = false;
global_variable Grid GameData;
global_variable Snake player;

#define NOSNAKE 0
#define SNAKE 1

internal void*
PermanentStorageAssign(MemoryManager *m, void  *newM, int size)
{
    char *csrc = (char*)m->NextStorage;
    char *cdest = (char*)newM;
    
    for (int i = 0; i < size; i++)
    {
        csrc[i] = cdest[i];
    }
    
    void *returnV = m->NextStorage;
    m->NextStorage += size;
    
    return returnV;
    //memcpy(manager->NextStorage, newM, size);
}

internal void
CreateGrid(Grid *newGrid, int width, int height)
{
    Node head = {};
    head.Data = "head";
    head.Value = NOSNAKE;
    head.NextR = 0;
    head.NextC = 0;
    newGrid->head = (Node*)PermanentStorageAssign(&manager, &head, sizeof(Node));
    newGrid->width = width;
    newGrid->height = height;
    
    Node* cursorR = newGrid->head;
    for (int i = 0; i < width; i++)
    {
        Node* cursorC = cursorR;
        for (int j = 0; j < height; j++)
        {
            Node o = {};
            o.Data = "column";
            o.Value = NOSNAKE;
            head.NextR = 0;
            head.NextC = 0;
            cursorC->NextC = (Node*)PermanentStorageAssign(&manager, &o, sizeof(Node));
            cursorC = cursorC->NextC;
        }
        
        Node n = {};
        n.Data = "row";
        n.Value = NOSNAKE;
        head.NextR = 0;
        head.NextC = 0;
        cursorR->NextR = (Node*)PermanentStorageAssign(&manager, &n, sizeof(Node));
        cursorR = cursorR->NextR;
    }
}

internal void
SetGridValue(Grid *grid, int x, int y, int value)
{
    Node* cursor = grid->head;
    for (int i = 0; i < x; i++)
    {
        cursor = cursor->NextR;
    }
    for (int j = 0; j < y; j++)
    {
        cursor = cursor->NextC;
    }
    
    cursor->Value = value;
}

internal int
GetGridValue(Grid *grid, int x, int y)
{
    Node* cursor = grid->head;
    for (int i = 0; i < x; i++)
    {
        cursor = cursor->NextR;
    }
    for (int j = 0; j < y; j++)
    {
        cursor = cursor->NextC;
    }
    
    return cursor->Value;
}

internal void
SetGridLocked(Grid *grid, int x, int y, int locked)
{
    Node* cursor = grid->head;
    for (int i = 0; i < x; i++)
    {
        cursor = cursor->NextR;
    }
    for (int j = 0; j < y; j++)
    {
        cursor = cursor->NextC;
    }
    cursor->Locked = locked;
}

internal int
GetGridLocked(Grid *grid, int x, int y)
{
    Node* cursor = grid->head;
    for (int i = 0; i < x; i++)
    {
        cursor = cursor->NextR;
    }
    for (int j = 0; j < y; j++)
    {
        cursor = cursor->NextC;
    }
    return cursor->Locked;
}

internal void
UnlockGrid(Grid *GD)
{
    for (int i = 0;
         i < GD->width;
         i++)
    {
        for (int j = 0;
             j < GD->height;
             j++)
        {
            SetGridLocked(GD, i, j, 0);
        }
    }
}

internal void
CreateSnake(Grid *GD)
{
    
}

internal void
RenderSnake(game_offscreen_buffer *Buffer, Grid *GD, int GridX, int GridY, 
            int GridWidth, int GridHeight, int GridSize)
{
    for (int i = 0;
         i < GridWidth;
         i++)
    {
        for (int j = 0;
             j < GridHeight;
             j++)
        {
            int v = GetGridValue(GD, i, j);
            if (v == SNAKE)
            {
                Square newSquare = {GridX + (i * GridSize), GridY + (j * GridSize), GridSize, GridSize};
                RenderSquare(Buffer, &newSquare, FILL, 0xFF000000);
            }
        }
    }
}

internal void
MoveSnake(Grid *GD, Snake *snake)
{
    for (int i = 0;
         i < GD->width;
         i++)
    {
        for (int j = 0;
             j < GD->height;
             j++)
        {
            int v = GetGridValue(GD, i, j);
            int l = GetGridLocked(GD, i, j);
            if (v == SNAKE && l == 0)
            {
                SetGridValue(GD, i, j, NOSNAKE);
                if (snake->direction == RIGHT)
                {
                    SetGridValue(GD, i + 1, j, SNAKE);
                    SetGridLocked(GD, i + 1, j, 1);
                }
                if (snake->direction == UP)
                {
                    SetGridValue(GD, i, j - 1, SNAKE);
                    SetGridLocked(GD, i, j - 1, 1);
                }
                if (snake->direction == LEFT)
                {
                    SetGridValue(GD, i - 1, j, SNAKE);
                    SetGridLocked(GD, i - 1, j, 1);
                }
                if (snake->direction == DOWN)
                {
                    SetGridValue(GD, i, j + 1, SNAKE);
                    SetGridLocked(GD, i, j + 1, 1);
                }
            }
        }
    }
}

real32 TimeTotal = 0;
int z = 0;
int c = 0;
int last = -1;

internal void
GameUpdateAndRender(game_memory *Memory, game_input *Input, game_offscreen_buffer *Buffer,
                    game_sound_output_buffer *SoundBuffer)
{
    Assert((&Input->Controllers[0].Terminator - &Input->Controllers[0].Buttons[0]) ==
           (ArrayCount(Input->Controllers[0].Buttons)));
    Assert(sizeof(game_state) <= Memory->PermanentStorageSize);
    
    game_state *GameState = (game_state *)Memory->PermanentStorage;
    
    if(!Memory->IsInitialized)
    {
        char *Filename = __FILE__;
        
        debug_read_file_result File = DEBUGPlatformReadEntireFile(Filename);
        if(File.Contents)
        {
            DEBUGPlatformWriteEntireFile("test.out", File.ContentsSize, File.Contents);
            DEBUGPlatformFreeFileMemory(File.Contents);
        }
        
        
        GameState->ToneHz = 256;
        
        manager.NextStorage = (char*)Memory->PermanentStorage + sizeof(game_state);
        
        // TODO(casey): This may be more appropriate to do in the platform layer
        Memory->IsInitialized = true;
    }
    
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
                GameState->BlueOffset -= 1;
            }
            
            if(Controller->MoveRight.EndedDown)
            {
                GameState->BlueOffset += 1;
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
    GameOutputSound(SoundBuffer, GameState->ToneHz);
    
    if (GameInitialized == false)
    {
        CreateGrid(&GameData, GRIDWIDTH, GRIDHEIGHT);
        SetGridValue(&GameData, 2, 2, SNAKE);
        SetGridValue(&GameData, 3, 2, SNAKE);
        SetGridValue(&GameData, 4, 2, SNAKE);
        
        player.direction = RIGHT;
        player.length = 3;
        
        GameInitialized = true;
    }
    
    MoveSnake(&GameData, &player);
    
#if 0
    TimeTotal += Input->SecondsElapsed;
    if (TimeTotal > 0.05)
    {
        TimeTotal = 0;
        SetGridValue(&GameData, z, c, SNAKE);
        SetGridValue(&GameData, last, c, NOSNAKE);
        last = z;
        
        if (z < GRIDWIDTH)
            z++;
        else
        {
            z = 0;
            last = z;
            SetGridValue(&GameData, z, c, SNAKE);
            z++;
        }
    }
    char FPSBuffer[256];
    _snprintf_s(FPSBuffer, sizeof(FPSBuffer),
                "Time: %f\n", TimeTotal);
    OutputDebugStringA(FPSBuffer);
#endif
    
    ClearScreen(Buffer);
    RenderBackgroundGrid(Buffer, 0, 0, GRIDWIDTH, GRIDHEIGHT, 25);
    RenderSnake(Buffer, &GameData, 0, 0, GRIDWIDTH, GRIDHEIGHT, 25);
    //RenderWeirdGradient(Buffer, GameState->BlueOffset, GameState->GreenOffset);
    UnlockGrid(&GameData);
}
