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
Texture CoffeeStreak;

Texture HeadOutline;
Texture StraightOutline;
Texture CornerOutline;

Font Faune50 = {};
Font Faune100 = {};
Font Faune = {};

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
RenderGrid(real32 GridX, real32 GridY, 
           int GridWidth, int GridHeight, int GridSize)
{
    for (int i = 0; i < GridWidth; i++) {
        for (int j = 0; j < GridHeight; j++) {
            v3 Coords = v3((real32)(GridX + (i * GridSize)),
                           (real32)(GridY + (j * GridSize)), 
                           -0.1f);
            Push(RenderGroup, Coords, v2(GridSize, GridSize), 
                 Grid, 0, BlendMode::gl_src_alpha);
        }
    }
}

internal void
DrawCoffee(Coffee *Cof, real32 GridX, real32 GridY, int GridSize)
{
    v3 Coords = v3(roundf(GridX + ((Cof->Coords.x) * GridSize)),
                   roundf(GridY + ((Cof->Coords.y) * GridSize)),
                   -0.1f);
    Push(RenderGroup, Coords, v2(GridSize, GridSize), CoffeeTex, Cof->Rotation,
         BlendMode::gl_src_alpha);
}

internal void
DrawCoffeeCowNodes(CoffeeCow *Cow, real32 GridX, real32 GridY, int GridSize)
{
    for (int i = 0; i < Cow->Nodes.Size; i++) {
        CoffeeCowNode *Node = (CoffeeCowNode*)Cow->Nodes[i];
        v3 Coords = v3(roundf(GridX + ((Node->Coords.x) * GridSize)),
                       roundf(GridY + ((Node->Coords.y) * GridSize)),
                       0.5f);
        Push(RenderGroup, Coords, v2(GridSize, GridSize), SnakeStraight, 0.0f,
             BlendMode::gl_one);
    }
}

inline v2
GetSize(v3 Left, v3 Right, real32 GridSize)
{
    v2 Size;
    Size.x = (Left.x - Right.x) * -1;
    Size.y = (Left.y - Right.y) * -1;
    if (Size.x == 0) Size.x = GridSize;
    if (Size.y == 0) Size.y = GridSize;
    
    return Size;
}

// with down being the default direction
internal real32
GetRotation(int Direction)
{
    real32 Rotation = 0;
    if (Direction == LEFT)
        Rotation = 90.0f;
    if (Direction == RIGHT)
        Rotation = 270.0f;
    if (Direction == UP)
        Rotation = 180.0f;
    if (Direction == DOWN)
        Rotation = 0.0f;
    
    return Rotation;
}

internal real32
GetAngleDiff(int StartDirection, int EndDirection)
{
    real32 Goal;
    real32 BRotation = GetRotation(StartDirection);
    real32 ERotation = GetRotation(EndDirection);
    if (StartDirection == DOWN && EndDirection == RIGHT) {
        Goal = -90;
    }
    else if (StartDirection == RIGHT && EndDirection == DOWN) {
        Goal = 90;
    }
    else {
        Goal = ERotation - BRotation;
    }
    return Goal;
}

