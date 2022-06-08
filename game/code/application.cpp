#include "qlib/qlib.cpp"

#include "snake.h"

Texture TexTest;
Texture Background;
Texture Grid;
Texture Rocks;
Texture CoffeeTex;

Texture SnakeHead;
Texture SnakeStraight;
Texture SnakeCorner;

Texture HeadOutline;
Texture StraightOutline;
Texture CornerOutline;

Font Faune50 = {};
Font Faune100 = {};
Font Faune = {};

RectBuffer RBuffer = {};

internal void
RenderBuffer(RectBuffer *Buffer)
{
    // Insertion Sort Rects
    {
        int i = 1;
        while (i < Buffer->Size)
        {
            int j = i;
            while (j > 0 && Buffer->Buffer[j-1].Coords.z > Buffer->Buffer[j].Coords.z)
            {
                Rect Temp = Buffer->Buffer[j];
                Buffer->Buffer[j] = Buffer->Buffer[j-1];
                Buffer->Buffer[j-1] = Temp;
                j = j - 1;
            }
            i++;
        }
    }
    
    // Render Rects
    for (int i = 0; i < Buffer->Size; i++)
    {
        if (Buffer->Buffer[i].RMode == RectMode::Tex)
            DrawRect(Buffer->Buffer[i].Coords, Buffer->Buffer[i].Size, 
                     Buffer->Buffer[i].Tex,
                     Buffer->Buffer[i].Rotation, Buffer->Buffer[i].Mode);
        else if (Buffer->Buffer[i].RMode == RectMode::Color)
            DrawRect((int)Buffer->Buffer[i].Coords.x, 
                     (int)Buffer->Buffer[i].Coords.y,
                     (int)Buffer->Buffer[i].Size.x,
                     (int)Buffer->Buffer[i].Size.y,
                     Buffer->Buffer[i].Color);
    }
    
    Buffer->Clear();
};

