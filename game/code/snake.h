/*
Todo:
Assets:
Think about how I am going to load them

Renderer:
Refactor code
Add live shader loading
Figure out image bitmap texture situation

Win32:
Fix NewEndedDown. Only sets NewEndedDown to false if the OnKeyDown is used in the loop for that key.

Snake:
Rename files to coffee cow - get server and client to work good together

Multiplayer:
Add different coloured snakes (Should find out how textures should work from doing this)
Make multiplayer actually a game
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
    Arr Corners;
    
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
    
    bitmap_id Bitmap;
};

enum struct
menu_mode
{
    not_in_menu,
    main_menu,
    multiplayer_menu,
    pause_menu,
    game_over_menu,
    local_multiplayer_menu,
    
    count
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

typedef platform_button_state game_button;
struct game_controller
{
    game_button *MoveUp;
    game_button *MoveLeft;
    game_button *MoveDown;
    game_button *MoveRight;
    
    game_button *Start;
};

struct game_state
{
    bool32 Initialized;
    
    game_mode Game;
    menu_mode Menu;
    
    qlib_bool EditMenu;
    
    int ActiveControllerIndex;
    game_controller Controllers[5];
    
    menu_controller MenuController;
    menu Menus[(int)menu_mode::count];
    
    gui Status;
    u32 FPSComponent;
    
    bool32 ShowFPS = false;
    bool32 ResetGame = true;
    
    assets Assets;
    
    const char* IP;
    const char* Port;
    
    real32 GridSize;
    v2 GridDim;
    camera Camera;
    
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
    
    Coffee Collect;
    
    audio_state AudioState;
    
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
