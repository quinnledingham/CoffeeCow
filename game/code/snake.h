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

Win32:
Add DirectSound

menu:
Fix qallocing problem (only do it once)
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

#define str(x) #x
#define xstr(x) str(x)

#define pairintstring(x) {x, xstr(x)}

inline int Enum(char *a, int b)
{
    if (Equal(a, xstr(b)))
        return b;
    return -1;
}

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
inline Texture *GetTexture(game_assets *Assets, const char* ID)
{
    Texture *Result = 0;
    if (Equal(ID, "GAI_Miz"))
        Result = Assets->Textures[GAI_Miz];
    else if (Equal(ID, "GAI_Join"))
        Result = Assets->Textures[GAI_Join];
    else if (Equal(ID, "GAI_JoinAlt"))
        Result = Assets->Textures[GAI_JoinAlt];
    else if (Equal(ID, "GAI_JoinHover"))
        Result = Assets->Textures[GAI_JoinHover];
    else if (Equal(ID, "GAI_JoinAltHover"))
        Result = Assets->Textures[GAI_JoinAltHover];
    else if (Equal(ID, "GAI_MainMenuBack"))
        Result = Assets->Textures[GAI_MainMenuBack];
    return Result;
}
inline font *GetFont(game_assets *Assets, game_asset_font_id ID)
{
    font *Result = Assets->Fonts[ID];
    return Result;
}
inline font *GetFont(game_assets *Assets, const char* ID)
{
    font *Result = 0;
    if (Equal(ID, "GAFI_Rubik"))
        Result = Assets->Fonts[GAFI_Rubik];
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

struct qlib_bool
{
    bool Value;
    bool New;
};
inline void Toggle(qlib_bool *Bool)
{
    Bool->Value = !Bool->Value;
    Bool->New = true;
}
inline bool GetNew(qlib_bool *Bool)
{
    bool Ret = Bool->New;
    Bool->New = false;
    return Ret;
}

struct game_state
{
    game_mode Game;
    menu_mode Menu;
    
    qlib_bool EditMenu;
    
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
