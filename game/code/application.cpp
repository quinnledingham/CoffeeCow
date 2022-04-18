#include "qlib/qlib.cpp"

#include "snake.h"

Image test;
Texture TexTest;
Font Faune50 = {};
Font Faune100 = {};
Font Faune = {};

internal void
RenderBackgroundGrid(int GridX, int GridY, int GridWidth, int GridHeight, int GridSize)
{
    for (int i = 0; i < GridWidth; i++)
    {
        for (int j = 0; j < GridHeight; j++)
        {
            DrawRect(GridX + (i * GridSize), GridY + (j * GridSize), GridSize, GridSize, TexTest);
        }
    }
}

internal void
SetCursorMode(platform_input *Input, CursorMode CursorM)
{
    if (Input->Cursor != CursorM)
    {
        Input->Cursor = CursorM;
        Input->NewCursor = true;
    }
}

internal void
AddInputNode(Snake *snake, int NewDirection)
{
    if (snake->InputHead == 0)
    {
        snake->InputHead = (InputNode*)qalloc(sizeof(InputNode));
        snake->InputHead->Direction = NewDirection;
        return;
    }
    
    InputNode* Cursor = snake->InputHead;
    while(Cursor->Next != 0)
    {
        Cursor = Cursor->Next;
    }
    Cursor->Next = (InputNode*)qalloc(sizeof(InputNode));
    Cursor->Direction = NewDirection;
}

internal void
PopInputNode(Snake *snake)
{
    snake->InputHead = snake->InputHead->Next;
}

internal void
AddSnakeNode(Snake *snake, real32 x, real32 y)
{
    if (snake->Head == 0)
    {
        snake->Head = (SnakeNode*)qalloc(sizeof(SnakeNode));
        snake->Head->Direction = NODIRECTION;
        snake->Head->x = x;
        snake->Head->y = y;
    }
    else
    {
        SnakeNode* Cursor = snake->Head;
        while(Cursor->Next != 0)
        {
            Cursor = Cursor->Next;
        }
        Cursor->Next = (SnakeNode*)qalloc(sizeof(SnakeNode));
        Cursor->Next->Direction = NODIRECTION;
        Cursor->Next->x = x;
        Cursor->Next->y = y;
        Cursor->Next->Previous = Cursor;
    }
}

internal void
DrawSnake(Snake *snake, int GridX, int GridY, int GridSize)
{
    SnakeNode* Cursor = snake->Head;
    while(Cursor != 0)
    {
        DrawRect((int)roundf(GridX + (Cursor->x * GridSize)),
                 (int)roundf(GridY + (Cursor->y * GridSize)),
                 GridSize, GridSize, 0xFFFF0000);
        Cursor = Cursor->Next;
    }
}

internal void
MoveSnakeNode(Snake *snake, SnakeNode *Node, real32 Dm)
{
    int* D = &Node->Direction;
    if (*D == NODIRECTION)
    {
        Node->NextDirection = snake->Direction;
        *D = snake->Direction;
    }
    
    if (snake->Head != Node)
    {
        if (snake->TransitionAmt > 1)
        {
            *D = Node->NextDirection;
            Node->NextDirection = Node->Previous->Direction;
            
            Node->x = roundf(Node->x);
            Node->y = roundf(Node->y);
            
            Dm = snake->TransitionAmt - 1;
        }
    }
    else
    {
        if (snake->TransitionAmt > 1)
        {
            if (snake->InputHead != 0)
            {
                snake->Direction = snake->InputHead->Direction;
                *D = snake->Direction;
                PopInputNode(snake);
                
                Node->x = roundf(Node->x);
                Node->y = roundf(Node->y);
                
                Dm = snake->TransitionAmt - 1;
            }
            else
            {
                Node->x = roundf(Node->x);
                Node->y = roundf(Node->y);
                
                Dm = snake->TransitionAmt - 1;
            }
        }
    }
    
    if (*D == DOWN)
    {
        Node->y += Dm;
    }
    if (*D == UP)
    {
        Node->y -= Dm;
    }
    if (*D == LEFT)
    {
        Node->x -= Dm;
    }
    if (*D == RIGHT)
    {
        Node->x += Dm;
    }
}

internal void
MoveSnake(Snake *snake, real32 SecondsElapsed)
{
    float Dm = (snake->Speed * SecondsElapsed);
    snake->TransitionAmt += Dm;
    PrintqDebug(S() + snake->Direction + "\n");
    //snake->Head->Direction = snake->Direction;
    MoveSnakeNode(snake, snake->Head, Dm);
    
    SnakeNode* Cursor = snake->Head->Next;
    while(Cursor != 0)
    {
        MoveSnakeNode(snake, Cursor, Dm);
        Cursor = Cursor->Next;
    }
    
    if (snake->TransitionAmt > 1)
    {
        //PrintqDebug("Wow");
        snake->TransitionAmt = 0;
    }
}

Snake Player = {};

