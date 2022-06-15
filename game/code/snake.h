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
    int Score;
    
    bool Moving = true;
    Arr Inputs; // int
    
    Texture *Head;
    Texture *Straight;
    Texture *Corner;
    Texture *CoffeeStreak;
    Texture *HeadOutline;
    Texture *StraightOutline;
    Texture *CornerOutline;
    
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
    Texture *CoffeeTex;
    
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
    GameMode Mode;
    GUI GUIs[10];
    
    Arr Textures;
    //Texture Textures[100];
    //Font Fonts[100];
    
    Texture *Background;
    Texture *Grid;
    Texture *Rocks;
    
    Font Faune50 = {};
    Font Faune100 = {};
    Font Faune = {};
    
    const char* IP;
    const char* Port;
    
    int ToneHz;
    int GridSize;
    int GridHeight;
    int GridWidth;
    Camera C;
    
    Client client;
    CoffeeCow Player1;
    CoffeeCow Player2;
    Coffee Collect;
};

#endif //SNAKE_H
