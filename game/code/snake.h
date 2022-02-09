#if !defined(SNAKE_H)
/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Casey Muratori $
   $Notice: (C) Copyright 2014 by Molly Rocket, Inc. All Rights Reserved. $
   ======================================================================== */

/*
  NOTE(casey):

  HANDMADE_INTERNAL:
    0 - Build for public release
    1 - Build for developer only

  HANDMADE_SLOW:
    0 - Not slow code allowed!
    1 - Slow code welcome.
*/

#if SNAKE_SLOW
// TODO(casey): Complete assertion macro - don't worry everyone!
#define Assert(Expression) if(!(Expression)) {*(int *)0 = 0;}
#else
#define Assert(Expression)
#endif

#define Kilobytes(Value) ((Value)*1024LL)
#define Megabytes(Value) (Kilobytes(Value)*1024LL)
#define Gigabytes(Value) (Megabytes(Value)*1024LL)
#define Terabytes(Value) (Gigabytes(Value)*1024LL)

#define ArrayCount(Array) (sizeof(Array) / sizeof((Array)[0]))
// TODO(casey): swap, min, max ... macros???

#define internal static 
#define local_persist static 
#define global_variable static

#define Pi32 3.14159265359f

typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;
typedef int32 bool32;

typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;

typedef float real32;
typedef double real64;

typedef int8 s8;
typedef int8 s08;
typedef int16 s16;
typedef int32 s32;
typedef int64 s64;
typedef bool32 b32;

typedef uint8 u8;
typedef uint8 u08;
typedef uint16 u16;
typedef uint32 u32;
typedef uint64 u64;

typedef real32 r32;
typedef real64 r64;

typedef uintptr_t umm;
typedef intptr_t smm;

#define BITMAP_BYTES_PER_PIXEL 4

inline uint32
SafeTruncateUInt64(uint64 Value)
{
    // TODO(casey): Defines for maximum values
    Assert(Value <= 0xFFFFFFFF);
    uint32 Result = (uint32)Value;
    return(Result);
}

/*
  NOTE(casey): Services that the platform layer provides to the game
*/
#if SNAKE_INTERNAL
/* IMPORTANT(casey):

   These are NOT for doing anything in the shipping game - they are
   blocking and the write doesn't protect against lost data!
*/
struct debug_read_file_result
{
    uint32 ContentsSize;
    void *Contents;
};
internal debug_read_file_result DEBUGPlatformReadEntireFile(char *Filename);
internal void DEBUGPlatformFreeFileMemory(void *Memory);
internal bool32 DEBUGPlatformWriteEntireFile(char *Filename, uint32 MemorySize, void *Memory);
#endif

/*
  NOTE(casey): Services that the game provides to the platform layer.
  (this may expand in the future - sound on separate thread, etc.)
*/

// FOUR THINGS - timing, controller/keyboard input, bitmap buffer to use, sound buffer to use

// TODO(casey): In the future, rendering _specifically_ will become a three-tiered abstraction!!!
struct game_offscreen_buffer
{
    // NOTE(casey): Pixels are alwasy 32-bits wide, Memory Order BB GG RR XX
    void *Memory;
    int Width;
    int Height;
    int Pitch;
    int BytesPerPixel;
};

struct game_sound_output_buffer
{
    int SamplesPerSecond;
    int SampleCount;
    int16 *Samples;
};

struct game_button_state
{
    int HalfTransitionCount;
    bool32 EndedDown;
};

struct game_controller_input
{
    bool32 IsConnected;
    bool32 IsAnalog;    
    real32 StickAverageX;
    real32 StickAverageY;
    
    union
    {
        game_button_state Buttons[12];
        struct
        {
            game_button_state MoveUp;
            game_button_state MoveDown;
            game_button_state MoveLeft;
            game_button_state MoveRight;
            
            game_button_state ActionUp;
            game_button_state ActionDown;
            game_button_state ActionLeft;
            game_button_state ActionRight;
            
