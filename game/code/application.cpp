#include "qlib/qlib.cpp"

#include "snake.h"

Image test;
Texture TexTest;
Texture Background;
Texture Grid;
Texture Rocks;

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
        DrawRect(Buffer->Buffer[i].Coords, Buffer->Buffer[i].Size, Buffer->Buffer[i].Tex,
                 Buffer->Buffer[i].Rotation, Buffer->Buffer[i].Mode);
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
            Rect GridRect = 
            {
                v3((real32)(GridX + (i * GridSize)), (real32)(GridY + (j * GridSize)), -0.1f),
                v2(GridSize, GridSize),
                Grid,
                0,
                BlendMode::gl_src_alpha,
            };
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
        Rect NodeRect =
        {
            v3(roundf(GridX + ((Node->Coords.x) * GridSize)),
               roundf(GridY + ((Node->Coords.y) * GridSize)),
               0.5f),
            v2(GridSize, GridSize),
            SnakeStraight,
            0.0f,
            BlendMode::gl_one,
        };
        
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
        
        if (Left->Coords.y == Right->Coords.y)
            Coords.x += (GridSize/2);
        if (Left->Coords.x == Right->Coords.x)
            Coords.y += (GridSize/2);
        
        Rect NodeRect =
        {
            v3(roundf(Coords.x),
               roundf(Coords.y),
               0.5f),
            Size,
            SnakeStraight,
            0.0f,
            BlendMode::gl_one,
        };
        
        if (Left->Coords.y != Right->Coords.y || Left->Coords.x != Right->Coords.x) {
            RBuffer.Push(NodeRect);
            
            NodeRect.Coords.z = 0.0f;
            NodeRect.Tex = StraightOutline;
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

internal void
MoveCoffeeCow(CoffeeCow *Cow, real32 SecondsElapsed)
{
    real32 Dm = (Cow->Speed * SecondsElapsed);
    real32 TransitionAmt = Cow->TransitionAmt + Dm;
    
    CoffeeCowNode* Node = (CoffeeCowNode*)Cow->Nodes[0];
    if (!CheckMove(Node->Coords.x, Node->Coords.y, Cow->Direction)) return;
    
    if (TransitionAmt > 1) {
        if (!DetermineNextDirectionCoffeeCow(Cow))
            return;
        
        Dm = TransitionAmt - 1;
        Cow->TransitionAmt = Dm;
    } 
    else {
        Cow->TransitionAmt += Dm;
    }
}

internal void
AddCoffeeCowNode(CoffeeCow *Cow, int X, int Y)
{
    CoffeeCowNode NewNode = {};
    NewNode.Coords = v2(X, Y);
    NewNode.CurrentDirection = DOWN;
    NewNode.NextDirection = DOWN;
    
    Cow->Nodes.Push((void*)&NewNode);
}

Snake Player = {};
CoffeeCow CowPlayer = {};

Camera C =
{
    v3(0, 0, -900),
    v3(0, 0, 0),
    v3(0, 1, 0),
    0,
    0,
    0,
};

real32 Rotation = 100;

global_variable GUI MainMenu = {};
global_variable GUI PauseMenu = {};

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
        
        Image ImgGrid = {};
        Image ImgBackground = {};
        Image ImgRocks = {};
        
        Image ImgSnakeHead = {};
        Image ImgSnakeStraight = {};
        Image ImgSnakeCorner = {};
        
        Image ImgHeadOutline = {};
        Image ImgStraightOutline = {};
        Image ImgCornerOutline = {};
        
#if SAVE_IMAGES
        test = LoadImage("grass2.png");
        test = ResizeImage(test, GRIDWIDTH * GRIDSIZE, GRIDHEIGHT * GRIDSIZE);
        //SaveImage(&test, "grass2.h");
        
        ImgGrid = LoadImage("grid.png");
        ImgGrid = ResizeImage(ImgGrid, GRIDSIZE, GRIDSIZE);
        
        ImgSnakeHead = LoadImage("cowhead.png");
        ImgSnakeHead = ResizeImage(ImgSnakeHead, GRIDSIZE, GRIDSIZE);
        ImgHeadOutline= LoadImage("cowheadoutline.png");
        ImgHeadOutline= ResizeImage(ImgHeadOutline, GRIDSIZE, GRIDSIZE);
        
        ImgSnakeStraight = LoadImage("straight.png");
        ImgSnakeStraight = ResizeImage(ImgSnakeStraight, GRIDSIZE, GRIDSIZE);
        ImgStraightOutline= LoadImage("straightoutline.png");
        ImgStraightOutline = ResizeImage(ImgStraightOutline, GRIDSIZE, GRIDSIZE);
        
        ImgSnakeCorner = LoadImage("circle.png");
        ImgSnakeCorner = ResizeImage(ImgSnakeCorner, GRIDSIZE, GRIDSIZE);
        ImgCornerOutline = LoadImage("circleoutline.png");
        ImgCornerOutline = ResizeImage(ImgCornerOutline, GRIDSIZE, GRIDSIZE);
        
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
        HeadOutline.Init(&ImgHeadOutline);
        
        SnakeStraight.Init(&ImgSnakeStraight);
        StraightOutline.Init(&ImgStraightOutline);
        
        SnakeCorner.Init(&ImgSnakeCorner);
        CornerOutline.Init(&ImgCornerOutline);
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
        if (CowPlayer.Initialized == false)
        {
            CowPlayer.Nodes.Init(GRIDWIDTH * GRIDHEIGHT, sizeof(CoffeeCowNode));
            AddCoffeeCowNode(&CowPlayer, 1, 4);
            AddCoffeeCowNode(&CowPlayer, 1, 3);
            AddCoffeeCowNode(&CowPlayer, 1, 2);
            AddCoffeeCowNode(&CowPlayer, 1, 1);
            CowPlayer.Inputs.Init(3, sizeof(int));
            CowPlayer.Speed = 7; // m/s
            CowPlayer.Direction = DOWN;
            CowPlayer.Initialized = true;
        }
        
        platform_controller_input *Controller = &p->Input.Controllers[0];
        
        if (Controller->Escape.NewEndedDown)
            GameState->Menu = 2;
        
        if(Controller->MoveLeft.NewEndedDown)
            AddInput(&CowPlayer, LEFT);
        if(Controller->MoveRight.NewEndedDown)
            AddInput(&CowPlayer, RIGHT);
        if(Controller->MoveDown.NewEndedDown)
            AddInput(&CowPlayer, DOWN);
        if(Controller->MoveUp.NewEndedDown)
            AddInput(&CowPlayer, UP);
        
        if(Controller->ActionUp.EndedDown)
            Rotation += 10.0f;
        if(Controller->ActionDown.EndedDown)
            Rotation -= 10.0f;
        
        MoveCoffeeCow(&CowPlayer, p->Input.WorkSecondsElapsed);
        
        BeginMode2D(C);
        
        RBuffer.Push(Rect(v3(-(real32)HalfGridX - (GRIDSIZE * GRIDWIDTH * 1/6),
                             -(real32)HalfGridY - (GRIDSIZE * GRIDHEIGHT * 1/6), 1.0f),
                          v2((real32)Rocks.mWidth, (real32)Rocks.mHeight),
                          Rocks, 0, BlendMode::gl_src_alpha));
        RBuffer.Push(Rect(v3(-((real32)p->Dimension.Width)/2 - 5, 
                             -((real32)p->Dimension.Height)/2 - 5, -1.0f),
                          v2(p->Dimension.Width + 5, p->Dimension.Height + 5), Background, 0, BlendMode::gl_src_alpha));
        
        RenderGrid(-HalfGridX, -HalfGridY, GRIDWIDTH, GRIDHEIGHT, GRIDSIZE);
        DrawCoffeeCow(&CowPlayer, -HalfGridX, -HalfGridY, GRIDSIZE);
        RenderBuffer(&RBuffer);
        
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