internal void
RenderGrid(int GridX, int GridY, int GridWidth, int GridHeight, int GridSize)
{
    for (int i = 0; i < GridWidth; i++)
    {
        for (int j = 0; j < GridHeight; j++)
        {
            Rect GridRect = {};
            GridRect.Coords = v3((real32)(GridX + (i * GridSize)),
                                 (real32)(GridY + (j * GridSize)), 
                                 -0.1f);
            GridRect.Size = v2(GridSize, GridSize);
            GridRect.Tex = Grid;
            GridRect.Rotation = 0;
            GridRect.Mode = BlendMode::gl_src_alpha;
            RBuffer.Push(GridRect);
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
        return false;
    // Backwards Direction
    if (OldDirection + 2 == NewDirection || OldDirection - 2 == NewDirection)
        return false;
    
    return true;
}

internal void
DrawCoffee(Coffee *C, int GridX, int GridY, int GridSize)
{
    Rect NodeRect = {};
    NodeRect.Coords = v3(roundf(GridX + ((C->Coords.x) * GridSize)),
                         roundf(GridY + ((C->Coords.y) * GridSize)),
                         -0.1f);
    NodeRect.Size = v2(GridSize, GridSize);
    NodeRect.Tex =  CoffeeTex;
    NodeRect.Rotation = 0.0f;
    NodeRect.Mode = BlendMode::gl_one;
    
    RBuffer.Push(NodeRect);
}

internal void
DrawCoffeeCowNodes(CoffeeCow *Cow, int GridX, int GridY, int GridSize)
{
    for (int i = 0; i < Cow->Nodes.Size; i++)
    {
        CoffeeCowNode *Node = (CoffeeCowNode*)Cow->Nodes[i];
        Rect NodeRect = {};
        NodeRect.Coords = v3(roundf(GridX + ((Node->Coords.x) * GridSize)),
                             roundf(GridY + ((Node->Coords.y) * GridSize)),
                             0.5f),
        NodeRect.Size = v2(GridSize, GridSize);
        NodeRect.Tex = SnakeStraight;
        NodeRect.Rotation = 0.0f;
        NodeRect.Mode = BlendMode::gl_one;
        
        RBuffer.Push(NodeRect);
    }
}

// First figure out where all the corners are
// Connect squares to them

internal void
DrawCoffeeCow(CoffeeCow *Cow, int GridX, int GridY, int GridSize)
{
    // Put the head, tail, and corners into a list
    Arr Corners = {};
    Corners.Init(Cow->Nodes.Size, sizeof(Rect));
    
    for (int i = 0; i < Cow->Nodes.Size; i++)
    {
        CoffeeCowNode *Node = (CoffeeCowNode*)Cow->Nodes[i];
        Rect NodeRect = {};
        NodeRect.Coords = v3(roundf(GridX + ((Node->Coords.x) * GridSize)),
                             roundf(GridY + ((Node->Coords.y) * GridSize)),
                             0.5f);
        NodeRect.Size = v2(GridSize, GridSize);
        NodeRect.Tex = SnakeStraight;
        NodeRect.Rotation = 0.0f;
        NodeRect.Mode = BlendMode::gl_one;
        
        if (i == 0 || i == Cow->Nodes.Size - 1)
        {
            if (Node->CurrentDirection == RIGHT)
                NodeRect.Coords.x = roundf(GridX + ((Node->Coords.x + Cow->TransitionAmt)* GridSize));
            if (Node->CurrentDirection == UP)
                NodeRect.Coords.y = roundf(GridY + ((Node->Coords.y - Cow->TransitionAmt) * GridSize));
            if (Node->CurrentDirection == LEFT)
                NodeRect.Coords.x = roundf(GridX + ((Node->Coords.x - Cow->TransitionAmt) * GridSize));
            if (Node->CurrentDirection == DOWN)
                NodeRect.Coords.y = roundf(GridY + ((Node->Coords.y + Cow->TransitionAmt) * GridSize));
            
            if (i == 0) {
                int Rotation = 0;
                if (Node->CurrentDirection == LEFT)
                    Rotation = 90;
                if (Node->CurrentDirection == RIGHT)
                    Rotation = -90;
                if (Node->CurrentDirection == UP)
                    Rotation = 180;
                if (Node->CurrentDirection == DOWN)
                    Rotation = 0;
                NodeRect.Rotation = (real32)Rotation;
                
                NodeRect.Coords.z = 2.0f;
                NodeRect.Tex = SnakeHead;
                RBuffer.Push(NodeRect);
                Corners.Push(&NodeRect);
                
                NodeRect.Coords.z = 0.0f;
                NodeRect.Tex = HeadOutline;
                RBuffer.Push(NodeRect);
            }
        }
        if (i == Cow->Nodes.Size - 1) {
            CoffeeCowNode *PreviousNode = (CoffeeCowNode*)Cow->Nodes[i - 1];
            NodeRect.Coords.z = 0.5f;
            NodeRect.Tex = SnakeCorner;
            RBuffer.Push(NodeRect);
            Corners.Push(&NodeRect);
            
            NodeRect.Coords.z = 0.0f;
            NodeRect.Tex = CornerOutline;
            RBuffer.Push(NodeRect);
        }
        else {
            CoffeeCowNode *NextNode = (CoffeeCowNode*)Cow->Nodes[i + 1];
            if (NextNode->CurrentDirection != Node->CurrentDirection) {
                NodeRect =
                {
                    v3(roundf(GridX + ((Node->Coords.x) * GridSize)),
                       roundf(GridY + ((Node->Coords.y) * GridSize)),
                       0.5f),
                    v2(GridSize, GridSize),
                    SnakeCorner,
                    0.0f,
                    BlendMode::gl_one
                };
                RBuffer.Push(NodeRect);
                Corners.Push(&NodeRect);
                
                NodeRect.Coords.z = 0.0f;
                NodeRect.Tex = CornerOutline;
                RBuffer.Push(NodeRect);
            }
        }
    }
    
    // Draw the connect rects
    for (int i = 0; i < (Corners.Size - 1); i++)
    {
        Rect *Node = (Rect*)Corners[i];
        Rect *NextNode = (Rect*)Corners[i + 1];
        Rect *Left;
        Rect *Right;
        if (Node->Coords.x < NextNode->Coords.x || Node->Coords.y < NextNode->Coords.y) {
            Left = Node;
            Right = NextNode;
        }
        else {
            Left = NextNode;
            Right = Node;
        }
        
        v3 Coords;
        v2 Size;
        Coords.x = Left->Coords.x;
        Coords.y = Left->Coords.y;
        
        Size.x = (Left->Coords.x - Right->Coords.x) * -1;
        Size.y = (Left->Coords.y - Right->Coords.y) * -1;
        
        if (Size.x == 0) Size.x = (real32)GridSize;
        if (Size.y == 0) Size.y = (real32)GridSize;
        
        real32 OutlineFactor = 0.925f;
        real32 Outline = (real32)(GridSize - (GridSize * OutlineFactor));
        Size.x -= Outline;
        Size.y -= Outline;
        
        if (Left->Coords.y == Right->Coords.y)
            Coords.x += (GridSize/2);
        if (Left->Coords.x == Right->Coords.x)
            Coords.y += (GridSize/2);
        
        Rect NodeRect = {};
        NodeRect.Coords = v3(roundf(Coords.x) + (Outline/2),
                             roundf(Coords.y) + (Outline/2),
                             0.5f);
        NodeRect.Size = Size;
        NodeRect.Color = 0xFFFFFFFF;
        NodeRect.Rotation = 0.0f;
        NodeRect.RMode = RectMode::Color;
        
        if (Left->Coords.y != Right->Coords.y || Left->Coords.x != Right->Coords.x) {
            RBuffer.Push(NodeRect);
            NodeRect.Coords = v3(roundf(Coords.x),
                                 roundf(Coords.y),
                                 0.5f);
            NodeRect.Coords.z = 0.0f;
            Size.x += Outline;
            Size.y += Outline;
            NodeRect.Size = Size;
            NodeRect.Tex = StraightOutline;
            NodeRect.RMode = RectMode::Tex;
            RBuffer.Push(NodeRect);
        }
    }
    
    Corners.Del();
}

internal void
AddInput(CoffeeCow *Cow, int Input)
{
    if (Cow->Inputs.Size < 3)
    {
        int LastDirection = 0;
        if (Cow->Inputs.Size == 0)
            LastDirection = Cow->Direction;
        else 
            LastDirection = *(int*)Cow->Inputs[Cow->Inputs.Size - 1];
        
        if (ValidDirection(LastDirection, Input))
        {
            Cow->Inputs.Push((void*)&Input);
        }
    }
}

internal bool32
CheckMove(real32 x, real32 y, int Direction)
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

internal bool32
DetermineNextDirectionCoffeeCow(CoffeeCow *Cow)
{
    for (int i = 0; i < Cow->Nodes.Size; i++)
    {
        CoffeeCowNode* Node = (CoffeeCowNode*)Cow->Nodes[i];
        if (Node->CurrentDirection == RIGHT)
            Node->Coords.x += 1;
        if (Node->CurrentDirection == UP)
            Node->Coords.y -= 1;
        if (Node->CurrentDirection == LEFT)
            Node->Coords.x -= 1;
        if (Node->CurrentDirection == DOWN)
            Node->Coords.y += 1;
        
        if (i == 0 && Cow->Inputs.Size != 0) {
            Cow->Direction = *(int*)Cow->Inputs[0];
            Node->CurrentDirection = Cow->Direction;
            Cow->Inputs.PopFront();
        }
        else if (i != 0) {
            Node->CurrentDirection = Node->NextDirection;
            CoffeeCowNode* PreviousNode = (CoffeeCowNode*)Cow->Nodes[i - 1];
            Node->NextDirection = PreviousNode->CurrentDirection;
        }
        
        //if (i == 0 && !CheckMove(Node->Coords.x, Node->Coords.y, Cow->Direction)) return false;
    }
    
    return true;
}

internal bool32
CheckCollision(CoffeeCow *Cow)
{
    CoffeeCowNode *Head = (CoffeeCowNode*)Cow->Nodes[0];
    
    int HeadX = (int)Head->Coords.x;
    int HeadY = (int)Head->Coords.y;
    if (Head->CurrentDirection == RIGHT)
        HeadX += 1;
    if (Head->CurrentDirection == UP)
        HeadY -= 1;
    if (Head->CurrentDirection == LEFT)
        HeadX -= 1;
    if (Head->CurrentDirection == DOWN)
        HeadY += 1;
    
    for (int i = 1; i < Cow->Nodes.Size; i++) {
        CoffeeCowNode* Node = (CoffeeCowNode*)Cow->Nodes[i];
        if (HeadX == Node->Coords.x &&
            HeadY == Node->Coords.y)
            return false;
    }
    
    return true;
}

internal bool32
MoveCoffeeCow(CoffeeCow *Cow, real32 SecondsElapsed)
{
    real32 Dm = (Cow->Speed * SecondsElapsed);
    real32 TransitionAmt = Cow->TransitionAmt + Dm;
    
    CoffeeCowNode* Node = (CoffeeCowNode*)Cow->Nodes[0];
    if (!CheckMove(Node->Coords.x, Node->Coords.y, Cow->Direction)) return false;
    if (!CheckCollision(Cow)) return false;
    if (TransitionAmt > 1) {
        if (!DetermineNextDirectionCoffeeCow(Cow))
            return false;
        Dm = TransitionAmt - 1;
        Cow->TransitionAmt = Dm;
    } 
    else {
        Cow->TransitionAmt += Dm;
    }
    
    return true;
}

internal void
AddCoffeeCowNode(CoffeeCow *Cow, int X, int Y, int CDirection, int NDirection)
{
    CoffeeCowNode NewNode = {};
    NewNode.Coords = v2(X, Y);
    NewNode.CurrentDirection = CDirection;
    NewNode.NextDirection = NDirection;
    
    Cow->Nodes.Push((void*)&NewNode);
}

//CoffeeCow CowPlayer = {};

Camera C =
{
    v3(0, 0, -900),
    v3(0, 0, 0),
    v3(0, 1, 0),
    0,
    0,
    0,
};

internal void
InitializeGame(GameMode Mode, game_state *GameState)
{
    if (Mode == GameMode::Singleplayer) {
        CoffeeCow *CowPlayer = &GameState->Player1;
        memset(CowPlayer, 0, sizeof(CoffeeCow));
        CowPlayer->Nodes.Init(GameState->GridWidth * GameState->GridHeight, sizeof(CoffeeCowNode));
        int Startx = Random(1, GameState->GridWidth - 1);
        int Starty = Random(1, GameState->GridHeight - 1);
        int incx = 0;
        int incy = 0;
        
        int Direction = 0;
        int HalfWidth = GameState->GridWidth/2;
        int HalfHeight = GameState->GridHeight/2;
        if (Startx < HalfWidth)
            Direction = RIGHT;
        else if (Startx >= HalfWidth)
            Direction = LEFT;
        
        if (Direction == RIGHT)
            incx = -1;
        else if (Direction == LEFT)
            incx = 1;
        
        if (Startx < 4)
            Startx = 4;
        else if (Startx > GameState->GridWidth - 4)
            Startx =  GameState->GridWidth - 4;
        
        //sPrintqDebug(S() + "Startx: " + Startx + " Starty: "+ Starty + "\n");
        
        for (int i = 0; i < 4; i++) {
            AddCoffeeCowNode(CowPlayer, Startx, Starty, Direction, Direction);
            Startx += incx;
        }
        
        CowPlayer->Inputs.Init(3, sizeof(int));
        CowPlayer->Speed = 7; // m/s
        CowPlayer->Direction = Direction;
        CowPlayer->Initialized = true;
    }
}

internal void
LoadGameAssets(int GridSize, int GridWidth, int GridHeight)
{
    TexTest = LoadTexture("grass2.png", GridSize, GridSize);
    Grid = LoadTexture("grid.png", GridSize, GridSize);
    Background.Init("sand.png");
    Rocks = LoadTexture("rocks.png", 
                        (GridWidth * GridSize) * 4/3 - 1,
                        (GridHeight * GridSize) * 4/3 - 1);
    
    SnakeHead = LoadTexture("cowhead.png", GridSize, GridSize);
    HeadOutline = LoadTexture("cowheadoutline.png", GridSize, GridSize);
    SnakeStraight = LoadTexture("straight.png", GridSize, GridSize);
    StraightOutline = LoadTexture("straightoutline.png", GridSize, GridSize);
    SnakeCorner = LoadTexture("circle.png", GridSize, GridSize);
    CornerOutline = LoadTexture("circleoutline.png", GridSize, GridSize);
    CoffeeTex = LoadTexture("coffee.png", GridSize, GridSize);
}

real32 Rotation = 100;

global_variable GUI MainMenu = {};
global_variable GUI PauseMenu = {};
global_variable GUI GameOverMenu = {};

//#include "../data/imagesaves/grass2.h"
//#include "../data/imagesaves/grass2.

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
        GameState->GridWidth = 17;
        GameState->GridHeight = 17;
        //GameState->GridSize = p->Dimension.Height / (GameState->GridWidth + 1);
        
        Client client = {};
        client.create("127.0.0.1", "44575", TCP);
        
        char Buffer[50];
        sprintf(Buffer, "yo homey\n");
        client.sendq(Buffer, 50);
        
#if SAVE_IMAGES
        Faune50 = LoadFont("Rubik-Medium.ttf", 50);
        Faune100 = LoadFont("Rubik-Medium.ttf", 100);
        Faune = LoadFont("Rubik-Medium.ttf", 350);
#else
        LoadImageFromgrass2_h(&test);
#endif
    }
    
    int NewGridSize = p->Dimension.Height / (GameState->GridWidth + 5);
    if (NewGridSize != GameState->GridSize) {
        GameState->GridSize = NewGridSize;
        LoadGameAssets(GameState->GridSize,GameState->GridWidth, GameState->GridHeight);
    }
    
    float FPS = 0;
    if (p->Input.dt != 0)
    {
        FPS = 1 / p->Input.WorkSecondsElapsed;
        //PrintqDebug(S() + "FPS: " + (int)FPS + "\n");
    }
    
    
    int HalfGridX = (GameState->GridWidth * GameState->GridSize) / 2;
    int HalfGridY = (GameState->GridHeight * GameState->GridSize) / 2;
    
    C.Dimension = p->Dimension;
    
    if (GameState->Menu == 1)
    {
        int btnPress = -1;
        int tbPress = -1;
        
        if(p->Input.MouseButtons[0].EndedDown) {
            btnPress = CheckButtonsClick(&MainMenu, p->Input.MouseX, p->Input.MouseY);
            tbPress = CheckTextBoxes(&MainMenu, p->Input.MouseX, p->Input.MouseY);
        }
        
        if (btnPress == GameStart) {
            SetCursorMode(&p->Input, Arrow);
            GameState->Menu = 0;
            return;
        }
        else if (btnPress == Btn4)
        {
            //createClient(&client, GetTextBoxText(&MainMenu, IP), GetTextBoxText(&MainMenu, PORT), TCP);
        }
        else if (btnPress == Quit)
            p->Input.Quit = 1;
        
        if (MainMenu.Initialized == 0) {
#include "main_menu.cpp" 
            MainMenu.Initialized = 1;
        }
        
        if (CheckButtonsHover(&MainMenu, p->Input.MouseX, p->Input.MouseY))
            SetCursorMode(&p->Input, Hand);
        else
            SetCursorMode(&p->Input, Arrow);
        
        BeginMode2D(C);
        UpdateGUI(&MainMenu, p->Dimension.Width, p->Dimension.Height);
        ClearScreen();
        RenderGUI(&MainMenu);
        EndMode2D();
    }
    else if (GameState->Menu == 0)
    {
        CoffeeCow *CowPlayer = &GameState->Player1;
        Coffee *Collect = &GameState->Collect;
        
        if (CowPlayer->Initialized == false)
            InitializeGame(GameMode::Singleplayer, GameState);
        
        platform_controller_input *Controller = &p->Input.Controllers[0];
        
        if (Controller->Escape.NewEndedDown)
            GameState->Menu = 2;
        
        if(Controller->MoveLeft.NewEndedDown)
            AddInput(CowPlayer, LEFT);
        if(Controller->MoveRight.NewEndedDown)
            AddInput(CowPlayer, RIGHT);
        if(Controller->MoveDown.NewEndedDown)
            AddInput(CowPlayer, DOWN);
        if(Controller->MoveUp.NewEndedDown)
            AddInput(CowPlayer, UP);
        
        if(Controller->ActionUp.EndedDown)
            Rotation += 10.0f;
        if(Controller->ActionDown.EndedDown)
            Rotation -= 10.0f;
        
        if (!MoveCoffeeCow(CowPlayer, p->Input.WorkSecondsElapsed))
            GameState->Menu = 3;
        
        CoffeeCowNode* Head = (CoffeeCowNode*)CowPlayer->Nodes[0];
        if (Head->Coords.x == Collect->Coords.x &&
            Head->Coords.y == Collect->Coords.y) {
            CowPlayer->Score++;
            Collect->Initialized = false;
            
            // Make Cow Longer
            CoffeeCowNode* Tail = (CoffeeCowNode*)CowPlayer->Nodes[CowPlayer->Nodes.Size-1];
            int NewX = (int)Tail->Coords.x;
            int NewY = (int)Tail->Coords.y;
            
            if (Tail->CurrentDirection == UP)
                NewY++;
            else if (Tail->CurrentDirection == LEFT)
                NewX++;
            else if (Tail->CurrentDirection == DOWN)
                NewY--;
            else if (Tail->CurrentDirection == RIGHT)
                NewX--;
            
            AddCoffeeCowNode(CowPlayer, NewX, NewY, Tail->CurrentDirection, Tail->CurrentDirection);
        }
        
        if (Collect->Initialized == false) {
            Collect->Coords.x = (real32)Random(0, GameState->GridWidth - 1);
            Collect->Coords.y = (real32)Random(0, GameState->GridHeight - 1);
            Collect->Initialized = true;
        }
        
        BeginMode2D(C);
        RBuffer.Push(Rect(v3(-(real32)HalfGridX - (GameState->GridSize * GameState->GridWidth * 1/6),
                             -(real32)HalfGridY - (GameState->GridSize * GameState->GridHeight * 1/6), 1.0f),
                          v2((real32)Rocks.mWidth, (real32)Rocks.mHeight),
                          Rocks, 0, BlendMode::gl_src_alpha));
        RBuffer.Push(Rect(v3(-((real32)p->Dimension.Width)/2 - 5, 
                             -((real32)p->Dimension.Height)/2 - 5, -1.0f),
                          v2(p->Dimension.Width + 5, p->Dimension.Height + 5), Background, 0, BlendMode::gl_src_alpha));
        RenderGrid(-HalfGridX, -HalfGridY, GameState->GridWidth, GameState->GridHeight, GameState->GridSize);
        DrawCoffeeCow(CowPlayer, -HalfGridX, -HalfGridY, GameState->GridSize);
        DrawCoffee(Collect, -HalfGridX, -HalfGridY, GameState->GridSize);
        RenderBuffer(&RBuffer);
        
        Strinq Score = S() + CowPlayer->Score;
        PrintOnScreen(&Faune50, 
                      GetData(Score), 
                      -p->Dimension.Width/2 + 10,
                      -p->Dimension.Height/2 + 10,
                      0xFFFFFFFF);
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
        
        if (btnPress == Menu) {
            SetCursorMode(&p->Input, Arrow);
            memset(&GameState->Player1, 0, sizeof(CoffeeCow));
            GameState->Menu = 1;
            return;
        }
        else if (btnPress == Reset) {
            GameState->Menu = 0;
            InitializeGame(GameMode::Singleplayer, GameState);
        }
        
        if (CheckButtonsHover(&PauseMenu, p->Input.MouseX, p->Input.MouseY))
            SetCursorMode(&p->Input, Hand);
        else
            SetCursorMode(&p->Input, Arrow);
        
        platform_controller_input *Controller = &p->Input.Controllers[0];
        
        if (Controller->Escape.NewEndedDown)
            GameState->Menu = 0;
        
        BeginMode2D(C);
        UpdateGUI(&PauseMenu, p->Dimension.Width, p->Dimension.Height);
        ClearScreen();
        RenderGUI(&PauseMenu);
        EndMode2D();
    }
    else if (GameState->Menu == 3)
    {
        if (GameOverMenu.Initialized == 0)
        {
#include "game_over_menu.cpp" 
            GameOverMenu.Initialized = 1;
        }
        
        int btnPress = -1;
        int tbPress = -1;
        
        if(p->Input.MouseButtons[0].EndedDown)
        {
            btnPress = CheckButtonsClick(&GameOverMenu, p->Input.MouseX, p->Input.MouseY);
            tbPress = CheckTextBoxes(&GameOverMenu, p->Input.MouseX, p->Input.MouseY);
        }
        
        if (btnPress == Menu) {
            SetCursorMode(&p->Input, Arrow);
            memset(&GameState->Player1, 0, sizeof(CoffeeCow));
            GameState->Menu = 1;
            return;
        }
        else if (btnPress == Reset) {
            GameState->Menu = 0;
            InitializeGame(GameMode::Singleplayer, GameState);
        }
        
        if (CheckButtonsHover(&GameOverMenu, p->Input.MouseX, p->Input.MouseY))
            SetCursorMode(&p->Input, Hand);
        else
            SetCursorMode(&p->Input, Arrow);
        
        platform_controller_input *Controller = &p->Input.Controllers[0];
        
        if (Controller->Escape.NewEndedDown)
            GameState->Menu = 0;
        
        BeginMode2D(C);
        UpdateGUI(&GameOverMenu, p->Dimension.Width, p->Dimension.Height);
        ClearScreen();
        RenderGUI(&GameOverMenu);
        EndMode2D();
    }
}