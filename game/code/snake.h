#ifndef SNAKE_H
#define SNAKE_H

struct game_state
{
    int Menu;
    int ToneHz;
    int GreenOffset;
    int BlueOffset;
};

struct Circle
{
    int X;
    int Y;
    int Radius;
    float StartDegree;
    uint32 Color;
};

#define GRIDWIDTH 17
#define GRIDHEIGHT 17
#define GRIDSIZE 70

#define RIGHT 0
#define UP 1
#define LEFT 2
#define DOWN 3
#define NODIRECTION 4

struct SnakeNode
{
    real32 x;
    real32 y;
    int GridX;
    int GridY;
    int Direction;
    int NextDirection;
    
    SnakeNode *Next;
    SnakeNode *Previous;
};

struct InputNode
{
    int Direction;
    InputNode *Next;
};

struct Snake
{
    SnakeNode *Head;
    real32 TransitionAmt = 0;
    real32 Speed;
    int Direction;
    
    bool Moving = true;
    InputNode *InputHead;
    
    bool32 Initialized = false;
};

struct Apple
{
    int X;
    int Y;
    int Score;
};

struct Coffee
{
    int X;
    int Y;
};

struct CoffeeCowNode
{
    v2 Coords;
    int CurrentDirection;
    int NextDirection;
};

struct CoffeeCowCollisionNode
{
    int X;
    int Y;
};

struct Arr
{
    void *Data;
    int Size;
    int MaxSize;
    int TypeSize;
    
    // Number of indices. Size of indices.
    void Init(int ms, int ts)
    {
        MaxSize = ms;
        TypeSize = ts;
        
        Data = qalloc(ms * ts);
    }
    
    void Del()
    {
        dalloc(Data);
    }
    
    void Push(void *NewData)
    {
        char *Cursor = (char*)Data;
        Cursor += (Size * TypeSize);
        memcpy(Cursor, NewData, TypeSize);
        Size++;
    }
    
    void PopFront()
    {
        char *Cursor = (char*)Data;
        Cursor += TypeSize;
        memcpy(Data, Cursor, Size * TypeSize);
        Size--;
    }
    
    void Clear()
    {
        memset(Data, 0, MaxSize * TypeSize);
        Size = 0;
    }
    
    void* operator[](int i)
    {
        char *Cursor = (char*)Data;
        Cursor += (i * TypeSize);
        return (void*)Cursor;
    }
};

struct CoffeeCow
{
    Arr Nodes;
    
    real32 TransitionAmt = 0;
    real32 Speed;
    int Direction;
    int CLength;
    int Score;
    
    bool Moving = true;
    Arr Inputs;
    
    bool32 Initialized = false;
};

struct Rect
{
    v3 Coords;
    v2 Size;
    Texture Tex;
    real32 Rotation;
    BlendMode Mode;
    
    inline Rect() : Coords(v3(0, 0, 0)), Size(v2(0, 0)), Tex(), 
    Rotation(0), Mode(BlendMode::gl_src_alpha) {}
    inline Rect(v3 _Coords, v2 _Size, Texture _Tex, real32 _Rotation, BlendMode _Mode) :
    Coords(_Coords), Size(_Size), Tex(_Tex), Rotation(_Rotation), Mode(_Mode) {}
};

struct RectBuffer
{
    Rect Buffer[1000];
    int Size = 0;
    int MaxSize = 1000;
    
    void Push(Rect NewRect)
    {
        Buffer[Size] = NewRect;
        Size++;
    }
    
    void Clear()
    {
        memset(Buffer, 0, MaxSize * sizeof(Rect));
        Size = 0;
    }
};


#endif //SNAKE_H
