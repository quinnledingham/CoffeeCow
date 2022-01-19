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

global_variable MemoryManager manager;
global_variable int GameInitialized = false;
global_variable Snake player;

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
        
        s->head = (SnakeNode*)PermanentStorageAssign(&manager, &part1, sizeof(SnakeNode));
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
    
    cursor->next = (SnakeNode*)PermanentStorageAssign(&manager, &partnew, sizeof(SnakeNode));
}

internal void
InitializeSnake(Snake *s)
{
    s->direction = RIGHT;
    s->length = 5;
    
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
        Square newSquare =
        {
            GridX + (cursor->x * GridSize) + cursor->transitionx, 
            GridY + (cursor->y * GridSize) + cursor->transitiony, 
            GridSize, GridSize
        };
        RenderSquare(Buffer, &newSquare, FILL, 0xFF32a89b);
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

int skip = 6;
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

Client client;

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
    
    if (skip == framesToSkip)
    {
        if (MoveSnake(&player, GRIDWIDTH, GRIDHEIGHT))
        {
            TransitionSnake(&player);
            skip = 0;
        }
        else
        {
            
        }
    }
    else
    {
        TransitionSnake(&player);
        skip++;
    }
    
    ClearScreen(Buffer);
    
    // Where the top left corner of the grid should be for it to look
    // centered in the window
    int centeredX = (Buffer->Width - (GRIDWIDTH * GRIDSIZE)) / 2;
    int centeredY = (Buffer->Height - (GRIDHEIGHT * GRIDSIZE)) / 2;
    
    RenderBackgroundGrid(Buffer, centeredX, centeredY, GRIDWIDTH, GRIDHEIGHT, GRIDSIZE);
    RenderSnake(Buffer, &player, centeredX, centeredY, GRIDWIDTH, GRIDHEIGHT, GRIDSIZE);
    //RenderWeirdGradient(Buffer, GameState->BlueOffset, GameState->GreenOffset);
}
