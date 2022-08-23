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

enum game_direction
{
    RIGHT,
    UP,
    LEFT,
    DOWN,
    
    NODIRECTION
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
    
    asset_tag_id Tag;
    game_controller Controller;
};

struct coffee_cow_collision_node
{
    u32 NumNodes;
    v2 Nodes[400];
    v2 Head;
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
    main_menu,
    multiplayer_menu,
    pause_menu,
    game_over_menu,
    local_multiplayer_menu,
    
    not_in_menu
};
enum struct
game_mode
{
    singleplayer,
    multiplayer,
    local_multiplayer,
    
    not_in_game
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
    bool32 Initialized;
    
    game_mode Game;
    menu_mode Menu;
    
    menu_controller MenuController;
    menu Menus[(int)menu_mode::not_in_menu];
    
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
    
    u32 NumPlayers;
    CoffeeCow Players[4];
    
    u32 NumCoffees;
    Coffee Coffees[4];
    
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
