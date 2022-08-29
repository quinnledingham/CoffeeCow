enum asset_font_type
{
    FontType_Default = 0,
};

enum asset_tag_id
{
    Tag_Player1,
    Tag_Player2,
    Tag_Player3,
    Tag_Player4,
    
    Tag_UnicodeCodepoint,
    Tag_FontType,
    
    Tag_Count
};

enum asset_type_id
{
    Asset_None,
    
    //
    // Bitmaps
    //
    // Cow related
    Asset_CowHead,
    Asset_CowHeadOutline,
    Asset_CowStraight,
    Asset_CowStraightOutline,
    Asset_CowCorner,
    Asset_CowCornerOutline,
    Asset_CowTail,
    Asset_CowTongue,
    Asset_CowSpot,
    
    // World related
    Asset_Rock,
    Asset_Background,
    Asset_Grass,
    Asset_Grid,
    Asset_Coffee,
    
    // Menu
    Asset_MenuBackground,
    Asset_MenuLogo,
    Asset_DefaultCheckBox,
    Asset_HighlightCheckBox,
    Asset_ActiveCheckBox,
    Asset_ActiveHighlightCheckBox,
    
    //
    // Fonts
    //
    Asset_Font,
    Asset_FontGlyph,
    
    //
    // Sounds
    //
    Asset_DrinkSound,
    Asset_Bonk,
    Asset_Song,
    
    Asset_Count
};

#define WindowName "Coffee Cow"
#define ClientHeight 0.80f
#define ClientWidth 0.0f

#define Permanent_Storage_Size Megabytes(512)
#define Transient_Storage_Size Megabytes(256)

#define IconFileName "bitmaps/icon2.png"
#define CurrentDirectory "../game/data"

#define QLIB_SDL
#define QLIB_OPENGL 1
//#define QLIB_DISCRETE_GRAPHICS
#define QLIB_WINDOW_APPLICATION
#include "qlib/application.h"

#include "coffee_cow.h"
#include "snake.h"

internal void
DrawEnvironment(bitmap_id BackgroundID,
                bitmap_id InnerBackgroundID,
                bitmap_id GridID,
                bitmap_id BorderID,
                v2 TopLeftCornerCoords,
                v2 PlatformDim,
                v2 GridCoords,
                v2 GridDim,
                v2 RealGridDim,
                real32 GridSize,
                real32 Z)
{
    // Draw Background
    v2 BackgroundCoords = TopLeftCornerCoords - 5;
    v2 BackgroundDim = 0;
    if (PlatformDim.x >= PlatformDim.y)
        BackgroundDim = v2(PlatformDim.x + 5, PlatformDim.x + 5);
    else
        BackgroundDim = v2(PlatformDim.y + 5, PlatformDim.y + 5);
    
    BackgroundCoords.y -= ((BackgroundDim.y - (PlatformDim.y + 5)) / 2);
    
    Push(v3(BackgroundCoords, Z), BackgroundDim, BackgroundID, 0, blend_mode::gl_src_alpha);
    
    // Draw InnerBackground
    Push(v3(GridCoords, Z + 0.01f), RealGridDim, InnerBackgroundID, 0, blend_mode::gl_src_alpha);
    
    // Draw Grid
    for (int i = 0; i < (int)GridDim.x; i++) {
        for (int j = 0; j < (int)GridDim.y; j++) {
            v3 Coords = v3((GridCoords.x + (i * GridSize)), (GridCoords.y + (j * GridSize)), Z + 0.02f);
            Push(Coords, v2(GridSize), GridID, 0, blend_mode::gl_src_alpha);
        }
    }
    
    // Draw Border
    v2 RockCoords = GridCoords - (RealGridDim/6) + 1;
    v2 RockDim = v2(RealGridDim.x * 4/3 - 2, RealGridDim.y * 4/3 - 2);
    Push(v3(RockCoords, Z + 0.03f), RockDim, BorderID, 0, blend_mode::gl_src_alpha);
}

internal void
DrawScore(assets *Assets, int Score, v2 TopLeftCornerCoords, v2 BufferDim)
{
    strinq ScoreStrinq = S() + "Score: " + Score;
    
    font_string FontString = {};
    FontStringInit(&FontString, GetFont(Assets, GetFirstFont(Assets, Asset_Font)), ScoreStrinq.Data, 50, 0xFFFFFFFF);
    
    v2 ResizeFactors = GetResizeFactor(v2(1000, 1000), BufferDim);
    FontStringResize(&FontString, ResizeEquivalentAmount(50, ResizeFactors.y));
    
    FontStringPrint(&FontString, TopLeftCornerCoords + 10);
    
    v2 ScoreSize = FontStringGetDim(&FontString);
    Push(v3(TopLeftCornerCoords + 5, 50.0f), ScoreSize + 10, 0x96000000, 0.0f);
}

internal void
DrawCoffee(Coffee *Cof, bitmap_id Bitmap, real32 Seconds, v2 GridCoords, real32 GridSize, real32 Z)
{
    Cof->Rotation += (Seconds * 100);
    if (Cof->Rotation > 360) Cof->Rotation -= 360;
    
    if (Cof->IncreasingHeight) {
        
        Cof->Height += (Seconds * 2);
        if (Cof->Height > (GridSize * 0.05f)) Cof->IncreasingHeight = false;
    }
    else if (!Cof->IncreasingHeight) {
        Cof->Height -= (Seconds * 2);
        if (Cof->Height < (-GridSize * 0.05f)) Cof->IncreasingHeight = true;
    }
    
    v3 Coords = v3(GridCoords.x + (Cof->Coords.x * GridSize) - (Cof->Height/2),
                   GridCoords.y + (Cof->Coords.y * GridSize) - (Cof->Height/2),
                   Z);
    Push(Coords, v2(GridSize + Cof->Height), Bitmap, Cof->Rotation, blend_mode::gl_src_alpha);
}
/*
internal void
DrawCoffeeCowNodes(texture *Texture, CoffeeCow *Cow, real32 GridX, real32 GridY, int GridSize)
{
    for (int i = 0; i < Cow->Nodes.Size; i++) {
        CoffeeCowNode *Node = (CoffeeCowNode*)Cow->Nodes[i];
        v3 Coords = v3(roundf(GridX + ((Node->Coords.x) * GridSize)), roundf(GridY + ((Node->Coords.y) * GridSize)), 0.5f);
        Push(Coords, v2(GridSize, GridSize), Texture, 0.0f, blend_mode::gl_one);
    }
}
*/
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
inline real32
GetRotation(int Direction)
{
    if (Direction == LEFT) return 270.0f;
    else if (Direction == RIGHT) return 90.0f;
    else if (Direction == UP) return 180.0f;
    else if (Direction == DOWN) return 0.0f;
    else return 0.0f;
}