internal void
DrawCoffeeCow(CoffeeCow *Cow, real32 GridX, real32 GridY, int GridSize)
{
    // Put the head, tail, and corners into a list
    Arr Corners = {};
    Corners.Init(Cow->Nodes.Size, sizeof(v3));
    
    for (int i = 0; i < Cow->Nodes.Size; i++)
    {
        CoffeeCowNode *Node = (CoffeeCowNode*)Cow->Nodes[i];
        real32 OutlineZ = 0.0f;
        
        v3 Coords = v3(Node->Coords, 0.0f);
        real32 TransitionAmt = Cow->TransitionAmt;
        if (Node->CurrentDirection == RIGHT) {
            Coords.x = roundf(GridX + ((Coords.x + TransitionAmt)* GridSize));
            Coords.y = roundf(GridY + (Coords.y * GridSize));
        }
        else if (Node->CurrentDirection == UP) {
            Coords.x = roundf(GridX + (Coords.x * GridSize));
            Coords.y = roundf(GridY + ((Coords.y - TransitionAmt) * GridSize));
        }
        else if (Node->CurrentDirection == LEFT) {
            Coords.x = roundf(GridX + ((Coords.x - TransitionAmt) * GridSize));
            Coords.y = roundf(GridY + (Coords.y * GridSize));
        }
        else if (Node->CurrentDirection == DOWN) {
            Coords.x = roundf(GridX + (Coords.x * GridSize));
            Coords.y = roundf(GridY + ((Coords.y + TransitionAmt) * GridSize));
        }
        
        if (i == 0)
        {
            Coords.z = 2.0f;
            Push(RenderGroup, Coords,
                 v2(GridSize, GridSize), SnakeHead, 
                 GetRotation(Node->CurrentDirection), 
                 BlendMode::gl_one);
            Corners.Push(&Coords);
            
            Coords.z = OutlineZ;
            Push(RenderGroup, Coords, v2(GridSize, GridSize), 
                 HeadOutline, GetRotation(Node->CurrentDirection), 
                 BlendMode::gl_one);
        }
        if (i == Cow->Nodes.Size - 1) {
            Coords.z = 0.5f;
            Push(RenderGroup, Coords, v2(GridSize, GridSize), 
                 SnakeCorner, 0.0f, BlendMode::gl_one);
            Corners.Push(&Coords);
            
            Coords.z = OutlineZ;
            Push(RenderGroup, Coords, v2(GridSize, GridSize), 
                 CornerOutline, 0.0f, BlendMode::gl_one);
        }
        else if (i != Cow->Nodes.Size - 1) {
            CoffeeCowNode *NextNode = (CoffeeCowNode*)Cow->Nodes[i + 1];
            if (NextNode->CurrentDirection != Node->CurrentDirection) {
                v3 Temp = v3(roundf(GridX + ((Node->Coords.x) * GridSize)),
                             roundf(GridY + ((Node->Coords.y) * GridSize)),
                             0.5f);
                Push(RenderGroup, Temp, v2(GridSize, GridSize), 
                     SnakeCorner, 0.0f, BlendMode::gl_one);
                Corners.Push(&Temp);
                
                Temp.z = OutlineZ;
                Push(RenderGroup, Temp, v2(GridSize, GridSize), 
                     CornerOutline, 0.0f, BlendMode::gl_one);
            }
        }
        
        if (Node->Streak) {
            Coords.z = 3.0f;
            real32 Rotation = 0.0f;
            
            CoffeeCowNode *NextNode = (CoffeeCowNode*)Cow->Nodes[i + 1];
            CoffeeCowNode *PreviousNode = (CoffeeCowNode*)Cow->Nodes[i - 1];
            
            bool32 MovingIntoCorner = (PreviousNode->CurrentDirection != Node->CurrentDirection);
            bool32 MovingOutOfCorner = (NextNode->CurrentDirection != Node->CurrentDirection);
            
            int Dir = Node->CurrentDirection;
            int PDir = PreviousNode->CurrentDirection;
            int NDir = NextNode->CurrentDirection;
            
            if (MovingIntoCorner && !MovingOutOfCorner) {
                real32 Goal = GetAngleDiff(Dir, PDir);
                real32 Transition = Cow->TransitionAmt - 0.5f;
                if (Transition < 0)
                    Transition = 0;
                else
                    Transition *= 2;
                Rotation = (Transition * (Goal/2)) +
                    GetRotation(Dir);
            }
            else if (MovingOutOfCorner && !MovingIntoCorner) {
                real32 Goal = GetAngleDiff(NDir, Dir);
                real32 Transition = Cow->TransitionAmt;
                if (Transition > 0.5f) {
                    Transition = 0;
                    Rotation = GetRotation(Dir);
                }
                else {
                    Transition *= 2;
                    Rotation = (Transition * (Goal/2)) +
                        GetRotation(NDir) + (Goal/2);
                }
            }
            else if (MovingIntoCorner && MovingOutOfCorner) {
                real32 Goal = 0;
                real32 Transition = Cow->TransitionAmt;
                if (Transition > 0.5f) {
                    Transition = Cow->TransitionAmt - 0.5f;
                    Transition *= 2;
                    Goal = GetAngleDiff(Dir, PDir);
                    Rotation = (Transition * (Goal/2)) + GetRotation(Dir);
                }
                else {
                    Transition *= 2;
                    Goal = GetAngleDiff(NDir, Dir);
                    Rotation = (Transition * (Goal/2)) +
                        GetRotation(NDir) + (Goal/2);
                }
            }
            else
                Rotation = GetRotation(Node->CurrentDirection);
            
            real32 OutlineFactor = 0.96f;
            real32 Outline = (real32)(GridSize - (GridSize * OutlineFactor));
            
            Coords.x += Outline;
            Coords.y += Outline;
            Coords.z = 3.0f;
            
            Push(RenderGroup, Coords, 
                 v2(GridSize - (Outline*2), GridSize - (Outline*2)), 
                 CoffeeStreak, Rotation, BlendMode::gl_one);
        }
    }
    
    // Draw the connect rects
    for (int i = 0; i < (Corners.Size - 1); i++)
    {
        v3 *Node = (v3*)Corners[i];
        v3 *NextNode = (v3*)Corners[i + 1];
        v3 *Left;
        v3 *Right;
        if (Node->x < NextNode->x || Node->y < NextNode->y) {
            Left = Node;
            Right = NextNode;
        }
        else {
            Left = NextNode;
            Right = Node;
        }
        
        v3 Coords = v3(Left->x, Left->y, 0.5f);
        
        real32 OutlineFactor = 0.96f;
        real32 Outline = (real32)(GridSize - (GridSize * OutlineFactor));
        
        if (Left->y == Right->y) {
            Coords.x += (GridSize/2);
            Coords.y += Outline;
        }
        if (Left->x == Right->x) {
            Coords.y += (GridSize/2);
            Coords.x += Outline;
        }
        
        if (Left->y != Right->y || Left->x != Right->x) {
            Push(RenderGroup, Coords, 
                 GetSize(*Left, *Right, GridSize - (Outline*2)),
                 0xFFFFFFFF, 0.0f);
            
            if (Left->y == Right->y)
                Coords.y -= Outline;
            if (Left->x == Right->x) 
                Coords.x -= Outline;
            
            Coords.z = 0.0f;
            Push(RenderGroup, Coords, 
                 GetSize(*Left, *Right, (real32)GridSize), 
                 StraightOutline, 0.0f, BlendMode::gl_one);
        }
    }
    
    Corners.Del();
}

