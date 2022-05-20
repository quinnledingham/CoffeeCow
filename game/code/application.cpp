#include "qlib/qlib.cpp"

#include "snake.h"

Image test;
Texture TexTest;
Texture Background;
Texture Grid;
Texture Rocks;
Texture SnakeHead;
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
            DrawRect(GridX + (i * GridSize), GridY + (j * GridSize), 0, GridSize, GridSize, Background, 0);
        }
    }
}

internal void
RenderGrid(int GridX, int GridY, int GridWidth, int GridHeight, int GridSize)
{
    for (int i = 0; i < GridWidth; i++)
    {
        for (int j = 0; j < GridHeight; j++)
        {
            DrawRect(GridX + (i * GridSize), GridY + (j * GridSize), -0.9f, GridSize, GridSize, Grid, 0);
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
    else
    {
        Input->NewCursor = false;
    }
}

internal bool32
ValidDirection(int OldDirection, int NewDirection)
{
    // Same Direction
    if (OldDirection == NewDirection)
    {
        return false;
    }
    // Backwards Direction
    if (OldDirection + 2 == NewDirection || OldDirection - 2 == NewDirection)
    {
        return false;
    }
    
    return true;
}

internal void
AddInputNode(Snake *snake, int NewDirection)
{
    if (snake->InputHead == 0)
    {
        if (!ValidDirection(snake->Direction, NewDirection))
        {
            return;
        }
        snake->InputHead = (InputNode*)qalloc(sizeof(InputNode));
        snake->InputHead->Direction = NewDirection;
        return;
    }
    
    InputNode* Cursor = snake->InputHead;
    while(Cursor->Next != 0)
    {
        Cursor = Cursor->Next;
    }
    
    if (!ValidDirection(Cursor->Direction, NewDirection))
    {
        return;
    }
    
    Cursor->Next = (InputNode*)qalloc(sizeof(InputNode));
    Cursor->Next->Direction = NewDirection;
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
InitSnake(Snake *snake)
{
    SnakeNode* Cursor = snake->Head;
    while(Cursor != 0)
    {
        // Sets the snake to move in the same direction
        int* D = &Cursor->Direction;
        if (*D == NODIRECTION)
        {
            Cursor->NextDirection = snake->Direction;
            *D = snake->Direction;
        }
        Cursor = Cursor->Next;
    }
}

internal void
DrawSnake(Snake *snake, int GridX, int GridY, int GridSize)
{
    SnakeNode* Cursor = snake->Head;
    
    int Rotation = 0;
    if (Cursor->Direction == LEFT)
        Rotation = 90;
    if (Cursor->Direction == RIGHT)
        Rotation = -90;
    if (Cursor->Direction == UP)
        Rotation = 180;
    if (Cursor->Direction == DOWN)
        Rotation = 0;
    
    DrawRect((int)roundf(GridX + (Cursor->x * GridSize)), (int)roundf(GridY + (Cursor->y * GridSize)), 1,
             GridSize, GridSize, SnakeHead, (real32)Rotation);
    Cursor = Cursor->Next;
    
    while(Cursor != 0)
    {
        
        
        // Draw turning snake
        if (Cursor->Next != 0 &&
            Cursor->Direction != Cursor->Next->Direction)
        {
            DrawRect((int)roundf(GridX + ((real32)Cursor->Next->GridX * GridSize)),
                     (int)roundf(GridY + ((real32)Cursor->Next->GridY * GridSize)),
                     GridSize, GridSize, 0xFF964B00);
        }
        
        
        DrawRect((int)roundf(GridX + (Cursor->x * GridSize)),
                 (int)roundf(GridY + (Cursor->y * GridSize)),
                 GridSize, GridSize, 0xFF964B00);
        
        Cursor = Cursor->Next;
    }
}

internal bool32
CheckMoveSnakeNode(real32 x, real32 y, int Direction)
{
    if (Direction == LEFT && x <= 0)
        return false;
    if (Direction == RIGHT && x >= GRIDWIDTH - 1)
        return false;
    if (Direction == UP && y <= 0)
        return false;
    if (Direction == DOWN && y >= GRIDHEIGHT - 1)
        return false;
    
    return true;
}

// returns amount to move
internal bool32
DetermineNextDirectionSnakeNode(Snake *snake)
{
    SnakeNode* Node = snake->Head;
    while(Node != 0)
    {
        if (snake->Head == Node && snake->InputHead != 0)
        {
            snake->Direction = snake->InputHead->Direction;
            Node->Direction = snake->Direction;
            PopInputNode(snake);
        }
        else if (snake->Head != Node)
        {
            Node->Direction = Node->NextDirection;
            Node->NextDirection = Node->Previous->Direction;
        }
        
        Node->x = roundf(Node->x);
        Node->y = roundf(Node->y);
        
        if (snake->Head == Node && !CheckMoveSnakeNode(snake->Head->x, snake->Head->y, snake->Head->Direction))
        {
            return false;
        }
        
        // Set SnakeNode next grid position
        if (Node->Direction == RIGHT)
        {
            Node->GridX = (int)Node->x + 1;
            Node->GridY = (int)Node->y;
        }
        if (Node->Direction == UP)
        {
            Node->GridX = (int)Node->x;
            Node->GridY = (int)Node->y - 1;
        }
        if (Node->Direction == LEFT)
        {
            Node->GridX = (int)Node->x - 1;
            Node->GridY = (int)Node->y;
        }
        if (Node->Direction == DOWN)
        {
            Node->GridX = (int)Node->x;
            Node->GridY = (int)Node->y + 1;
        }
        
        Node = Node->Next;
    }
    
    return true;
}

internal void
MoveSnakeNode(Snake* snake, real32 Dm)
{
    SnakeNode* Node = snake->Head;
    while(Node != 0)
    {
        int* D = &Node->Direction;
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
        
        Node = Node->Next;
    }
}

internal void
AlignSnake(Snake *snake)
{
    SnakeNode* Node = snake->Head;
    while(Node != 0)
    {
        Node->x = roundf(Node->x);
        Node->y = roundf(Node->y);
        
        Node = Node->Next;
    }
}

internal void
MoveSnake(Snake *snake, real32 SecondsElapsed)
{
    real32 Dm = (snake->Speed * SecondsElapsed);
    real32 TransitionAmt = snake->TransitionAmt + Dm;
    
    if (TransitionAmt > 1)
    {
        if (!DetermineNextDirectionSnakeNode(snake))
        {
            AlignSnake(snake);
            return;
        }
        Dm = TransitionAmt - 1;
        
        snake->TransitionAmt = 0;
        MoveSnakeNode(snake, Dm);
    }
    else
    {
        MoveSnakeNode(snake, Dm);
        snake->TransitionAmt += Dm;
    }
}

Snake Player = {};

Camera C =
{
    v3(0, 0, -900),
    v3(0, 0, 0),
    v3(0, 1, 0),
    0,
    0,
    0,
};

real32 Rotation = 0;

global_variable GUI MainMenu = {};
global_variable GUI PauseMenu = {};

//#include "../data/imagesaves/grass2.h"
//#include "../data/imagesaves/grass2.cpp"

void UpdateRender(platform* p)
{
    game_state *GameState = (game_state*)p->Memory.PermanentStorage;
    
    if (!p->Initialized)
    {
        Manager.Next = (char*)p->Memory.PermanentStorage;
        qalloc(sizeof(game_state));
        
        C.FOV = 90.0f;
        C.F = 0.01f;
        p->Initialized = true;
        
        GameState->Menu = 1;
        
        Image ImgGrid = {};
        Image ImgBackground = {};
        Image ImgRocks = {};
        Image ImgSnakeHead = {};
        
#if SAVE_IMAGES
        test = LoadImage("grass2.png");
        test = ResizeImage(test, GRIDWIDTH * GRIDSIZE, GRIDHEIGHT * GRIDSIZE);
        //SaveImage(&test, "grass2.h");
        
        ImgGrid = LoadImage("grid.png");
        ImgGrid = ResizeImage(ImgGrid, GRIDSIZE, GRIDSIZE);
        
        ImgSnakeHead = LoadImage("cowhead.png");
        ImgSnakeHead = ResizeImage(ImgSnakeHead, GRIDSIZE, GRIDSIZE);
        
        //ImgBackground = LoadImage("sand.png");
        //ImgBackground = ResizeImage(ImgBackground, , GRIDHEIGHT * GRIDSIZE);
        
        int GridSizeWidth = GRIDWIDTH * GRIDSIZE;
        int GridSizeHeight = GRIDHEIGHT * GRIDSIZE;
        
        ImgRocks = LoadImage("rocks.png");
        ImgRocks = ResizeImage(ImgRocks, GridSizeWidth * 4/3 - 1, GridSizeHeight * 4/3 - 1);
        
        Faune50 = LoadFont("Rubik-Medium.ttf", 50);
        Faune100 = LoadFont("Rubik-Medium.ttf", 100);
        Faune = LoadFont("Rubik-Medium.ttf", 350);
#else
        LoadImageFromgrass2_h(&test);
#endif
        TexTest.Init(&test);
        Grid.Init(&ImgGrid);
        Background.Init("sand.png");
        Rocks.Init(&ImgRocks);
        SnakeHead.Init(&ImgSnakeHead);
    }
    
    
    float FPS = 0;
    if (p->Input.dt != 0)
    {
        FPS = 1 / p->Input.WorkSecondsElapsed;
        PrintqDebug(S() + "FPS: " + (int)FPS + "\n");
    }
    
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
            return;
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
    else if (GameState->Menu == 0)
    {
        if (Player.Initialized == false)
        {
            AddSnakeNode(&Player, 1, 4);
            AddSnakeNode(&Player, 1, 3);
            AddSnakeNode(&Player, 1, 2);
            AddSnakeNode(&Player, 1, 1);
            
            Player.Speed = 10; // m/s
            Player.Direction = DOWN;
            InitSnake(&Player);
            //AddInputNode(&Player, DOWN);
            Player.Initialized = true;
        }
        
        platform_controller_input *Controller = &p->Input.Controllers[0];
        
        if (Controller->Escape.NewEndedDown)
        {
            GameState->Menu = 2;
        }
        
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
        
        if(Controller->ActionUp.EndedDown)
        {
            Rotation += 5.0f;
        }
        if(Controller->ActionDown.EndedDown)
        {
            Rotation -= 5.0f;
        }
        
        MoveSnake(&Player, p->Input.WorkSecondsElapsed);
        
        BeginMode2D(C);
        
        DrawRect(-(p->Dimension.Width)/2 - 5,
                 -(p->Dimension.Height)/2 - 5, 1.0f,
                 p->Dimension.Width + 5,  
                 p->Dimension.Height + 5, Background, 0);
        RenderGrid(-HalfGridX, -HalfGridY, GRIDWIDTH, GRIDHEIGHT, GRIDSIZE);
        DrawSnake(&Player, -HalfGridX, -HalfGridY, GRIDSIZE);
        DrawRect(-HalfGridX - (GRIDSIZE * GRIDWIDTH * 1/6),
                 -HalfGridY - (GRIDSIZE * GRIDHEIGHT * 1/6),
                 -1.0f,
                 Rocks.mWidth, Rocks.mHeight,
                 Rocks, Rotation);
        
        
        EndMode2D();
    }
    else if (GameState->Menu == 2)
    {
        if (PauseMenu.Initialized == 0)
        {
#include "pause_menu.cpp" 
            PauseMenu.Initialized = 1;
        }
        
        int btnPress = -1;
        int tbPress = -1;
        
        if(p->Input.MouseButtons[0].EndedDown)
        {
            btnPress = CheckButtonsClick(&PauseMenu, p->Input.MouseX, p->Input.MouseY);
            tbPress = CheckTextBoxes(&PauseMenu, p->Input.MouseX, p->Input.MouseY);
        }
        
        if (btnPress == Menu)
        {
            SetCursorMode(&p->Input, Arrow);
            GameState->Menu = 1;
            return;
        }
        
        if (CheckButtonsHover(&PauseMenu, p->Input.MouseX, p->Input.MouseY))
        {
            SetCursorMode(&p->Input, Hand);
        }
        else
        {
            SetCursorMode(&p->Input, Arrow);
        }
        
        platform_controller_input *Controller = &p->Input.Controllers[0];
        if (Controller->Escape.NewEndedDown)
        {
            GameState->Menu = 0;
        }
        
        BeginMode2D(C);
        UpdateGUI(&PauseMenu, p->Dimension.Width, p->Dimension.Height);
        ClearScreen();
        RenderGUI(&PauseMenu);
        EndMode2D();
    }
}