            game_button_state LeftShoulder;
            game_button_state RightShoulder;
            
            game_button_state Back;
            game_button_state Start;
            
            // NOTE(casey): All buttons must be added above this line
            
            game_button_state Terminator;
        };
    };
    
    union
    {
        game_button_state Numbers[12];
        struct
        {
            game_button_state Zero;
            game_button_state One;
            game_button_state Two;
            game_button_state Three;
            game_button_state Four;
            game_button_state Five;
            game_button_state Six;
            game_button_state Seven;
            game_button_state Eight;
            game_button_state Nine;
            game_button_state Period;
            game_button_state Backspace;
        };
    };
};

struct game_input
{
    game_button_state MouseButtons[5];
    int32 MouseX, MouseY, MouseZ;
    
    // TODO(casey): Insert clock values here.    
    game_controller_input Controllers[5];
    real32 SecondsElapsed;
    
    int32 quit;
};
inline game_controller_input *GetController(game_input *Input, int unsigned ControllerIndex)
{
    Assert(ControllerIndex < ArrayCount(Input->Controllers));
    
    game_controller_input *Result = &Input->Controllers[ControllerIndex];
    return(Result);
}

struct game_memory
{
    bool32 IsInitialized;
    
    uint64 PermanentStorageSize;
    void *PermanentStorage; // NOTE(casey): REQUIRED to be cleared to zero at startup
    
    uint64 TransientStorageSize;
    void *TransientStorage; // NOTE(casey): REQUIRED to be cleared to zero at startup
    
    void *Bitmap;
};

internal void GameUpdateAndRender(game_memory *Memory, game_input *Input, game_offscreen_buffer *Buffer,
                                  game_sound_output_buffer *SoundBuffer);

//
//
//

struct game_state
{
    int Menu;
    int ToneHz;
    int GreenOffset;
    int BlueOffset;
};

// Set if the shape should be filled in
#define NOFILL 0
#define FILL 1

struct Rect
{
    int x;
    int y;
    int width;
    int height;
    uint32 color;
};

struct NewRect
{
    float x;
    float y;
    int width;
    int height;
    uint32 color;
};

internal void 
RenderRect(game_offscreen_buffer *Buffer, Rect *S, int fill, uint32 color);

#define GRIDWIDTH 17
#define GRIDHEIGHT 17
#define GRIDSIZE 30

#define RIGHT 0
#define UP 1
#define LEFT 2
#define DOWN 3

struct SnakeNode
{
    real32 X;
    real32 Y;
    int Direction;
    int NextDirection;
    
    SnakeNode* Next;
    SnakeNode* Previous;
};

struct Snake
{
    int Direction;
    int NextDirection = -1;
    int LastMoveDirection; // Direction when MoveSnake was called last
    int Length;
    real32 Speed;
    real32 MaxSpeed;
    real32 DistanceTravelled;
    SnakeNode* Head;
};

struct Apple
{
    int X;
    int Y;
    int Score;
};

struct Image
{
    int x;
    int y;
    int n;
    unsigned char* data;
};

struct loaded_bitmap
{
    int32 Width;
    int32 Height;
    int32 Pitch;
    void *Memory;
    
    void *Free;
};

struct LinkedListNode
{
    void* Data;
    LinkedListNode* Next;
};

struct LinkedList
{
    LinkedListNode* Head;
};

union v2
{
    struct
    {
        real32 x, y;
    };
    struct
    {
        real32 u, v;
    };
    real32 E[2];
};

struct PrintOnScreenReturn
{
    v2 Top;
    
    int Width;
    int Height;
};

struct entire_file
{
    u32 ContentsSize;
    void *Contents;
};

entire_file
ReadEntireFile(char *FileName);

internal void
RenderBitmap(game_offscreen_buffer *Buffer, loaded_bitmap *Bitmap, real32 RealX, real32 RealY);



#define SNAKE_H
#endif
