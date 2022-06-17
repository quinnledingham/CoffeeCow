#include "qlib/qlib.cpp"
#include "coffee_cow.h"
#include "snake.h"

/*
DWORD WINAPI
RecvPlayers(LPVOID lpParam)
{
    thread_param *tp = (thread_param*)lpParam;
    CoffeeCow* CowPlayer2 = tp->Cow;
    HANDLE *p2Mutex = tp->p2Mutex;
    
    char Buffer[1000];
    
    Client client = {};
    client.create(tp->IP, tp->Port, TCP);
    packet Packet = {};
    Packet.Type = connection_type::receiver;
    memset(Buffer, 0, BUF_SIZE);
    memcpy(Buffer, &Packet, sizeof(packet)); 
    client.sendq(Buffer, SEND_BUFFER_SIZE);
    
    // Receive other snake
    
    while(1) {
        memset(Buffer, 0, BUF_SIZE);
        client.recvq(Buffer, BUF_SIZE);
        game_packet *Recv = (game_packet*)Buffer; 
        ServerCoffeeCow *Cow = &Recv->Cow;
        
        switch(WaitForSingleObject(p2Mutex, INFINITE))
        {
            case WAIT_OBJECT_0: _try 
            {
                CowPlayer2->TransitionAmt = Cow->TransitionAmt;
                CowPlayer2->Score = Cow->Score;
                CowPlayer2->Nodes.Clear();
            }
            _finally{if(!ReleaseMutex(p2Mutex)){}}break;case WAIT_ABANDONED:return false;
        }
        
        for (int i = 0; i < Cow->NumOfNodes; i++) {
            CoffeeCowNode Node = {};
            Node.Coords = v2(Cow->Nodes[i].Coords.x, Cow->Nodes[i].Coords.y);
            Node.CurrentDirection = Cow->Nodes[i].CurrentDirection;
            Node.Streak = Cow->Nodes[i].Streak;
            
            switch(WaitForSingleObject(p2Mutex, INFINITE))
            {
                case WAIT_OBJECT_0: _try 
                {
                    CowPlayer2->Nodes.Push(&Node);
                }
                _finally{if(!ReleaseMutex(p2Mutex)){}}break;case WAIT_ABANDONED:return false;
            }
        }
    }
}
*/
internal void
RenderGrid(game_assets *Assets, v2 Grid, v2 GridDim, real32 GridSize)
{
    for (int i = 0; i < (int)GridDim.x; i++) {
        for (int j = 0; j < (int)GridDim.y; j++) {
            v3 Coords = v3((-Grid.x + (i * GridSize)), (-Grid.y + (j * GridSize)), -0.1f);
            Push(RenderGroup, Coords, v2(GridSize), GetTexture(Assets, GAI_Grid), 0, BlendMode::gl_src_alpha);
        }
    }
}

internal void
DrawCoffee(game_assets *Assets, Coffee *Cof, v2 HalfGrid, real32 GridSize)
{
    v3 Coords = v3(roundf(-HalfGrid.x + ((Cof->Coords.x) * GridSize)), roundf(-HalfGrid.y + ((Cof->Coords.y) * GridSize)), -0.1f);
    Push(RenderGroup, Coords, v2(GridSize, GridSize), GetTexture(Assets, GAI_CoffeeTex), Cof->Rotation, BlendMode::gl_src_alpha);
}

