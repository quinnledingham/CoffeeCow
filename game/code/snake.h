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
    
    Arr Inputs; // int
};

struct Coffee
{
    v2 Coords;
    real32 Rotation;
    
    bool32 NewLocation;
};

enum game_asset_id
{
    GAI_Background,
    GAI_Rocks,
    GAI_Grid,
    GAI_CoffeeTex,
    GAI_Head,
    GAI_Straight,
    GAI_Corner,
    GAI_CoffeeStreak,
    GAI_HeadOutline,
    GAI_StraightOutline,
    GAI_CornerOutline,
    
    GAI_Count
};

enum font_id
{
    FI_Faune50,
    FI_Faune100,
    
    FI_COUNT
};

struct game_assets
{
    Texture *Textures[GAI_Count];
    Font *Fonts[FI_COUNT];
};
inline Texture *GetTexture(game_assets *Assets, game_asset_id ID)
{
    Texture *Result = Assets->Textures[ID];
    return Result;
}
inline Font *GetFont(game_assets *Assets, font_id ID)
{
    Font *Result = Assets->Fonts[ID];
    return Result;
}

enum struct
menu
{
    not_in_menu,
    main_menu,
    multiplayer_menu,
    pause_menu,
    game_over_menu,
};

enum struct
game_mode
{
    not_in_game,
    singleplayer,
    multiplayer,
};

struct game_state
{
    game_mode Mode;
    game_mode PreviousMode;
    menu Menu;
    
    GUI GUIs[10];
    
    bool32 ShowFPS = false;
    bool32 ResetGame = true;
    
    game_assets Assets;
    
    const char* IP;
    const char* Port;
    
    real32 GridSize;
    v2 GridDim;
    Camera C;
    
    Client client;
    CoffeeCow Player1;
    CoffeeCow Player2;
    Coffee Collect;
};

#endif //SNAKE_H