internal real32
GetAngleDiff(int StartDirection, int EndDirection)
{
    real32 Goal;
    real32 BRotation = GetRotation(StartDirection);
    real32 ERotation = GetRotation(EndDirection);
    if (StartDirection == DOWN && EndDirection == LEFT) Goal = -90;
    else if (StartDirection == LEFT && EndDirection == DOWN) Goal = 90;
    else Goal = ERotation - BRotation;
    return Goal;
}

internal void
DrawCoffeeCow(CoffeeCow *Cow, 
              v2 CoffeeCoords,
              assets *Assets,
              asset_tag_id Tag,
              real32 Seconds, 
              real32 GridX,
              real32 GridY,
              real32 GridSize)
{
    // Put the head, tail, and corners into a list
    Arr *Corners = &Cow->Corners;
    
    asset_vector MatchVector = {};
    asset_vector WeightVector = {};
    MatchVector.E[Tag] = 1.0f;
    WeightVector.E[Tag] = 1.0f;
    
    qlibBlendMode(BLEND_MODE_GL_ONE);
    
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
            real32 HeadResize = GridSize * 0.05f;
            v3 Temp = Coords;
            Temp.x -= (HeadResize/2);
            Temp.y -= (HeadResize/2);
            Temp.z = 2.0f;
            
            
            Push(Temp, v2(GridSize + HeadResize),
                 GetBestMatchBitmap(Assets, Asset_CowHead, &MatchVector, &WeightVector),
                 GetRotation(Node->CurrentDirection));
            
            Corners->Push(&Coords);
            
            CoffeeCowNode *Head = (CoffeeCowNode*)Cow->Nodes[i];
            v2 NextCoords = Node->Coords;
            if (Node->CurrentDirection == RIGHT) NextCoords.x += 1;
            else if (Node->CurrentDirection == UP) NextCoords.y -= 1;
            else if (Node->CurrentDirection == LEFT) NextCoords.x -= 1;
            else if (Node->CurrentDirection == DOWN) NextCoords.y += 1;
            
            v2 NextNextCoords = NextCoords;
            if (Node->CurrentDirection == RIGHT) NextNextCoords.x += 1;
            else if (Node->CurrentDirection == UP) NextNextCoords.y -= 1;
            else if (Node->CurrentDirection == LEFT) NextNextCoords.x -= 1;
            else if (Node->CurrentDirection == DOWN) NextNextCoords.y += 1;
            
            real32 MouthOffset = GridSize * 0.30f;
            if ((NextCoords.x == CoffeeCoords.x && NextCoords.y == CoffeeCoords.y) ||
                (NextNextCoords.x == CoffeeCoords.x && NextNextCoords.y == CoffeeCoords.y)) {
                if (Cow->MouthOpening < MouthOffset)
                    Cow->MouthOpening += (MouthOffset * Seconds * Cow->Speed);
            }
            else {
                if (Cow->MouthOpening > 0) {
                    Cow->MouthOpening -= (MouthOffset * Seconds * Cow->Speed);
                }
            }
            
            v3 MouthCoords = Temp;
            if (Node->CurrentDirection == RIGHT) MouthCoords.x += Cow->MouthOpening;
            else if (Node->CurrentDirection == UP) MouthCoords.y -= Cow->MouthOpening;
            else if (Node->CurrentDirection == LEFT) MouthCoords.x -= Cow->MouthOpening;
            else if (Node->CurrentDirection == DOWN) MouthCoords.y += Cow->MouthOpening;
            
            MouthCoords.z = 0.0f;
            Push(MouthCoords, v2(GridSize + HeadResize), 
                 GetBestMatchBitmap(Assets, Asset_CowTongue, &MatchVector, &WeightVector),
                 GetRotation(Node->CurrentDirection));
            
            Temp.z = OutlineZ;
            Coords.z = OutlineZ;
            Push(Temp, v2(GridSize + HeadResize), 
                 GetBestMatchBitmap(Assets, Asset_CowHeadOutline, &MatchVector, &WeightVector),
                 GetRotation(Node->CurrentDirection));
        }
        if (i == Cow->Nodes.Size - 1) {
            Coords.z = 0.5f;
            Push(Coords, v2(GridSize, GridSize), 
                 GetBestMatchBitmap(Assets, Asset_CowCorner, &MatchVector, &WeightVector));
            Corners->Push(&Coords);
            
            v3 TailCoords = v3();
            v3 OldDirTailCoords = Coords;
            v3 NewDirTailCoords = Coords;
            real32 TailOffset = GridSize * 0.90f;
            
            if (Node->CurrentDirection == RIGHT) NewDirTailCoords.x -= TailOffset;
            else if (Node->CurrentDirection == UP) NewDirTailCoords.y += TailOffset;
            else if (Node->CurrentDirection == LEFT) NewDirTailCoords.x += TailOffset;
            else if (Node->CurrentDirection == DOWN) NewDirTailCoords.y -= TailOffset;
            
            if (Cow->Tail.OldDir == RIGHT) OldDirTailCoords.x -= TailOffset;
            else if (Cow->Tail.OldDir == UP) OldDirTailCoords.y += TailOffset;
            else if (Cow->Tail.OldDir == LEFT) OldDirTailCoords.x += TailOffset;
            else if (Cow->Tail.OldDir == DOWN) OldDirTailCoords.y -= TailOffset;
            
            
            real32 TransitionAmt = Cow->TransitionAmt;
            
            real32 Rotation = 0;
            if (Cow->Tail.ChangingDirections) {
                real32 Goal = GetAngleDiff(Cow->Tail.OldDir, Cow->Tail.NewDir);
                real32 Transition = Cow->TransitionAmt;
                v3 DiffCoords = NewDirTailCoords - OldDirTailCoords;
                DiffCoords = DiffCoords *  Transition;
                TailCoords = OldDirTailCoords + DiffCoords;
                Rotation = (Transition * Goal) + GetRotation(Cow->Tail.OldDir);
            }
            else {
                TailCoords = NewDirTailCoords;
                Rotation = GetRotation(Node->CurrentDirection);
            }
            TailCoords.z = 0.0f;
            Push(TailCoords, v2(GridSize, GridSize), 
                 GetBestMatchBitmap(Assets, Asset_CowTail, &MatchVector, &WeightVector),
                 Rotation);
            
            Coords.z = OutlineZ;
            Push(Coords, v2(GridSize, GridSize),
                 GetBestMatchBitmap(Assets, Asset_CowCornerOutline, &MatchVector, &WeightVector));
        }
        else if (i != Cow->Nodes.Size - 1) {
            CoffeeCowNode *NextNode = (CoffeeCowNode*)Cow->Nodes[i + 1];
            if (NextNode->CurrentDirection != Node->CurrentDirection) {
                v3 Temp = v3(roundf(GridX + ((Node->Coords.x) * GridSize)),
                             roundf(GridY + ((Node->Coords.y) * GridSize)),
                             0.5f);
                Push(Temp, v2(GridSize, GridSize), 
                     GetBestMatchBitmap(Assets, Asset_CowCorner, &MatchVector, &WeightVector));
                Corners->Push(&Temp);
                
                Temp.z = OutlineZ;
                Push(Temp, v2(GridSize, GridSize),
                     GetBestMatchBitmap(Assets, Asset_CowCornerOutline, &MatchVector, &WeightVector));
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
            
            real32 OutlineFactor = 0.9625f;
            real32 Outline = (real32)(GridSize - (GridSize * OutlineFactor));
            
            Coords.x += Outline;
            Coords.y += Outline;
            Coords.z = 3.0f;
            
            Push(Coords, v2(GridSize - (Outline*2), GridSize - (Outline*2)),
                 GetIndexBitmap(Assets, Asset_CowSpot, Node->Streak-1), Rotation);
        }
    }
    
    // Draw the connect rects
    for (int i = 0; i < (Corners->Size - 1); i++)
    {
        v3 *Node = (v3*)Corners->Get(i);
        v3 *NextNode = (v3*)Corners->Get(i + 1);
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
        
        real32 OutlineFactor = 0.9625f;
        real32 Outline = (real32)(GridSize - (GridSize * OutlineFactor));
        
        v3 Coords = v3(Left->x, Left->y, 0.5f);
        if (Left->y == Right->y) Coords = Coords + v3(GridSize/2, Outline, 0);
        if (Left->x == Right->x) Coords = Coords + v3(Outline, GridSize/2, 0);
        
        if (Left->y != Right->y || Left->x != Right->x) {
            Push(Coords, GetSize(*Left, *Right, GridSize - (Outline*2)), 0xFFFFFFFF);
            
            if (Left->y == Right->y) Coords.y -= Outline;
            if (Left->x == Right->x) Coords.x -= Outline;
            Coords.z = 0.0f;
            
            Push(Coords, GetSize(*Left, *Right, GridSize), 0xFF000000);
            //Push(Coords, GetSize(*Left, *Right, (real32)GridSize), GetBestMatchBitmap(Assets, Asset_CowStraightOutline, &MatchVector, &WeightVector));
        }
    }
    
    qlibBlendMode(BLEND_MODE_GL_SRC_ALPHA);
    Corners->Clear();
}
inline void
DrawCoffeeCow(CoffeeCow *Cow, v2 CoffeeCoords,  real32 Seconds, real32 GridX, real32 GridY, real32 GridSize)
{
    DrawCoffeeCow(Cow, CoffeeCoords, 0, Tag_Player1, Seconds, GridX, GridY, GridSize);
}