internal void
AddInput(CoffeeCow *Cow, int NewDirection)
{
    if (Cow->Inputs.Size >= 3)
        return;
    
    int OldDirection = 0;
    if (Cow->Inputs.Size == 0)
        OldDirection = Cow->Direction;
    else 
        OldDirection = *(int*)Cow->Inputs[Cow->Inputs.Size - 1];
    
    if (!(OldDirection == NewDirection) && // Same Direction
        !(OldDirection + 2 == NewDirection) && // Backwards Direction
        !(OldDirection - 2 == NewDirection))
        Cow->Inputs.Push((void*)&NewDirection);
}

internal void
MoveInDirection(v2 *Coords, int Direction)
{
    if (Direction == RIGHT)
        Coords->x += 1;
    else if (Direction == UP)
        Coords->y -= 1;
    else if (Direction == LEFT)
        Coords->x -= 1;
    else if (Direction == DOWN)
        Coords->y += 1;
}

internal bool32
MoveCoffeeCow(CoffeeCow *Cow, real32 SecondsElapsed, v2 GridDim)
{
    real32 Dm = (Cow->Speed * SecondsElapsed);
    real32 TransitionAmt = Cow->TransitionAmt + Dm;
    CoffeeCowNode *Head = (CoffeeCowNode*)Cow->Nodes[0];
    
    // Check collision with wall
    {
        v2 Coords = Head->Coords;
        int Direction = Cow->Direction;
        if ((Direction == LEFT && Coords.x <= 0) ||
            (Direction == RIGHT && Coords.x >= GridDim.x - 1) ||
            (Direction == UP && Coords.y <= 0) ||
            (Direction == DOWN && Coords.y >= GridDim.y - 1))
            return false;
    }
    
    // Check collision with self
    {
        v2 HeadCoords = v2(Head->Coords);
        MoveInDirection(&HeadCoords, Head->CurrentDirection);
        
        for (int i = 1; i < Cow->Nodes.Size; i++) {
            CoffeeCowNode* Node = (CoffeeCowNode*)Cow->Nodes[i];
            if (HeadCoords.x == Node->Coords.x &&
                HeadCoords.y == Node->Coords.y)
                return false;
        }
    }
    
    if (TransitionAmt > 1) {
        for (int i = 0; i < Cow->Nodes.Size; i++)
        {
            CoffeeCowNode* Node = (CoffeeCowNode*)Cow->Nodes[i];
            MoveInDirection(&Node->Coords, Node->CurrentDirection);
            
            if (i == 0 && Cow->Inputs.Size != 0) {
                Cow->Direction = *(int*)Cow->Inputs[0];
                Node->CurrentDirection = Cow->Direction;
                Cow->Inputs.PopFront();
            }
            else if (i != 0) {
                Node->CurrentDirection = Node->NextDirection;
                CoffeeCowNode* PreviousNode = (CoffeeCowNode*)Cow->Nodes[i-1];
                Node->NextDirection = PreviousNode->CurrentDirection;
            }
        }
        
        Dm = TransitionAmt - 1;
        Cow->TransitionAmt = Dm;
    } 
    else
        Cow->TransitionAmt += Dm;
    
    return true;
}

