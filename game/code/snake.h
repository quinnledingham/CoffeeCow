/*
Todo:
Coffee:
Make Coffee not theoretically run forever

Assets:
Think about how I am going to load them

Renderer:
Refactor code
Add live shader loaing
Figure out image bitmap texture situation

Win32
Add DirectSound
*/

#ifndef SNAKE_H
#define SNAKE_H

#define RIGHT 0
#define UP 1
#define LEFT 2
#define DOWN 3
#define NODIRECTION 4

enum game_direction
{
    Right,
    Up,
    Left,
    Down,
    
    NoDirection
};

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
    Reset,
    Multiplayer
};

struct CoffeeCowTail
{
    v2 Coords;
    bool ChangingDirections;
    int OldDir;
    int NewDir;
};

struct CoffeeCowNode
{
    v2 Coords;
    int CurrentDirection;
    int NextDirection;
    
    int Streak;
};

struct CoffeeCow
{
    Arr Nodes; // CoffeeCowNode
    
    real32 TransitionAmt = 0;
    real32 Speed;
    int Direction;
    int Score;
    
    CoffeeCowTail Tail;
    real32 MouthOpening = 0;
    real32 LastTransitionAmt = 0;
    
    Arr Inputs; // int
    
    platform_controller_input *Input;
};

struct Coffee
{
    v2 Coords;
    real32 Rotation;
    
    bool IncreasingHeight;
    real32 Height;
    
    bool32 NewLocation;
};

enum game_asset_id
{
    GAI_Background,
    GAI_Grass,
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
    GAI_Tail,
    GAI_Tongue,
    GAI_Miz,
    GAI_Join,
    GAI_JoinAlt,
    GAI_JoinHover,
    GAI_JoinAltHover,
    
    GAI_MainMenuBack,
    
    GAI_Count
};


enum game_asset_font_id
{
    GAFI_Rubik,
    
    GAFI_Count
};
struct game_assets
{
    Texture *Textures[GAI_Count];
    Texture *Spots[4];
    
    font *Fonts[GAFI_Count];
};
inline Texture *GetTexture(game_assets *Assets, game_asset_id ID)
{
    Texture *Result = Assets->Textures[ID];
    return Result;
}
inline font *GetFont(game_assets *Assets, game_asset_font_id ID)
{
    font *Result = Assets->Fonts[ID];
    return Result;
}

enum struct
menu_mode
{
    not_in_menu,
    main_menu,
    multiplayer_menu,
    pause_menu,
    game_over_menu,
    local_multiplayer_menu,
    
    menu_mode_Count
};
enum struct
game_mode

{
    not_in_game,
    singleplayer,
    multiplayer,
    local_multiplayer,
};

#define MAX_THREADS 2
struct thread
{
    DWORD dwThreadIdArray[MAX_THREADS];
    HANDLE hThreadArray[MAX_THREADS]; 
};

struct thread_param
{
    CoffeeCow *Cow;
    client *Client;
};

struct game_state
{
    game_mode Game;
    menu_mode Menu;
    menu Menus[(int)menu_mode::menu_mode_Count];
    
    bool32 ShowFPS = false;
    bool32 ResetGame = true;
    
    game_assets Assets;
    
    const char* IP;
    const char* Port;
    
    real32 GridSize;
    v2 GridDim;
    Camera C;
    
    char Buffer[BUF_SIZE];
    client Client;
    thread_param ThreadParams;
    thread Thread;
    int8 Disconnect = 0;
    
    union
    {
        CoffeeCow Players[4];
        struct
        {
            CoffeeCow Player1;
            CoffeeCow Player2;
            CoffeeCow Player3;
            CoffeeCow Player4;
        };
    };
    
    platform_controller_input *ActiveInput;
    
    Coffee Collect;
    
    v2 OldPlatformDim;
    
    platform_work_queue *Queue;
    HANDLE ThreadHandle;
};
inline menu* GetMenu(game_state *GameState, menu_mode MenuMode)
{
    return &GameState->Menus[(int)MenuMode];
}
inline void SetMenu(game_state *GameState, menu_mode MenuMode)
{
    if (GameState->Menu != MenuMode) {
        menu *Menu = GetMenu(GameState, MenuMode);
        Menu->Reset = true;
        GameState->Menu = MenuMode;
    }
}
inline void  MenuToggle(game_state *GameState, menu_mode Default, menu_mode Toggle) 
{
    if (GameState->Menu == Toggle)
        SetMenu(GameState, Default);
    else if (GameState->Menu== Default)
        SetMenu(GameState, Toggle);
}

#endif //SNAKE_H
