/*
Todo:
Coffee:
Coffee can't spawn under snake
Coffee resets with game

CoffeeCow:
Fix how the Rectangles outline and body are being drawn

Assets:
Think about how I am going to load them

Fonts:
Be able to resize fonts depending on the screen size

GUI:
Make adding a new gui easier
Make resizing a gui easier
*/

#ifndef SNAKE_H
#define SNAKE_H

#define RIGHT 0
#define UP 1
#define LEFT 2
#define DOWN 3
#define NODIRECTION 4

enum ComponentIDs
{
    Btn1,
    Btn2,
    Btn3,
    Btn4,
    PORT,
    IP,
    Restart,
    JOIN,
    GameStart,
    Quit,
    Menu,
    Reset,
    Multiplayer
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

struct CoffeeCowNode
{
    v2 Coords;
    int CurrentDirection;
    int NextDirection;
    bool32 Streak;
};

struct CoffeeCow
{
    Arr Nodes; // CoffeeCowNode
    
    real32 TransitionAmt = 0;
    real32 Speed;
    int Direction;
    int CLength;
    int Score;
    
    bool Moving = true;
    Arr Inputs; // int
    
    bool32 Initialized = false;
};

enum struct
RectMode
{
    Color,
    Tex,
};

struct Coffee
{
    v2 Coords;
    real32 Rotation;
    bool32 Initialized;
};

enum struct
menu
{
    game,
    main_menu,
    multiplayer_menu,
    pause_menu,
    game_over_menu,
};

enum struct
GameMode
{
    Singleplayer,
    Multiplayer,
};

struct game_state
{
    menu Menu;
    int ToneHz;
    int GridSize;
    int GridHeight;
    int GridWidth;
    
    CoffeeCow Player1;
    Coffee Collect;
};

#endif //SNAKE_H