internal void
AddInput(CoffeeCow *Cow, int NewDirection)
{
    Cow->Speed = 8; // m/s
    if (Cow->Inputs.Size >= 3)
        return;
    
    int OldDirection = 0;
    if (Cow->Inputs.Size == 0) {
        OldDirection = Cow->Direction;
    }
    else 
        OldDirection = *(int*)Cow->Inputs[Cow->Inputs.Size - 1];
    
    if (!(OldDirection == NewDirection) && // Same Direction
        !(OldDirection + 2 == NewDirection) && // Backwards Direction
        !(OldDirection - 2 == NewDirection)) {
        Cow->Inputs.Push((void*)&NewDirection);
        //Cow->TransitionAmt = 1;
    }
}

inline void
CoffeeCowProcessInput(platform_controller_input *Controller, CoffeeCow *Cow) {}

inline void
CoffeeCowProcessInput(game_controller *Controller, CoffeeCow *Cow)
{
    if(OnKeyDown(Controller->MoveLeft)) AddInput(Cow, LEFT);
    if(OnKeyDown(Controller->MoveRight))AddInput(Cow, RIGHT);
    if(OnKeyDown(Controller->MoveDown)) AddInput(Cow, DOWN);
    if(OnKeyDown(Controller->MoveUp)) AddInput(Cow, UP);
}

internal void
MoveInDirection(v2 *Coords, int Direction)
{
    if (Direction == RIGHT) Coords->x += 1;
    else if (Direction == UP) Coords->y -= 1;
    else if (Direction == LEFT) Coords->x -= 1;
    else if (Direction == DOWN) Coords->y += 1;
}


internal bool32
CheckCollisonCow(CoffeeCowNode *Head, CoffeeCow *Cow)
{
    v2 HeadCoords = v2(Head->Coords);
    MoveInDirection(&HeadCoords, Head->CurrentDirection);
    int i = 0;
    if (Head == Cow->Nodes[0])
        i = 1;
    for (i; i < Cow->Nodes.Size; i++) {
        CoffeeCowNode* Node = (CoffeeCowNode*)Cow->Nodes[i];
        if (HeadCoords.x == Node->Coords.x &&
            HeadCoords.y == Node->Coords.y)
            return false;
    }
    
    return true;
}

internal bool32
CheckCollisionCoffeeCow(CoffeeCow *Cow, CoffeeCow *OtherCow, v2 GridDim)
{
    CoffeeCowNode *Head = (CoffeeCowNode*)Cow->Nodes[0];
    
    // Check collision with wall
    v2 Coords = Head->Coords;
    int Direction = Cow->Direction;
    if ((Direction == LEFT && Coords.x <= 0) || (Direction == RIGHT && Coords.x >= GridDim.x - 1) ||
        (Direction == UP && Coords.y <= 0) || (Direction == DOWN && Coords.y >= GridDim.y - 1))
        return false;
    
    //for (int i = 0; i < OtherCows->Nodes.Size; i++) {
    if (!CheckCollisonCow((CoffeeCowNode*)Cow->Nodes[0], OtherCow))
        return false;
    //}
    
    return true;
}