Camera C =
{
    v3(0, 0, -5),
    v3(0, 0, 0),
    v3(0, 1, 0),
    0,
    0,
    0,
};

global_variable GUI MainMenu = {};

#include "../data/imagesaves/grass2.h"
#include "../data/imagesaves/grass2.cpp"

void UpdateRender(platform* p)
{
    game_state *GameState = (game_state*)p->Memory.PermanentStorage;
    
    if (!p->Initialized)
    {
        Manager.Next = (char*)p->Memory.PermanentStorage;
        qalloc(sizeof(game_state));
        
        C.FOV = 60.0f;
        C.F = 0.01f;
        p->Initialized = true;
        
        GameState->Menu = 0;
        
#if SAVE_IMAGES
        test = LoadImage("grass2.png");
        test = ResizeImage(test, GRIDSIZE, GRIDSIZE);
        SaveImage(&test, "grass2.h");
        Faune50 = LoadFont("Rubik-Medium.ttf", 50);
        Faune100 = LoadFont("Rubik-Medium.ttf", 100);
        Faune = LoadFont("Rubik-Medium.ttf", 350);
#else
        LoadImageFromgrass2_h(&test);
#endif
        TexTest.Init(&test);
    }
    
    
    float FPS = 0;
    if (p->Input.dt != 0)
    {
        FPS = 1 / p->Input.WorkSecondsElapsed;
        //PrintqDebug(S() + (int)FPS + "\n");
    }
    
    //p->Input.Seconds += p->Input.WorkSecondsElapsed;
    //PrintqDebug(S() + (int)p->Input.Seconds + "\n");
    
    int HalfGridX = (GRIDWIDTH * GRIDSIZE) / 2;
    int HalfGridY = (GRIDHEIGHT * GRIDSIZE) / 2;
    
    C.Dimension = p->Dimension;
    
    if (GameState->Menu == 1)
    {
        int btnPress = -1;
        int tbPress = -1;
        
        if(p->Input.MouseButtons[0].EndedDown)
        {
            btnPress = CheckButtonsClick(&MainMenu, p->Input.MouseX, p->Input.MouseY);
            tbPress = CheckTextBoxes(&MainMenu, p->Input.MouseX, p->Input.MouseY);
        }
        
        if (btnPress == GameStart)
        {
            SetCursorMode(&p->Input, Arrow);
            GameState->Menu = 0;
        }
        else if (btnPress == Btn4)
        {
            //createClient(&client, GetTextBoxText(&MainMenu, IP), GetTextBoxText(&MainMenu, PORT), TCP);
        }
        else if (btnPress == Quit)
        {
            p->Input.Quit = 1;
        }
        
        if (MainMenu.Initialized == 0)
        {
#include "main_menu.cpp" 
            MainMenu.Initialized = 1;
        }
        
        if (CheckButtonsHover(&MainMenu, p->Input.MouseX, p->Input.MouseY))
        {
            SetCursorMode(&p->Input, Hand);
        }
        else
        {
            SetCursorMode(&p->Input, Arrow);
        }
        
        //PrintqDebug(S() + p->Input.MouseX + " " + p->Input.MouseY + "\n");
        
        BeginMode2D(C);
        UpdateGUI(&MainMenu, p->Dimension.Width, p->Dimension.Height);
        ClearScreen();
        //bool32 g = Equal("wo", "wow");
        RenderGUI(&MainMenu);
        //DrawRect(0, 0, GRIDSIZE, GRIDSIZE, TexTest);
        //PrintOnScreen(&Faune, "Wow", 0, 0, 0xFFFF0000);
        EndMode2D();
    }
    else
    {
        if (Player.Initialized == false)
        {
            AddSnakeNode(&Player, 1, 4);
            AddSnakeNode(&Player, 1, 3);
            AddSnakeNode(&Player, 1, 2);
            AddSnakeNode(&Player, 1, 1);
            
            Player.Speed = 10; // m/s
            Player.Direction = DOWN;
            //AddInputNode(&Player, DOWN);
            Player.Initialized = true;
        }
        
        platform_controller_input *Controller = &p->Input.Controllers[0];
        if(Controller->MoveLeft.NewEndedDown)
        {
            AddInputNode(&Player, LEFT);
        }
        
        if(Controller->MoveRight.NewEndedDown)
        { 
            AddInputNode(&Player, RIGHT);
        }
        
        if(Controller->MoveDown.NewEndedDown)
        {
            AddInputNode(&Player, DOWN);
        }
        
        if(Controller->MoveUp.NewEndedDown)
        {
            AddInputNode(&Player, UP);
        }
        
        MoveSnake(&Player, p->Input.WorkSecondsElapsed);
        
        BeginMode2D(C);
        
        RenderBackgroundGrid(-HalfGridX, -HalfGridY, GRIDWIDTH, GRIDHEIGHT, GRIDSIZE);
        DrawSnake(&Player, -HalfGridX, -HalfGridY, GRIDSIZE);
        
        EndMode2D();
    }
}