internal void
AddCoffeeCowNode(CoffeeCow *Cow, int X, int Y, int CDirection, int NDirection)
{
    CoffeeCowNode NewNode = {};
    NewNode.Coords = v2(X, Y);
    NewNode.CurrentDirection = CDirection;
    NewNode.NextDirection = NDirection;
    NewNode.Streak = false;
    
    if (Cow->Nodes.Size > 1) {
        CoffeeCowNode *LastTail = (CoffeeCowNode*)Cow->Nodes[Cow->Nodes.Size - 1];
        int Rand = Random(1, 2);
        if (Rand == 2)
            LastTail->Streak = true;
    }
    
    Cow->Nodes.Push((void*)&NewNode);
}

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
        
        for (int i = 0; i < 4; i++) {
            AddCoffeeCowNode(CowPlayer, Startx, Starty, Direction, Direction);
            Startx += incx;
        }
        
        CowPlayer->Inputs.Init(3, sizeof(int));
        CowPlayer->Speed = 7; // m/s
        CowPlayer->Direction = Direction;
        CowPlayer->Initialized = true;
        GameState->Collect.Initialized = false;
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
    CoffeeStreak = LoadTexture("coffeestreak.png", GridSize, GridSize);
}

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
        
        GameState->Menu = menu::main_menu;
        GameState->GridWidth = 17;
        GameState->GridHeight = 17;
        
        /*
        Client client = {};
        client.create("127.0.0.1", "44575", TCP);
        char Buffer[50];
        sprintf(Buffer, "yo homey\n");
        client.sendq(Buffer, 50);
        */
        
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
    
    real32 HalfGridX = (real32)((GameState->GridWidth * GameState->GridSize) / 2);
    real32 HalfGridY = (real32)((GameState->GridHeight * GameState->GridSize) / 2);
    
    C.Dimension = p->Dimension;
    
    if (GameState->Menu == menu::game) {
        CoffeeCow *CowPlayer = &GameState->Player1;
        Coffee *Collect = &GameState->Collect;
        
        if (CowPlayer->Initialized == false)
            InitializeGame(GameMode::Singleplayer, GameState);
        
        platform_controller_input *Controller = &p->Input.Controllers[0];
        platform_keyboard_input *Keyboard = &p->Input.Keyboard;
        if (Keyboard->Escape.NewEndedDown)
            GameState->Menu = menu::pause_menu;
        
        if(Controller->MoveLeft.NewEndedDown)
            AddInput(CowPlayer, LEFT);
        if(Controller->MoveRight.NewEndedDown)
            AddInput(CowPlayer, RIGHT);
        if(Controller->MoveDown.NewEndedDown)
            AddInput(CowPlayer, DOWN);
        if(Controller->MoveUp.NewEndedDown)
            AddInput(CowPlayer, UP);
        
        if (!MoveCoffeeCow(CowPlayer, p->Input.WorkSecondsElapsed, 
                           v2(GameState->GridWidth, GameState->GridHeight)))
            GameState->Menu = menu::game_over_menu;
        
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
            bool32 ValidLocation = false;
            while (!ValidLocation) {
                Collect->Coords.x = (real32)Random(0, GameState->GridWidth - 1);
                Collect->Coords.y = (real32)Random(0, GameState->GridHeight - 1);
                ValidLocation = true;
                for (int i = 0; i < CowPlayer->Nodes.Size; i++) {
                    CoffeeCowNode* Node = (CoffeeCowNode*)CowPlayer->Nodes[i];
                    if (Node->Coords.x == Collect->Coords.x &&
                        Node->Coords.y == Collect->Coords.y)
                        ValidLocation = false;
                }
            }
            Collect->Initialized = true;
        }
        
        Push(RenderGroup, 
             v3(-HalfGridX - (GameState->GridSize * GameState->GridWidth*1/6),
                -HalfGridY - (GameState->GridSize * GameState->GridHeight*1/6), 
                1.0f),
             v2(Rocks.mWidth, Rocks.mHeight), Rocks, 0, 
             BlendMode::gl_src_alpha);
        Push(RenderGroup, 
             v3(-((real32)p->Dimension.Width)/2 - 5, 
                -((real32)p->Dimension.Height)/2 - 5, -1.0f),
             v2(p->Dimension.Width + 5, p->Dimension.Height + 5), Background, 0, BlendMode::gl_src_alpha);
        
        RenderGrid(-HalfGridX, -HalfGridY, GameState->GridWidth, GameState->GridHeight, GameState->GridSize);
        DrawCoffeeCow(CowPlayer, -HalfGridX, -HalfGridY, GameState->GridSize);
        
        Collect->Rotation += (p->Input.WorkSecondsElapsed * 100);
        if (Collect->Rotation > 360)
            Collect->Rotation -= 360;
        DrawCoffee(Collect, -HalfGridX, -HalfGridY, GameState->GridSize);
        
        Strinq Score = S() + CowPlayer->Score;
        PrintOnScreen(&Faune50, 
                      GetData(Score), 
                      v2(-p->Dimension.Width/2 + 10, -p->Dimension.Height/2 + 10),
                      0xFFFFFFFF);
        
        BeginMode2D(C);
        RenderPieceGroup(RenderGroup);
        EndMode2D();
    }
    else if (GameState->Menu == menu::main_menu) {
#include "main_menu.cpp" 
    }
    else if (GameState->Menu == menu::multiplayer_menu) {
#include "multiplayer_menu.cpp"
    }
    else if (GameState->Menu == menu::pause_menu) {
#include "pause_menu.cpp" 
    }
    else if (GameState->Menu == menu::game_over_menu) {
#include "game_over_menu.cpp" 
    }
}