internal bool32
CheckCollionCoffeeCow(CoffeeCow *Cow, coffee_cow_collision_node *CollisionNodes, v2 GridDim)
{
    CoffeeCowNode *Head = (CoffeeCowNode*)Cow->Nodes[0];
    v2 HeadCoords = Head->Coords;
    
    // Check collision with wall
    int Direction = Cow->Direction;
    if ((Direction == LEFT && HeadCoords.x <= 0) || (Direction == RIGHT && HeadCoords.x >= GridDim.x - 1) ||
        (Direction == UP && HeadCoords.y <= 0) || (Direction == DOWN && HeadCoords.y >= GridDim.y - 1))
        return false;
    
    MoveInDirection(&HeadCoords, Head->CurrentDirection);
    u32 i = 0;
    if (HeadCoords == CollisionNodes->Head)
        i = 1;
    for (i; i < CollisionNodes->NumNodes; i++) {
        v2 Node = CollisionNodes->Nodes[i];
        if (HeadCoords.x == Node.x &&
            HeadCoords.y == Node.y)
            return false;
    }
    
    return true;
}

internal void
MoveCoffeeCow(CoffeeCow *Cow, real32 SecondsElapsed, v2 GridDim)
{
    real32 Dm = (Cow->Speed * SecondsElapsed);
    real32 TransitionAmt = Cow->TransitionAmt + Dm;
    CoffeeCowNode *Head = (CoffeeCowNode*)Cow->Nodes[0];
    
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
                if (i == Cow->Nodes.Size-1 && Node->CurrentDirection != Node->NextDirection) {
                    Cow->Tail.ChangingDirections = true;
                    Cow->Tail.OldDir = Node->CurrentDirection;
                    Cow->Tail.NewDir = Node->NextDirection;
                } 
                else
                    Cow->Tail.ChangingDirections = false;
                
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
        int Rand = Random(1, 5);
        if (Rand < 4)
            LastTail->Streak = Rand;
        else
            LastTail->Streak = 0;
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
    Cow->Corners.Init(Cow->Nodes.MaxSize + 2, sizeof(v3));
    int Startx = Random(1, (int)GridDim.x - 1);
    int Starty = Random(1, (int)GridDim.y - 1);
    int incx = 0;
    int incy = 0;
    
    int Direction = 0;
    int HalfWidth = (int)(GridDim.x/2);
    int HalfHeight = (int)(GridDim.y/2);
    if (Startx < HalfWidth) Direction = RIGHT;
    else if (Startx >= HalfWidth) Direction = LEFT;
    
    if (Direction == RIGHT) incx = -1;
    else if (Direction == LEFT) incx = 1;
    
    if (Startx < 4) Startx = 4;
    else if (Startx > (int)GridDim.x - 4) Startx = (int)GridDim.y - 4;
    
    for (int i = 0; i < 4; i++) {
        AddCoffeeCowNode(Cow, Startx, Starty, Direction, Direction);
        Startx += incx;
    }
    
    Cow->Inputs.Init(3, sizeof(int));
    Cow->Speed = 8; // m/s
    Cow->Direction = Direction;
}

// See if coffee location has a snake part there
internal bool32
CoffeeValidLocation(Coffee *Cof, CoffeeCow *Cow)
{
    for (int i = 0; i < Cow->Nodes.Size; i++) {
        CoffeeCowNode* Node = (CoffeeCowNode*)Cow->Nodes[i];
        if (Node->Coords.x == Cof->Coords.x && Node->Coords.y == Cof->Coords.y)
            return false;
    }
    return true;
}

internal void
MoveCoffee(Coffee *Cof, CoffeeCow *Cows, u32 NumPlayers, v2 GridDim)
{
    bool32 ValidLocation = false;
    int Attempts = 0;
    
    int X = 0;
    int Y = 0;
    
    while (!ValidLocation)
    {
        Attempts++;
        
        if (Attempts < 10) {
            Cof->Coords = v2(Random(0, (int)GridDim.x - 1), Random(0, (int)GridDim.y - 1));
        }
        else if (Attempts >= 10) {
            Cof->Coords = v2(X++, Y++); 
            // After 10 random attempts just find
            // first open spot on grid
        }
        
        ValidLocation = true;
        Cof->IncreasingHeight = true;
        Cof->Height = 0;
        
        for (u32 i = 0; i < NumPlayers; i++) {
            CoffeeCow *Cow = &Cows[i];
            for (int i = 0; i < Cow->Nodes.Size; i++) {
                CoffeeCowNode* Node = (CoffeeCowNode*)Cow->Nodes[i];
                if (Node->Coords.x == Cof->Coords.x && Node->Coords.y == Cof->Coords.y)
                    ValidLocation = false;
            }
        }
    }
}

internal bool32
CollectCoffee(CoffeeCow *Cow, Coffee *Cof, v2 GridDim)
{
    CoffeeCowNode* Head = (CoffeeCowNode*)Cow->Nodes[0];
    if (Head->Coords.x == Cof->Coords.x && Head->Coords.y == Cof->Coords.y) {
        Cow->Score++;
        Cof->NewLocation = true;
        
        // Make Cow Longer
        CoffeeCowNode* LastNode = (CoffeeCowNode*)Cow->Nodes[Cow->Nodes.Size-1];
        int NewX = (int)LastNode->Coords.x;
        int NewY = (int)LastNode->Coords.y;
        
        if (LastNode->CurrentDirection == UP) NewY++;
        else if (LastNode->CurrentDirection == LEFT) NewX++;
        else if (LastNode->CurrentDirection == DOWN) NewY--;
        else if (LastNode->CurrentDirection == RIGHT) NewX--;
        
        AddCoffeeCowNode(Cow, NewX, NewY, LastNode->CurrentDirection, LastNode->CurrentDirection);
        
        return true;
    }
    return false;
}