internal void
DrawCoffeeCowNodes(game_assets *Assets, CoffeeCow *Cow, real32 GridX, real32 GridY, int GridSize)
{
    for (int i = 0; i < Cow->Nodes.Size; i++) {
        CoffeeCowNode *Node = (CoffeeCowNode*)Cow->Nodes[i];
        v3 Coords = v3(roundf(GridX + ((Node->Coords.x) * GridSize)), roundf(GridY + ((Node->Coords.y) * GridSize)), 0.5f);
        Push(RenderGroup, Coords, v2(GridSize, GridSize), GetTexture(Assets, GAI_Straight), 0.0f, BlendMode::gl_one);
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
    if (StartDirection == DOWN && EndDirection == RIGHT)
        Goal = -90;
    else if (StartDirection == RIGHT && EndDirection == DOWN)
        Goal = 90;
    else
        Goal = ERotation - BRotation;
    
    return Goal;
}

internal void
DrawCoffeeCow(game_assets *Assets, CoffeeCow *Cow, real32 GridX, real32 GridY, real32 GridSize)
{
    // Put the head, tail, and corners into a list
    Arr Corners = {};
    Corners.Init(Cow->Nodes.Size + 2, sizeof(v3));
    
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
            Push(RenderGroup, Coords, v2(GridSize, GridSize), GetTexture(Assets, GAI_Head), GetRotation(Node->CurrentDirection), BlendMode::gl_one);
            Corners.Push(&Coords);
            
            Coords.z = OutlineZ;
            Push(RenderGroup, Coords, v2(GridSize, GridSize), GetTexture(Assets, GAI_HeadOutline), GetRotation(Node->CurrentDirection), BlendMode::gl_one);
        }
        if (i == Cow->Nodes.Size - 1) {
            Coords.z = 0.5f;
            Push(RenderGroup, Coords, v2(GridSize, GridSize), GetTexture(Assets, GAI_Corner), 0.0f, BlendMode::gl_one);
            Corners.Push(&Coords);
            
            Coords.z = OutlineZ;
            Push(RenderGroup, Coords, v2(GridSize, GridSize), GetTexture(Assets, GAI_CornerOutline), 0.0f, BlendMode::gl_one);
        }
        else if (i != Cow->Nodes.Size - 1) {
            CoffeeCowNode *NextNode = (CoffeeCowNode*)Cow->Nodes[i + 1];
            if (NextNode->CurrentDirection != Node->CurrentDirection) {
                v3 Temp = v3(roundf(GridX + ((Node->Coords.x) * GridSize)),
                             roundf(GridY + ((Node->Coords.y) * GridSize)),
                             0.5f);
                Push(RenderGroup, Temp, v2(GridSize, GridSize), GetTexture(Assets, GAI_Corner), 0.0f, BlendMode::gl_one);
                Corners.Push(&Temp);
                
                Temp.z = OutlineZ;
                Push(RenderGroup, Temp, v2(GridSize, GridSize), GetTexture(Assets, GAI_CornerOutline), 0.0f, BlendMode::gl_one);
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
                Rotation = (Transition * (Goal/2)) + GetRotation(Dir);
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
                    Rotation = (Transition * (Goal/2)) + GetRotation(NDir) + (Goal/2);
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
                    Rotation = (Transition * (Goal/2)) + GetRotation(NDir) + (Goal/2);
                }
            }
            else
                Rotation = GetRotation(Node->CurrentDirection);
            
            real32 OutlineFactor = 0.96f;
            real32 Outline = (real32)(GridSize - (GridSize * OutlineFactor));
            
            Coords.x += Outline;
            Coords.y += Outline;
            Coords.z = 3.0f;
            
            Push(RenderGroup, Coords, v2(GridSize - (Outline*2), GridSize - (Outline*2)), 
                 GetTexture(Assets, GAI_CoffeeStreak), Rotation, BlendMode::gl_one);
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
            Push(RenderGroup, Coords, GetSize(*Left, *Right, GridSize - (Outline*2)), 0xFFFFFFFF, 0.0f);
            
            if (Left->y == Right->y)
                Coords.y -= Outline;
            if (Left->x == Right->x) 
                Coords.x -= Outline;
            
            Coords.z = 0.0f;
            Push(RenderGroup, Coords, GetSize(*Left, *Right, (real32)GridSize), GetTexture(Assets, GAI_StraightOutline), 0.0f, BlendMode::gl_one);
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
InitializeCow(CoffeeCow *Cow, v2 GridDim)
{
    Cow->Nodes.Clear();
    Cow->TransitionAmt = 0;
    Cow->Score = 0;
    Cow->Inputs.Clear();
    
    Cow->Nodes.Init((int)(GridDim.x * GridDim.y), sizeof(CoffeeCowNode));
    int Startx = Random(1, (int)GridDim.x - 1);
    int Starty = Random(1, (int)GridDim.y - 1);
    int incx = 0;
    int incy = 0;
    
    int Direction = 0;
    int HalfWidth = (int)(GridDim.x/2);
    int HalfHeight = (int)(GridDim.y/2);
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
    else if (Startx > (int)GridDim.x - 4)
        Startx =  (int)GridDim.y - 4;
    
    for (int i = 0; i < 4; i++) {
        AddCoffeeCowNode(Cow, Startx, Starty, Direction, Direction);
        Startx += incx;
    }
    
    Cow->Inputs.Init(3, sizeof(int));
    Cow->Speed = 7; // m/s
    Cow->Direction = Direction;
}

internal void
LoadAssets(game_assets *Assets)
{
    Assets->Textures[GAI_Background] = LoadTexture("sand.png");
    Assets->Textures[GAI_Grid] = LoadTexture("grid.png");
    Assets->Textures[GAI_Rocks] = LoadTexture("rocks.png");
    Assets->Textures[GAI_CoffeeTex] = LoadTexture("coffee.png");
    Assets->Textures[GAI_Head] = LoadTexture("cowhead.png");
    Assets->Textures[GAI_Straight] = LoadTexture("straight.png");
    Assets->Textures[GAI_Corner] = LoadTexture("circle.png");
    Assets->Textures[GAI_CoffeeStreak] = LoadTexture("coffeestreak.png");
    Assets->Textures[GAI_HeadOutline] = LoadTexture("cowheadoutline.png");
    Assets->Textures[GAI_StraightOutline] = LoadTexture("straightoutline.png");
    Assets->Textures[GAI_CornerOutline] = LoadTexture("circleoutline.png");
    
    Assets->Fonts[FI_Faune50] = LoadFont("Rubik-Medium.ttf", 50);
    Assets->Fonts[FI_Faune100] = LoadFont("Rubik-Medium.ttf", 100);
}

internal PLATFORM_WORK_QUEUE_CALLBACK(RecvData)
{
    game_state *GameState = (game_state*)Data;
    CoffeeCow *CowPlayer2 = &GameState->Player2;
    
    char Buffer[BUF_SIZE];
    memset(Buffer, 0, BUF_SIZE);
    GameState->client.recvq(Buffer, BUF_SIZE);
    game_packet *Recv = (game_packet*)Buffer; 
    ServerCoffeeCow *Cow = &Recv->Cow;
    CowPlayer2->TransitionAmt = Cow->TransitionAmt;
    CowPlayer2->Score = Cow->Score;
    CowPlayer2->Nodes.Clear();
    for (int i = 0; i < Cow->NumOfNodes; i++) {
        CoffeeCowNode Node = {};
        Node.Coords = v2(Cow->Nodes[i].Coords.x, Cow->Nodes[i].Coords.y);
        Node.CurrentDirection = Cow->Nodes[i].CurrentDirection;
        Node.Streak = Cow->Nodes[i].Streak;
        CowPlayer2->Nodes.Push(&Node);
    }
}

internal PLATFORM_WORK_QUEUE_CALLBACK(SendData)
{
    game_state *GameState = (game_state*)Data;
    CoffeeCow *CowPlayer = &GameState->Player1;
    
    
    game_packet Send = {};
    Send.Cow.TransitionAmt = CowPlayer->TransitionAmt;
    Send.Cow.Score = CowPlayer->Score;
    for (int i = 0; i < CowPlayer->Nodes.Size; i++) {
        CoffeeCowNode* N = (CoffeeCowNode*)CowPlayer->Nodes[i];
        ServerCoffeeCowNode Node = {};
        Node.Coords = iv2(N->Coords);
        Node.CurrentDirection = (int8)N->CurrentDirection;
        Node.Streak = (int8)N->Streak;
        Send.Cow.Nodes[i] = Node;
        Send.Cow.NumOfNodes++;
    }
    Send.Disconnect = GameState->Disconnect;
    
    char Buffer[BUF_SIZE];
    memset(Buffer, 0, BUF_SIZE);
    memcpy(Buffer, &Send, sizeof(game_packet)); 
    GameState->client.sendq(Buffer, SEND_BUFFER_SIZE);
    
    if (Send.Disconnect == 1)
        GameState->client.disconnect();
}

void UpdateRender(platform* p)
{
    game_state *GameState = (game_state*)p->Memory.TransientStorage;
    Camera *C = &GameState->C;
    
    if (!p->Initialized)
    {
        p->Initialized = true;
        
        Manager.Next = (char*)p->Memory.PermanentStorage;
        qalloc(sizeof(game_state));
        
        *C = {};
        C->Position = v3(0, 0, -900);
        C->Target = v3(0, 0, 0);
        C->Up = v3(0, 1, 0);
        C->FOV = 90.0f;
        C->F = 0.01f;
        
        GameState->Menu = menu::main_menu;
        GameState->Mode = game_mode::not_in_game;
        GameState->PreviousMode = game_mode::not_in_game;
        GameState->GridDim = v2(17, 17);
        
        LoadAssets(&GameState->Assets);
    }
    
    real32 NewGridSize = p->Dimension.Height / (GameState->GridDim.x + 5);
    if (NewGridSize != GameState->GridSize)
        GameState->GridSize = NewGridSize;
    
    platform_keyboard_input *Keyboard = &p->Input.Keyboard;
    if (Keyboard->F5.NewEndedDown)
        GameState->ShowFPS = !GameState->ShowFPS;
    
    if (GameState->ShowFPS) {
        real32 fps = 0;
        if (p->Input.WorkSecondsElapsed != 0) {
            fps= 1 / p->Input.WorkSecondsElapsed;
            Strinq FPS = S() + (int)fps;
            v2 SDim = GetStringDimensions(GetFont(&GameState->Assets, FI_Faune50), GetData(FPS));
            PrintOnScreen(GetFont(&GameState->Assets, FI_Faune50), GetData(FPS), v2((p->Dimension.Width/2)-(int)SDim.x-10, -p->Dimension.Height/2 + 10), 0xFF000000);
        }
    }
    
    C->Dimension = p->Dimension;
    
    if (GameState->Mode == game_mode::singleplayer) {
        CoffeeCow *CowPlayer = &GameState->Player1;
        Coffee *Collect = &GameState->Collect;
        
        if (GameState->ResetGame) {
            InitializeCow(CowPlayer, GameState->GridDim);
            Collect->NewLocation = true;
            GameState->ResetGame = false;
        }
        platform_controller_input *Controller = &p->Input.Controllers[0];
        
        if (Keyboard->Escape.NewEndedDown) {
            GameState->Mode = game_mode::not_in_game;
            GameState->Menu = menu::pause_menu;
        }
        
        if(Controller->MoveLeft.NewEndedDown)
            AddInput(CowPlayer, LEFT);
        if(Controller->MoveRight.NewEndedDown)
            AddInput(CowPlayer, RIGHT);
        if(Controller->MoveDown.NewEndedDown)
            AddInput(CowPlayer, DOWN);
        if(Controller->MoveUp.NewEndedDown)
            AddInput(CowPlayer, UP);
        
        if (!MoveCoffeeCow(CowPlayer, p->Input.WorkSecondsElapsed, GameState->GridDim)) {
            GameState->Mode = game_mode::not_in_game;
            GameState->Menu = menu::game_over_menu;
        }
        
        CoffeeCowNode* Head = (CoffeeCowNode*)CowPlayer->Nodes[0];
        if (Head->Coords.x == Collect->Coords.x && Head->Coords.y == Collect->Coords.y) {
            CowPlayer->Score++;
            Collect->NewLocation = true;
            
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
        
        if (Collect->NewLocation) {
            bool32 ValidLocation = false;
            while (!ValidLocation) {
                Collect->Coords = v2((real32)Random(0, (int)GameState->GridDim.x - 1),
                                     (real32)Random(0, (int)GameState->GridDim.y - 1));
                ValidLocation = true;
                for (int i = 0; i < CowPlayer->Nodes.Size; i++) {
                    CoffeeCowNode* Node = (CoffeeCowNode*)CowPlayer->Nodes[i];
                    if (Node->Coords.x == Collect->Coords.x &&
                        Node->Coords.y == Collect->Coords.y)
                        ValidLocation = false;
                }
            }
            Collect->NewLocation = false;
        }
        
        v2 HalfGrid = v2((GameState->GridDim.x * GameState->GridSize)/2, (GameState->GridDim.y * GameState->GridSize)/2);
        
        v3 RockCoords = v3(-HalfGrid.x - (GameState->GridSize * GameState->GridDim.x*1/6),
                           -HalfGrid.y - (GameState->GridSize * GameState->GridDim.y*1/6), 
                           1.0f);
        v2 RockDim = v2((GameState->GridDim.x * GameState->GridSize) * 4/3 - 1, (GameState->GridDim.y * GameState->GridSize) * 4/3 - 1);
        Push(RenderGroup, RockCoords, RockDim, GetTexture(&GameState->Assets, GAI_Rocks), 0, BlendMode::gl_src_alpha);
        
        v3 BackgroundCoords = v3(-((real32)p->Dimension.Width)/2 - 5, 
                                 -((real32)p->Dimension.Height)/2 - 5,
                                 -1.0f);
        v2 BackgroundDim = v2(p->Dimension.Width + 5, p->Dimension.Height + 5);
        Push(RenderGroup, BackgroundCoords, BackgroundDim, GetTexture(&GameState->Assets, GAI_Background), 0, BlendMode::gl_src_alpha);
        
        RenderGrid(&GameState->Assets, HalfGrid, GameState->GridDim, GameState->GridSize);
        DrawCoffeeCow(&GameState->Assets, CowPlayer, -HalfGrid.x, -HalfGrid.y, GameState->GridSize);
        
        Collect->Rotation += (p->Input.WorkSecondsElapsed * 100);
        if (Collect->Rotation > 360)
            Collect->Rotation -= 360;
        DrawCoffee(&GameState->Assets, Collect, HalfGrid, GameState->GridSize);
        
        Strinq Score = S() + CowPlayer->Score;
        PrintOnScreen(GetFont(&GameState->Assets, FI_Faune50), GetData(Score), v2(-p->Dimension.Width/2 + 10, -p->Dimension.Height/2 + 10), 0xFFFFFFFF);
        
        BeginMode2D(*C);
        RenderPieceGroup(RenderGroup);
        EndMode2D();
    }
    else if (GameState->Mode == game_mode::multiplayer) {
        CoffeeCow *CowPlayer = &GameState->Player1;
        CoffeeCow *CowPlayer2 = &GameState->Player2;
        Coffee *Collect = &GameState->Collect;
        
        if (GameState->ResetGame) {
            InitializeCow(CowPlayer, GameState->GridDim);
            CowPlayer2->Nodes.Init((int)(GameState->GridDim.x * GameState->GridDim.y), sizeof(CoffeeCowNode));
            
            GameState->client.create(GameState->IP, GameState->Port, TCP);
            GameState->ResetGame = false;
            GameState->Disconnect = 0;
        }
        
        platform_controller_input *Controller = &p->Input.Controllers[0];
        if (Keyboard->Escape.NewEndedDown) {
            GameState->PreviousMode = game_mode::multiplayer;
            GameState->Mode = game_mode::not_in_game;
            GameState->Menu = menu::pause_menu;
        }
        
        if(Controller->MoveLeft.NewEndedDown)
            AddInput(CowPlayer, LEFT);
        if(Controller->MoveRight.NewEndedDown)
            AddInput(CowPlayer, RIGHT);
        if(Controller->MoveDown.NewEndedDown)
            AddInput(CowPlayer, DOWN);
        if(Controller->MoveUp.NewEndedDown)
            AddInput(CowPlayer, UP);
        
        if (!MoveCoffeeCow(CowPlayer, p->Input.WorkSecondsElapsed, GameState->GridDim))
            int i = 0;
        
        Win32AddEntry(&p->Queue, RecvData, GameState);
        Win32AddEntry(&p->Queue, SendData, GameState);
        
        PrintqDebug(S() + (int)CowPlayer2->TransitionAmt + "\n");
        
        v2 HalfGrid = v2((GameState->GridDim.x * GameState->GridSize)/2, (GameState->GridDim.y * GameState->GridSize)/2);
        
        v3 RockCoords = v3(-HalfGrid.x - (GameState->GridSize * GameState->GridDim.x*1/6),
                           -HalfGrid.y - (GameState->GridSize * GameState->GridDim.y*1/6), 
                           1.0f);
        v2 RockDim = v2((GameState->GridDim.x * GameState->GridSize) * 4/3 - 1, 
                        (GameState->GridDim.y * GameState->GridSize) * 4/3 - 1);
        Push(RenderGroup, RockCoords, RockDim, GetTexture(&GameState->Assets, GAI_Rocks), 0, BlendMode::gl_src_alpha);
        
        v3 BackgroundCoords = v3(-((real32)p->Dimension.Width)/2 - 5, 
                                 -((real32)p->Dimension.Height)/2 - 5,
                                 -1.0f);
        v2 BackgroundDim = v2(p->Dimension.Width + 5, p->Dimension.Height + 5);
        Push(RenderGroup, BackgroundCoords, BackgroundDim, GetTexture(&GameState->Assets, GAI_Background), 0, BlendMode::gl_src_alpha);
        
        RenderGrid(&GameState->Assets, HalfGrid, GameState->GridDim, GameState->GridSize);
        DrawCoffeeCow(&GameState->Assets, CowPlayer, -HalfGrid.x, -HalfGrid.y, GameState->GridSize);
        DrawCoffeeCow(&GameState->Assets, CowPlayer2, -HalfGrid.x, -HalfGrid.y, GameState->GridSize);
        
        BeginMode2D(*C);
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