internal void
MakeAssetFile(platform_work_queue *Queue, assets *Assets)
{
    debug_builder_assets BuilderAssets = {};
    
    //
    // Bitmaps
    //
    BuilderAddBitmap(&BuilderAssets, "bitmaps/sand.png", Asset_Background);
    BuilderAddBitmap(&BuilderAssets, "bitmaps/grass.png", Asset_Grass);
    BuilderAddBitmap(&BuilderAssets, "bitmaps/grid.png", Asset_Grid);
    BuilderAddBitmap(&BuilderAssets, "bitmaps/coffee.png", Asset_Coffee);
    BuilderAddBitmap(&BuilderAssets, "bitmaps/rocks.png", Asset_Rock);
    
    BuilderAddBitmap(&BuilderAssets, "bitmaps/logo.png", Asset_MenuLogo);
    BuilderAddBitmap(&BuilderAssets, "bitmaps/mainmenuback.png", Asset_MenuBackground);
    
    BuilderAddBitmap(&BuilderAssets, "bitmaps/join.png", Asset_DefaultCheckBox);
    BuilderAddBitmap(&BuilderAssets, "bitmaps/joinalt.png", Asset_HighlightCheckBox);
    BuilderAddBitmap(&BuilderAssets, "bitmaps/joinhover.png", Asset_ActiveCheckBox);
    BuilderAddBitmap(&BuilderAssets, "bitmaps/joinalthover.png", Asset_ActiveHighlightCheckBox);
    
    BuilderAddBitmapTag(&BuilderAssets, "bitmaps/cow1/cowhead.png", Asset_CowHead, Tag_Player1);
    BuilderAddBitmapTag(&BuilderAssets, "bitmaps/cow1/cowheadoutline.png", Asset_CowHeadOutline, Tag_Player1);
    BuilderAddBitmapTag(&BuilderAssets, "bitmaps/cow1/straight.png", Asset_CowStraight, Tag_Player1);
    BuilderAddBitmapTag(&BuilderAssets, "bitmaps/cow1/straightoutline.png", Asset_CowStraightOutline, Tag_Player1);
    BuilderAddBitmapTag(&BuilderAssets, "bitmaps/cow1/circle.png", Asset_CowCorner, Tag_Player1);
    BuilderAddBitmapTag(&BuilderAssets, "bitmaps/cow1/circleoutline.png", Asset_CowCornerOutline, Tag_Player1);
    BuilderAddBitmapTag(&BuilderAssets, "bitmaps/cow1/tail.png", Asset_CowTail, Tag_Player1);
    BuilderAddBitmapTag(&BuilderAssets, "bitmaps/cow1/tongue.png", Asset_CowTongue, Tag_Player1);
    
    BuilderAddBitmap(&BuilderAssets, "bitmaps/cow1/spot1.png", Asset_CowSpot);
    BuilderAddBitmap(&BuilderAssets, "bitmaps/cow1/spot2.png", Asset_CowSpot);
    BuilderAddBitmap(&BuilderAssets, "bitmaps/cow1/spot3.png", Asset_CowSpot);
    
    //
    // Sounds
    //
    BuilderAddSound(&BuilderAssets, "sounds/gulp.wav", Asset_DrinkSound);
    BuilderAddSound(&BuilderAssets, "sounds/bloop_01.wav", Asset_DrinkSound);
    BuilderAddSound(&BuilderAssets, "sounds/bonk.wav", Asset_Bonk);
    BuilderAddSound(&BuilderAssets, "sounds/Bummin on Tremelo.wav", Asset_Song);
    BuilderAddSound(&BuilderAssets, "sounds/Guts and Bourbon.wav", Asset_Song);
    BuilderAddSound(&BuilderAssets, "sounds/Happy Alley.wav", Asset_Song);
    
    //
    // Fonts
    //
    BuilderAddFont(&BuilderAssets, "fonts/Rubik-Medium.ttf", Asset_Font);
    
    BuilderMakeFile(&BuilderAssets);
}

internal void
LoadAssetFile(platform_work_queue *Queue, assets *Assets)
{
    BuilderLoadFile(Assets);
}

internal bool32
PlatformKeyboardToGameController(platform_keyboard_input *Keyboard, game_controller *Controller)
{
    Controller->MoveUp = &Keyboard->W;
    Controller->MoveLeft = &Keyboard->A;
    Controller->MoveDown = &Keyboard->S;
    Controller->MoveRight = &Keyboard->D;
    
    Controller->Start = &Keyboard->Esc;
    
    return true;
}

internal bool32
PlatformControllerToGameController(platform_controller_input *PlatformController, game_controller *Controller)
{
    Controller->MoveUp = &PlatformController->DPadUp;
    Controller->MoveLeft = &PlatformController->DPadLeft;
    Controller->MoveDown = &PlatformController->DPadDown;
    Controller->MoveRight = &PlatformController->DPadRight;
    
    Controller->Start = &PlatformController->Start;
    
    return true;
}

internal PLATFORM_WORK_QUEUE_CALLBACK(RecvData)
{
    game_state *GameState = (game_state*)Data;
    CoffeeCow *CowPlayer2 = &GameState->Players[1];
    
    char Buffer[BUF_SIZE];
    int bytes = SocketqRecv(&GameState->Client, BufferC(Buffer, BUF_SIZE), BUF_SIZE);
    if (bytes > 0) {
        game_packet *PacketRecv = (game_packet*)Buffer; 
        ServerCoffeeCow *Cow = &PacketRecv->Cow;
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
}

internal PLATFORM_WORK_QUEUE_CALLBACK(SendData)
{
    game_state *GameState = (game_state*)Data;
    CoffeeCow *CowPlayer = &GameState->Players[0];
    
    game_packet PacketSend = {};
    PacketSend.Cow.TransitionAmt = CowPlayer->TransitionAmt;
    PacketSend.Cow.Score = CowPlayer->Score;
    for (int i = 0; i < CowPlayer->Nodes.Size; i++) {
        CoffeeCowNode* N = (CoffeeCowNode*)CowPlayer->Nodes[i];
        ServerCoffeeCowNode Node = {};
        Node.Coords = iv2(N->Coords);
        Node.CurrentDirection = (int8)N->CurrentDirection;
        Node.Streak = (int8)N->Streak;
        PacketSend.Cow.Nodes[i] = Node;
        PacketSend.Cow.NumOfNodes++;
    }
    PacketSend.Disconnect = GameState->Disconnect;
    
    char Buffer[BUF_SIZE];
    SocketqSend(&GameState->Client, BufferCC(Buffer, BUF_SIZE, &PacketSend, sizeof(game_packet)), SEND_BUFFER_SIZE);
}

DWORD WINAPI SendRecvFunction(LPVOID lpParam)
{
    game_state *GameState = (game_state*)lpParam;
    
    while(1) {
        WorkQueueAddEntry(GameState->Queue, RecvData, GameState);
        WorkQueueAddEntry(GameState->Queue, SendData, GameState);
        Sleep(1);
        WorkQueueCompleteAllWork(GameState->Queue);
        
        if (GameState->Disconnect == 1) {
            WorkQueueAddEntry(GameState->Queue, SendData, GameState);
            Sleep(1);
            WorkQueueCompleteAllWork(GameState->Queue);
            GameState->Disconnect = 0;
            break;
        }
    }
    
    return true;
}

struct coffee_cow_thread_params
{
    bool32 Reset;
    bool32 GameOver;
    menu_mode Menu;
    v2 GridDim;
    CoffeeCow *Cow;
    coffee_cow_collision_node *Nodes;
    real32 SecondsElapsed;
};

internal int
DoCoffeeCow(void *Data)
{
    coffee_cow_thread_params *Params = (coffee_cow_thread_params*)Data;
    if (Params->Reset)
        InitializeCow(Params->Cow, Params->GridDim);
    
    if (Params->Menu == menu_mode::pause_menu)
        return 0;
    
    if (Params->Menu != menu_mode::game_over_menu)
        CoffeeCowProcessInput(&Params->Cow->Controller, Params->Cow);
    
    if (CheckCollionCoffeeCow(Params->Cow, Params->Nodes, Params->GridDim))
        MoveCoffeeCow(Params->Cow, Params->SecondsElapsed, Params->GridDim);
    else
        Params->GameOver = true;
    
}

internal void
CoffeeCowCollisionNode(coffee_cow_collision_node *CollisionNode, CoffeeCow *Cow)
{
    CoffeeCowNode *Head = (CoffeeCowNode*)Cow->Nodes[0];
    CollisionNode->Head = Head->Coords;
    
    for (int i = 0; Cow->Nodes.Size; i++) {
        CoffeeCowNode *Node = (CoffeeCowNode*)Cow->Nodes[i];
        CollisionNode->Nodes[i] = Node->Coords;
        CollisionNode->NumNodes++;
    }
}

void UpdateRender(platform* p)
{
    game_state *GameState = MemStart(game_state);
    GameState->Queue = &p->Queue;
    
    v2 TopLeftCornerCoords = GetTopLeftCornerCoords(p);
    v2 PlatformDim = GetDim(p);
    
    camera *C = &GameState->Camera;
    qlibCoordSystem(QLIB_TOP_RIGHT);
    
    if (!GameState->Initialized)
    {
        GameState->Initialized = true;
        
        InitializeAudioState(&p->AudioState);
        p->AudioState.Assets = (void*)&GameState->Assets;
        
        C->Position = v3(0, 0, 900);
        C->Target = v3(0, 0, 0);
        C->Up = v3(0, 1, 0);
        C->FOV = 90.0f;
        C->Mode3D = false;
        
        GameState->Menu = menu_mode::main_menu;
        GameState->Game = game_mode::not_in_game;
        GameState->GridDim = v2(17, 17);
        
        MakeAssetFile(&p->Queue, &GameState->Assets);
        LoadAssetFile(&p->Queue, &GameState->Assets);
        
        SetTrue(&p->AudioState.Paused);
    }
    
    // Background Music
    if (!p->AudioState.PlayingSounds[0].Playing)
    {
        PlaySound(&p->AudioState, GetIndexSound(&GameState->Assets, Asset_Song, Random(0, 2)));
    }
    
    real32 NewGridSize = p->Dimension.Height / (GameState->GridDim.x + 6);
    if (NewGridSize != GameState->GridSize)
        GameState->GridSize = NewGridSize;
    
    // Do/Render Game
    if (GameState->Game != game_mode::not_in_game) {
        v2 RealGridDim = v2(GameState->GridDim.x * GameState->GridSize, GameState->GridDim.y * GameState->GridSize);
        v2 GridCoords = (RealGridDim * -1) / 2 + (PlatformDim/2);
        
        if (GameState->ResetGame)
        {
            for (u32  i = 0; i < GameState->NumPlayers; i++)
                InitializeCow(&GameState->Players[i], GameState->GridDim);
            
            // Init Coffee
            for (u32 i = 0; i < GameState->NumCoffees; i++) {
                Coffee *Cof = &GameState->Coffees[i];
                Cof->Rotation = 0;
                MoveCoffee(Cof, GameState->Players, GameState->NumPlayers, GameState->GridDim);
            }
            
            GameState->ResetGame = false;
        }
        
        for (u32 i = 0; i < GameState->NumPlayers; i++)
        {
            CoffeeCow *Cow = &GameState->Players[i];
            if (OnKeyDown(Cow->Controller.Start))
                MenuToggle(GameState, menu_mode::not_in_menu, menu_mode::pause_menu);
        }
        
        if (GameState->Menu != menu_mode::pause_menu)
        {
            // Only take process inputs when the game isn't in game over
            if (GameState->Menu != menu_mode::game_over_menu)
            {
                for (u32 i = 0; i < GameState->NumPlayers; i++) {
                    CoffeeCow *Cow = &GameState->Players[i];
                    CoffeeCowProcessInput(&Cow->Controller, Cow);
                }
            }
            
            // Collisions
            for (u32 i = 0; i < GameState->NumPlayers; i++)
            {
                CoffeeCow *Cow = &GameState->Players[i];
                bool32 CanMove = false;
                for (u32 j = 0; j < GameState->NumPlayers; j++)
                {
                    CoffeeCow *OtherCow = &GameState->Players[j];
                    if (CheckCollisionCoffeeCow(Cow, OtherCow, GameState->GridDim))
                        CanMove = true;
                }
                
                if (!CanMove && GameState->Menu != menu_mode::game_over_menu)
                    PlaySound(&p->AudioState, GetFirstSound(&GameState->Assets, Asset_Bonk));
                
                if (CanMove)
                    MoveCoffeeCow(Cow, p->Input.WorkSecondsElapsed, GameState->GridDim);
                else
                    SetMenu(GameState, menu_mode::game_over_menu);
            }
            
            for (u32 i = 0; i < GameState->NumPlayers; i++) {
                for (u32 j = 0; j < GameState->NumCoffees; j++) {
                    
                    CoffeeCow *Cow = &GameState->Players[i];
                    Coffee *Cof = &GameState->Coffees[j];
                    
                    if (CollectCoffee(Cow, Cof, GameState->GridDim)) {
                        MoveCoffee(Cof, GameState->Players, GameState->NumPlayers, GameState->GridDim);
                        PlaySound(&p->AudioState, GetFirstSound(&GameState->Assets, Asset_DrinkSound));
                    }
                }
            }
        }
        
        if (GameState->Game == game_mode::singleplayer) {
            CoffeeCow *Cow = &GameState->Players[0];
            if (GameState->Menu != menu_mode::pause_menu)
                DrawScore(&GameState->Assets, Cow->Score, v2(0, 0), GetDim(p));
        }
        else if (GameState->Game == game_mode::local_multiplayer) {
            
        }
        else if (GameState->Game == game_mode::multiplayer) {
            game_controller *Controller = &GameState->Players[0].Controller;
            CoffeeCow *CowPlayer = &GameState->Players[0];
            CoffeeCow *CowPlayer2 = &GameState->Players[1];
            Coffee *Collect = &GameState->Coffees[0];
            
            if (GameState->ResetGame) {
                InitializeCow(CowPlayer, GameState->GridDim);
                CowPlayer2->Nodes.Init((int)(GameState->GridDim.x * GameState->GridDim.y), sizeof(CoffeeCowNode));
                
                SocketqInit(&GameState->Client, GameState->IP, GameState->Port, TCP);
                GameState->ResetGame = false;
                GameState->Disconnect = 0;
                
                DWORD ThreadID;
                //HANDLE ThreadHandle = CreateThread(0, 0, SendRecvFunction, GameState, 0, &ThreadID);
                //CloseHandle(ThreadHandle);
                GameState->ThreadHandle = CreateThread(0, 0, SendRecvFunction, GameState, 0, &ThreadID);
            }
            
            if (OnKeyDown(Controller->Start))
                MenuToggle(GameState, menu_mode::not_in_menu, menu_mode::pause_menu);
            
            if (GameState->Menu != menu_mode::pause_menu) {
                CoffeeCowProcessInput(Controller, CowPlayer);
                
                if (CheckCollisionCoffeeCow(CowPlayer, CowPlayer, GameState->GridDim))
                    MoveCoffeeCow(CowPlayer, p->Input.WorkSecondsElapsed, GameState->GridDim);
                else
                    SetMenu(GameState, menu_mode::game_over_menu);
                
                CollectCoffee(CowPlayer, Collect, GameState->GridDim);
                
                v2 RealGridDim = v2(GameState->GridDim.x * GameState->GridSize, GameState->GridDim.y * GameState->GridSize);
                v2 GridCoords = (RealGridDim * -1) / 2;
                
                
                DrawCoffeeCow(CowPlayer, Collect->Coords, p->Input.WorkSecondsElapsed, GridCoords.x, GridCoords.y, GameState->GridSize);
                DrawCoffeeCow(CowPlayer2, Collect->Coords, p->Input.WorkSecondsElapsed, GridCoords.x, GridCoords.y, GameState->GridSize);
                DrawCoffee(Collect, GetFirstBitmap(&GameState->Assets, Asset_Coffee), p->Input.WorkSecondsElapsed, GridCoords, GameState->GridSize, 0.1f);
                //DrawScore(&GameState->Assets, CowPlayer->Score, TopLeftCornerCoords, GetDim(p));
            }
        }
        
        DrawEnvironment(GetFirstBitmap(&GameState->Assets, Asset_Background),
                        GetFirstBitmap(&GameState->Assets, Asset_Grass),
                        GetFirstBitmap(&GameState->Assets, Asset_Grid),
                        GetFirstBitmap(&GameState->Assets, Asset_Rock),
                        v2(0, 0),
                        PlatformDim,
                        GridCoords,
                        GameState->GridDim,
                        RealGridDim,
                        GameState->GridSize,
                        -1.0f);
        
        for (u32 i = 0; i < GameState->NumPlayers; i++)
            DrawCoffeeCow(&GameState->Players[i], GameState->Coffees[0].Coords, &GameState->Assets, GameState->Players[i].Tag, p->Input.WorkSecondsElapsed, GridCoords.x, GridCoords.y, GameState->GridSize);
        for (u32 i = 0; i < GameState->NumCoffees; i++)
            DrawCoffee(&GameState->Coffees[i], GetFirstBitmap(&GameState->Assets, Asset_Coffee), p->Input.WorkSecondsElapsed, GridCoords, GameState->GridSize, 0.1f);
        
    }
    // End of Do/Render Game
    
    // Do/Render Menu
    if (GameState->Menu != menu_mode::not_in_menu) {
        
        if (GameState->Menu == menu_mode::main_menu) {
            enum menu_component_id
            {
                MCI_Singleplayer,
                MCI_LocalMultiplayer,
                MCI_Multiplayer,
                MCI_Quit,
                MCI_Count
            };
            pair_int_string IDs[] = {
                pairintstring(MCI_Singleplayer),
                pairintstring(MCI_LocalMultiplayer),
                pairintstring(MCI_Multiplayer),
                pairintstring(MCI_Quit),
                pairintstring(MCI_Count)
            };
            
            menu *Menu = GetMenu(GameState, menu_mode::main_menu);
            if (DoMenu(Menu, "menus/main.menu", p, &GameState->Assets, IDs, MCI_Count)) 
            {
                if (Menu->Events.ButtonClicked == MCI_Singleplayer) {
                    PlatformSetCursorMode(&p->Input.Mouse, platform_cursor_mode::Arrow);
                    SetMenu(GameState, menu_mode::not_in_menu);
                    GameState->ResetGame = true;
                    GameState->Game = game_mode::singleplayer;
                    GameState->NumPlayers = 1;
                    GameState->NumCoffees = 1;
                    
                    switch(ActiveInputType(p->Input.ActiveInput))
                    {
                        case Keyboard:
                        case Mouse: {
                            PlatformKeyboardToGameController(&p->Input.Keyboard, &GameState->Players[0].Controller);
                        } break;
                        case Controller: {
                            PlatformControllerToGameController(GetActiveController(&p->Input), &GameState->Players[0].Controller);
                        }
                    }
                    GameState->Players[0].Tag = Tag_Player1;
                }
                else if (Menu->Events.ButtonClicked == MCI_Multiplayer) {
                    SetMenu(GameState, menu_mode::multiplayer_menu);
                }
                else if (Menu->Events.ButtonClicked == MCI_LocalMultiplayer) {
                    SetMenu(GameState, menu_mode::local_multiplayer_menu);
                }
                else if (Menu->Events.ButtonClicked == MCI_Quit)
                    p->Quit = true;
            }
            
            DrawMenu(Menu, v2(0, 0), GetDim(p), 100.0f);
        }
        else if (GameState->Menu == menu_mode::multiplayer_menu) {
            enum menu_component_id
            {
                MCI_IP,
                MCI_Port,
                MCI_Join,
                MCI_Back,
                MCI_PortText,
                MCI_Count
            };
            pair_int_string IDs[] = {
                pairintstring(MCI_IP),
                pairintstring(MCI_Port),
                pairintstring(MCI_Join),
                pairintstring(MCI_Back),
                pairintstring(MCI_PortText),
                pairintstring(MCI_Count)
            };
            
            menu *Menu = GetMenu(GameState, menu_mode::multiplayer_menu);
            if (DoMenu(Menu, "menus/online_multiplayer.menu", p, &GameState->Assets, IDs, MCI_Count))
            {
                if (Menu->Events.ButtonClicked == MCI_Join) {
                    PlatformSetCursorMode(&p->Input.Mouse, platform_cursor_mode::Arrow);
                    GameState->ResetGame = true;
                    SetMenu(GameState, menu_mode::not_in_menu);
                    GameState->Game = game_mode::multiplayer;
                    
                    GameState->IP = MenuGetTextBoxText(Menu, MCI_IP);
                    GameState->Port = MenuGetTextBoxText(Menu, MCI_Port);
                }
                else if (Menu->Events.ButtonClicked == MCI_Back) {
                    SetMenu(GameState, menu_mode::main_menu);
                }
                
                if (Menu->ScreenDim != GetDim(p)) {
                    ResizeMenu(Menu, GetDim(p), &GameState->Assets);
                    UpdateMenu(Menu);
                }
            }
            DrawMenu(Menu, v2(0, 0), GetDim(p), 100.0f);
        }
        else if (GameState->Menu == menu_mode::pause_menu) {
            
            enum menu_component_id
            {
                MCI_Reset,
                MCI_Menu,
                MCI_Count
            };
            pair_int_string IDs[] = {
                pairintstring(MCI_Reset),
                pairintstring(MCI_Menu),
                pairintstring(MCI_Count)
            };
            
            menu *Menu = GetMenu(GameState, menu_mode::pause_menu);
            if (DoMenu(Menu, "menus/pause.menu", p, &GameState->Assets, IDs, MCI_Count))
            {
                if (Menu->Events.ButtonClicked == MCI_Menu) {
                    GameState->Game = game_mode::not_in_game;
                    SetMenu(GameState, menu_mode::main_menu);
                }
                else if (Menu->Events.ButtonClicked == MCI_Reset) {
                    GameState->ResetGame = true;
                    GameState->Game = game_mode::singleplayer;
                    SetMenu(GameState, menu_mode::not_in_menu);
                }
            }
            
            DrawMenu(Menu, v2(0, 0), GetDim(p), 100.0f);
        }
        else if (GameState->Menu == menu_mode::game_over_menu) {
            
            enum menu_component_id
            {
                MCI_Reset,
                MCI_Menu,
                MCI_Count
            };
            pair_int_string IDs[] = {
                pairintstring(MCI_Reset),
                pairintstring(MCI_Menu),
                pairintstring(MCI_Count)
            };
            
            menu *Menu = GetMenu(GameState, menu_mode::game_over_menu);
            if (DoMenu(Menu, "menus/game_over.menu", p, &GameState->Assets, IDs, MCI_Count))
            {
                if (Menu->Events.ButtonClicked == MCI_Menu) {
                    GameState->Game = game_mode::not_in_game;
                    SetMenu(GameState, menu_mode::main_menu);
                    
                }
                else if (Menu->Events.ButtonClicked == MCI_Reset) {
                    GameState->ResetGame = true;
                    SetMenu(GameState, menu_mode::not_in_menu);
                }
            }
            
            DrawMenu(Menu, 100.0f);
        }
        else if (GameState->Menu == menu_mode::local_multiplayer_menu) {
            enum menu_component_id
            {
                MCI_Default,
                MCI_Start,
                MCI_Back,
                PI_Player1, //3
                PI_Player2, //4
                PI_Player3, //5
                PI_Player4, //6
                
                MCI_Count
            };
            pair_int_string IDs[] = {
                pairintstring(MCI_Default),
                pairintstring(MCI_Start),
                pairintstring(MCI_Back),
                pairintstring(PI_Player1),
                pairintstring(PI_Player2),
                pairintstring(PI_Player3),
                pairintstring(PI_Player4),
            };
            
            menu *Menu = GetMenu(GameState, menu_mode::local_multiplayer_menu);
            if (DoMenu(Menu, "menus/local_multiplayer.menu", p, &GameState->Assets, IDs, MCI_Count))
            {
                if (Menu->Events.ButtonClicked == MCI_Start) {
                    GameState->NumPlayers = 0;
                    for (int i = 3; i < 7; i++) {
                        menu_component *Component = MenuGetComponent(Menu->CheckBoxes, i);
                        menu_component_checkbox *CheckBox = (menu_component_checkbox*)Component->Data;
                        if (CheckBox->Clicked)
                        {
                            GameState->Players[GameState->NumPlayers].Tag = Tag_Player1;
                            switch(ActiveInputType(CheckBox->ControllerIndex))
                            {
                                case Keyboard:
                                case Mouse: {
                                    PlatformKeyboardToGameController(&p->Input.Keyboard, &GameState->Players[GameState->NumPlayers].Controller);
                                } break;
                                case Controller: {
                                    PlatformControllerToGameController(&p->Input.Controllers[CheckBox->ControllerIndex - 2], &GameState->Players[GameState->NumPlayers].Controller);
                                }
                            }
                            GameState->NumPlayers++;
                        }
                    }
                    if (GameState->NumPlayers > 1)
                    {
                        PlatformSetCursorMode(&p->Input.Mouse, platform_cursor_mode::Arrow);
                        SetMenu(GameState, menu_mode::not_in_menu);
                        GameState->ResetGame = true;
                        GameState->Game = game_mode::local_multiplayer;
                    }
                }
                else if (Menu->Events.ButtonClicked == MCI_Back) {
                    SetMenu(GameState, menu_mode::main_menu);
                }
            }
            DrawMenu(Menu, v2(0, 0), GetDim(p), 100.0f);
        }
    }
    // End of Do/Render Menu
    
    
    platform_keyboard_input *Keyboard = &p->Input.Keyboard;
    if (OnKeyDown(&Keyboard->F5))
        GameState->ShowFPS = !GameState->ShowFPS;
    
    if (GameState->ShowFPS) {
        DrawFPS(p->Input.MillisecondsElapsed, GetDim(p), GetFont(&GameState->Assets, GetFirstFont(&GameState->Assets, Asset_Font)));
        //GUIAdd();
    }
    
    if (GUIRun(&GameState->Status))
    {
        
    }
    
    C->PlatformDim = v2(p->Dimension.Width, p->Dimension.Height);
    
    RenderPieceGroup(C, &GameState->Assets);
}
