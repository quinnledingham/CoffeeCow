/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Casey Muratori $
   $Notice: (C) Copyright 2014 by Molly Rocket, Inc. All Rights Reserved. $
   ======================================================================== */

#include "snake.h"

internal void
FilenameSearchModify(char* filename, char* result)
{
    int j = 0;
    
    char* cursor = filename;
    while (*cursor != 0)
    {
        if (*cursor == '.')
        {
            result[j] = '_';
        }
        else
        {
            result[j] = *cursor;
        }
        cursor++;
        j++;
    }
    result[j] = 0;
}

internal void
FilenameCapitalize(char* filename, char* result)
{
    int j = 0;
    
    char* cursor = filename;
    while (*cursor != 0)
    {
        if (*cursor == '_' || *cursor == '.')
        {
            result[j] = '_';
        }
        else
        {
            char c = *cursor;
            result[j] = c - 32;
        }
        cursor++;
        j++;
    }
    result[j] = 0;
}

#include <math.h>
inline int32
RoundReal32ToInt32(real32 Real32)
{
    int32 Result = (int32)roundf(Real32);
    return(Result);
}

unsigned long createRGBA(int r, int g, int b, int a)
{   
    return ((a & 0xff) << 24) + ((r & 0xff) << 16) + ((g & 0xff) << 8) + ((b & 0xff));
}

unsigned long createRGB(int r, int g, int b)
{   
    return ((r & 0xff) << 16) + ((g & 0xff) << 8) + (b & 0xff);
}

#include "stdio.h"

#if !defined(RAYLIB_H)
#include "text.h"
#endif

#include "gui.h"
#include "memorymanager.cpp" 
#include "memorymanager.h"

#if !defined(RAYLIB_H)
#include "text.cpp"
#include "renderer.cpp"
#include "image.cpp"
#endif


#include "gui.cpp"

//#include "socketq.h"

//#include "socketq.cpp"
//#include "socketq.h"



internal void
GameOutputSound(game_sound_output_buffer *SoundBuffer, int ToneHz)
{
    local_persist real32 tSine;
    int16 ToneVolume = 3000;
    int WavePeriod = SoundBuffer->SamplesPerSecond/ToneHz;
    
    int16 *SampleOut = SoundBuffer->Samples;
    for(int SampleIndex = 0;
        SampleIndex < SoundBuffer->SampleCount;
        ++SampleIndex)
    {
        // TODO(casey): Draw this out for people
        real32 SineValue = sinf(tSine);
        int16 SampleValue = (int16)(SineValue * ToneVolume);
        *SampleOut++ = SampleValue;
        *SampleOut++ = SampleValue;
        
        tSine += 2.0f*Pi32*1.0f/(real32)WavePeriod;
    }
}

global_variable int GameInitialized = false;
global_variable Snake Player;
global_variable GUI menu = {};
global_variable NewGUI MainMenu = {};
global_variable NewGUI LoseMenu = {};

internal void 
NewAddSnakeNode(Snake *S, float X, float Y)
{
    if (S->Head == 0)
    {
        SnakeNode part1 = {};
        part1.X = X;
        part1.Y = Y;
        
        S->Head = (SnakeNode*)PermanentStorageAssign(&part1, sizeof(SnakeNode));
        return;
    }
    
    SnakeNode* cursor = S->Head;
    while(cursor->Next != 0)
    {
        cursor = cursor->Next;
    }
    SnakeNode partnew = {};
    partnew.X = X;
    partnew.Y = Y;
    partnew.Previous = cursor;
    partnew.Direction = partnew.Previous->Direction;
    partnew.NextDirection = partnew.Previous->NextDirection;
    
    cursor->Next = (SnakeNode*)PermanentStorageAssign(&partnew, sizeof(SnakeNode));
}


internal void
NewInitializeSnake(Snake *S)
{
    S->Direction = RIGHT;
    S->Length = 3;
    
    NewAddSnakeNode(S, 2, 2);
    NewAddSnakeNode(S, 1, 2);
    NewAddSnakeNode(S, 0, 2);
}

global_variable Image test;
global_variable Image right;
global_variable Image up;
global_variable Image left;
global_variable Image down;

internal void
NewRenderSnake(Snake* s, int  GridX, int  GridY, 
               int GridWidth, int GridHeight, int GridSize)
{
    SnakeNode* cursor = s->Head;
    while(cursor != 0)
    {
        
        if (cursor == s->Head || cursor->Next == 0)
        {
            
            int CursorX = (int)(cursor->X * GridSize);
            int CursorY = (int)(cursor->Y * GridSize);
            /*
            Circle NewHalfCircle =
            {
                GridX + CursorX,
                GridY + CursorY,
                GridSize / 2,
                0
            };
            RenderCircle(&NewHalfCircle, FILL, 0xFFFF5E5E);
            Circle Eye1 = {};
            int X = 0;
            int Y = 0;
            int Width = 0;
            int Height = 0;
            if (cursor->Direction == RIGHT || cursor->Direction == LEFT)
            {
                Width = GridSize / 2;
                Height = GridSize;
            }
            else if (cursor->Direction == UP || cursor->Direction == DOWN)
            {
                Width = GridSize;
                Height = GridSize / 2;
            }
            
            if (cursor == s->Head)
            {
                if (s->Head->Direction == RIGHT)
                {
                    X = 0;
                    Y = 0;
                }
                else if (s->Head->Direction == LEFT)
                {
                    X = 1;
                    Y = 0;
                }
                else if (s->Head->Direction == UP)
                {
                    X = 0;
                    Y = 1;
                }
                else if (s->Head->Direction == DOWN)
                {
                    X = 0;
                    Y = 0;
                }
                
                Eye1  =
                {
                    GridX + CursorX + (X * GridSize),
                    GridY + CursorY + (Y * GridSize),
                    GridSize / 5,
                    0
                };
            }
            else
            {
                if (cursor->Direction == RIGHT)
                {
                    X = 1;
                    Y = 0;
                }
                else if (cursor->Direction == LEFT)
                {
                    X = 0;
                    Y = 0;
                }
                else if (cursor->Direction == UP)
                {
                    X = 0;
                    Y = 0;
                }
                else if (cursor->Direction == DOWN)
                {
                    X = 0;
                    Y = 1;
                }
            }
            */
            Rect newSquare =
            {
                GridX + CursorX + (0 * (GRIDSIZE / 2)), 
                GridY + CursorY + (0 * (GRIDSIZE / 2)), 
                GRIDSIZE, GRIDSIZE
            };
            //RenderRect(&newSquare, FILL, 0xFFFF5E5E);
            if (cursor == s->Head)
            {
                loaded_bitmap BMP = {};
                BMP.Width = GridSize;
                BMP.Height = GridSize;
                BMP.Pitch = BITMAP_BYTES_PER_PIXEL * BMP.Width;
                
                if (cursor->Direction == RIGHT)
                {
                    BMP.Memory = right.data;
                }
                else if (cursor->Direction == LEFT)
                {
                    BMP.Memory = left.data;
                }
                else if (cursor->Direction == UP)
                {
                    BMP.Memory = down.data;
                }
                else if (cursor->Direction == DOWN)
                {
                    BMP.Memory = up.data;
                }
                RenderBitmap(&BMP, (real32)(GridX + CursorX), (real32)(GridY + CursorY));
                
            }
            
            //RenderCircle(&Eye1, FILL, 0xFFFFFFFF);
        }
        else
        {
            int CursorX = (int)(cursor->X * GridSize);
            int CursorY = (int)(cursor->Y * GridSize);
            Rect newSquare =
            {
                GridX + CursorX, 
                GridY + CursorY, 
                GridSize, GridSize
            };
            
            RenderRect(&newSquare, FILL, 0xFFFF5E5E);
        }
        cursor = cursor->Next;
    }
}

internal void
RenderApple(Apple* A, int GridX, int GridY, 
            int GridWidth, int GridHeight, int GridSize)
{
    Rect newSquare =
    {
        GridX + (A->X * GridSize), 
        GridY + (A->Y * GridSize), 
        GridSize, GridSize
    };
    RenderRect(&newSquare, FILL, 0xFFff4040);
}

internal void
NewTransitionSnake(Snake* snake, float SecondsElapsed)
{
    float Dm = (snake->Speed * SecondsElapsed);
    float Dp = Dm;
    
    snake->DistanceTravelled += Dp;
    
    SnakeNode* cursor = snake->Head;
    while(cursor != 0)
    {
        if (cursor->Direction == RIGHT)
        {
            cursor->X += Dp;
        }
        else if (cursor->Direction == UP)
        {
            cursor->Y -= Dp;
        }
        else if (cursor->Direction == LEFT)
        {
            cursor->X -= Dp;
        }
        else if (cursor->Direction == DOWN)
        {
            cursor->Y += Dp;
        }
        cursor = cursor->Next;
    }
}

internal void
NewMoveSnake(Snake* snake)
{
    SnakeNode* cursor = snake->Head;
    
    if (snake->NextDirection == -1)
    {
        cursor->Direction = snake->Direction;
    }
    else
    {
        snake->Direction = snake->NextDirection;
        cursor->Direction = snake->NextDirection;
        snake->NextDirection = -1;
    }
    snake->LastMoveDirection = cursor->Direction;
    
    /*
    char FPSBuffer[256];
    _snprintf_s(FPSBuffer, sizeof(FPSBuffer),
                "X:%.02f Y:%.02f\n", cursor->X, cursor->Y);
    OutputDebugStringA(FPSBuffer);
    */
    cursor->X = (real32)(RoundReal32ToInt32(cursor->X));
    cursor->Y = (real32)(RoundReal32ToInt32(cursor->Y));
    cursor = cursor->Next;
    
    while(cursor != 0)
    {
        cursor->X = (real32)(RoundReal32ToInt32(cursor->X));
        cursor->Y = (real32)(RoundReal32ToInt32(cursor->Y));
        
        cursor->Direction = cursor->NextDirection;
        cursor->NextDirection = cursor->Previous->Direction;
        cursor = cursor->Next;
    }
}

internal void
CheckGetApple(Snake* S, Apple* A)
{
    if(S->Head->X == A->X && S->Head->Y == A->Y)
    {
        A->Score++;
        S->Length++;
        
        SnakeNode* Cursor = S->Head;
        while(Cursor->Next != 0)
        {
            Cursor = Cursor->Next;
        }
        
        if (Cursor->NextDirection == RIGHT)
            NewAddSnakeNode(S, Cursor->X - 1, Cursor->Y);
        if (Cursor->NextDirection == UP)
            NewAddSnakeNode(S, Cursor->X, Cursor->Y + 1);
        if (Cursor->NextDirection == LEFT)
            NewAddSnakeNode(S, Cursor->X + 1, Cursor->Y);
        if (Cursor->NextDirection == DOWN)
            NewAddSnakeNode(S, Cursor->X, Cursor->Y - 1);
        
        int Good = 0;
        while(!Good)
        {
            A->X = rand() % 17;
            A->Y = rand() % 17;
            Good = 1;
            
            Cursor = S->Head;
            while(Cursor != 0)
            {
                if (Cursor->X == A->X && Cursor->Y == A->Y)
                {
                    Good = 0;
                }
                Cursor = Cursor->Next;
            }
        }
    }
}

internal int
CheckBounds(Snake* snake, int GridWidth, int GridHeight)
{
    if (snake->Head->X  == 0 && snake->Direction == LEFT)
    {
        snake->Speed = 0;
    }
    else if(snake->Head->X + 1 == GridWidth && snake->Direction == RIGHT)
    {
        snake->Speed = 0;
    }
    else if (snake->Head->Y == 0 && snake->Direction == UP)
    {
        snake->Speed = 0;
    }
    else if (snake->Head->Y == GridHeight - 1 && snake->Direction == DOWN)
    {
        snake->Speed = 0;
    }
    else
    {
        snake->Speed = snake->MaxSpeed;
        return 1;
    }
    
    return 0;
}


entire_file
ReadEntireFile(char *FileName)
{
    entire_file Result = {};
    
    FILE *In = fopen(FileName, "rb");
    if(In)
    {
        fseek(In, 0, SEEK_END);
        Result.ContentsSize = ftell(In);
        fseek(In, 0, SEEK_SET);
        
        Result.Contents = malloc(Result.ContentsSize);
        fread(Result.Contents, Result.ContentsSize, 1, In);
        fclose(In);
    }
    else
    {
        printf("ERROR: Cannot open file %s.\n", FileName);
    }
    
    return(Result);
}


loaded_bitmap yo;
//Client client;

real32 BackspaceTime = 0;
int Backspace = 0;

Font Faune50 = {};
Font Faune100 = {};

real32 SnakeTimeCounter = 0;

Apple App = {};


#include "../data/imagesaves/grass.h"
#include "../data/imagesaves/grass.cpp"
#include "../data/imagesaves/right.h"
#include "../data/imagesaves/right.cpp"
#include "../data/imagesaves/up.h"
#include "../data/imagesaves/up.cpp"
#include "../data/imagesaves/left.h"
#include "../data/imagesaves/left.cpp"
#include "../data/imagesaves/down.h"
#include "../data/imagesaves/down.cpp"

#include "../data/imagesaves/faunefifty.h"
#include "../data/imagesaves/fauneonehundred.h"

internal void
GameUpdateAndRender(game_memory *Memory, game_input *Input, game_offscreen_buffer *Buffer,
                    game_sound_output_buffer *SoundBuffer)
{
    Assert((&Input->Controllers[0].Terminator - &Input->Controllers[0].Buttons[0]) ==
           (ArrayCount(Input->Controllers[0].Buttons)));
    Assert(sizeof(game_state) <= Memory->PermanentStorageSize);
    
    game_state *GameState = (game_state *)Memory->PermanentStorage;
    
    int centeredX = (Buffer->Width - (GRIDWIDTH * GRIDSIZE)) / 2;
    int centeredY = (Buffer->Height - (GRIDHEIGHT * GRIDSIZE)) / 2;
    //Rect imageRect = {centeredX, centeredY, (GRIDWIDTH * GRIDSIZE), (GRIDHEIGHT * GRIDSIZE), 0};
    Rect imageRect = {centeredX, centeredY, (GRIDSIZE), (GRIDSIZE), 0};
    
    if(!Memory->IsInitialized)
    {
        
#if SNAKE_INTERNAL
        char *Filename = __FILE__;
        debug_read_file_result File = DEBUGPlatformReadEntireFile(Filename);
        if(File.Contents)
        {
            DEBUGPlatformWriteEntireFile("test.out", File.ContentsSize, File.Contents);
            DEBUGPlatformFreeFileMemory(File.Contents);
        }
#endif
        
        GameState->ToneHz = 256;
        GameState->Menu = 1;
        
        manager.NextStorage = (char*)Memory->PermanentStorage + sizeof(game_state);
        
#if !defined(RAYLIB_H)
#if SAVE_IMAGES
        test = LoadImageResize("grass.jpg", GRIDSIZE, GRIDSIZE, "grass.h");
        right = LoadImageResize("right.png", GRIDSIZE, GRIDSIZE, "right.h");
        up = LoadImageResize("up.png", GRIDSIZE, GRIDSIZE, "up.h");
        left = LoadImageResize("left.png", GRIDSIZE, GRIDSIZE, "left.h");
        down = LoadImageResize("down.png", GRIDSIZE, GRIDSIZE, "down.h");
#else
        LoadImageFromgrass_h(&test);
        LoadImageFromright_h(&right);
        LoadImageFromup_h(&up);
        LoadImageFromleft_h(&left);
        LoadImageFromdown_h(&down);
#endif
#else
        testTexture = LoadImageResize("grass.jpg", GRIDSIZE, GRIDSIZE);
#endif
        //loaded_bitmap grass = LoadBMP("grass.png");
        // Where the top left corner of the grid should be for it to look
        // centered in the window
        
#if !defined(RAYLIB_H)
#if SAVE_IMAGES
        
        Faune50 = LoadEntireFont("Faune-TextRegular.otf", 50);
        Faune100 = LoadEntireFont("Faune-TextRegular.otf", 100);
        SaveFontToHeaderFile("FAUNE50", "faunefifty.h", "imagesaves/faunefifty.h" ,&Faune50);
        SaveFontToHeaderFile("FAUNE100", "fauneonehundred.h", "imagesaves/fauneonehundred.h" ,&Faune100);
#else
        LoadFontFromHeaderFile(&Faune50, FAUNE50stbtt, FAUNE50stbttuserdata, FAUNE50stbttdata, FAUNE50Size,
                               FAUNE50Ascent, FAUNE50Scale, FAUNE50FontChar, FAUNE50FontCharMemory);
        LoadFontFromHeaderFile(&Faune100, FAUNE100stbtt, FAUNE100stbttuserdata, FAUNE100stbttdata, FAUNE100Size,
                               FAUNE100Ascent, FAUNE100Scale, FAUNE100FontChar, FAUNE100FontCharMemory);
#endif
#else 
        Faune100 = LoadFontEx("Faune-TextRegular.otf", 100, 0, 255);
        Faune50 = LoadFontEx("Faune-TextRegular.otf", 50, 0, 255);
#endif
        
        
        // TODO(casey): This may be more appropriate to do in the platform layer
        Memory->IsInitialized = true;
    }
    
    int intFPS = (int)(1/Input->SecondsElapsed);
    char FPS[10]; 
    sprintf(FPS, "%d", intFPS);
    Rect FPSRect = {10, 10, 100, 50, 0};
    
    int btnPress = -1;
    int tbPress = -1;
    
    UpdateNewGUI(&MainMenu, Buffer->Width, Buffer->Height);
    
    for(int ControllerIndex = 0;
        ControllerIndex < ArrayCount(Input->Controllers);
        ++ControllerIndex)
    {
        game_controller_input *Controller = GetController(Input, ControllerIndex);
        if(Controller->IsAnalog)
        {
            // NOTE(casey): Use analog movement tuning
            GameState->BlueOffset += (int)(4.0f*Controller->StickAverageX);
            GameState->ToneHz = 256 + (int)(128.0f*Controller->StickAverageY);
        }
        else
        {
            // NOTE(casey): Use digital movement tuning
            if(Controller->MoveLeft.NewEndedDown)
            {
                if (LEFT  == Player.LastMoveDirection)
                {
                    if (Player.LastMoveDirection != RIGHT)
                        Player.Direction = LEFT;
                    
                    Player.NextDirection = -1;
                }
                else
                {
                    if (Player.LastMoveDirection != RIGHT)
                        Player.NextDirection = LEFT;
                }
            }
            
            if(Controller->MoveRight.NewEndedDown)
            {
                if (RIGHT  == Player.LastMoveDirection)
                {
                    if (Player.LastMoveDirection != LEFT)
                        Player.Direction = RIGHT;
                    
                    Player.NextDirection = -1;
                }
                else
                {
                    if (Player.LastMoveDirection != LEFT)
                        Player.NextDirection = RIGHT;
                }
            }
            
            if(Controller->MoveDown.NewEndedDown)
            {
                if (DOWN == Player.LastMoveDirection)
                {
                    if (Player.LastMoveDirection != UP)
                        Player.Direction = DOWN;
                    
                    Player.NextDirection = -1;
                }
                else
                {
                    if (Player.LastMoveDirection != UP)
                        Player.NextDirection = DOWN;
                }
            }
            
            if(Controller->MoveUp.NewEndedDown)
            {
                if (UP == Player.LastMoveDirection)
                {
                    if (Player.LastMoveDirection != DOWN)
                        Player.Direction = UP;
                    
                    Player.NextDirection = -1;
                }
                else
                {
                    if (Player.LastMoveDirection != DOWN)
                        Player.NextDirection = UP;
                }
            }
            //sprintf(FPS, "%d", Player.Direction);
            //PrintOnScreen(Buffer, &Faune50, FPS, FPSRect.x, FPSRect.y, 0xFF000000, &FPSRect);
            
            if(Input->MouseButtons[0].EndedDown)
            {
                btnPress = CheckButtonsClick(&MainMenu, Input->MouseX, Input->MouseY);
                
                tbPress = CheckTextBoxes(&MainMenu, Input->MouseX, Input->MouseY);
            }
            
            if(Controller->Zero.NewEndedDown)
            {
                AddCharTextBoxText(&MainMenu, "0");
            }
            if(Controller->One.NewEndedDown)
            {
                AddCharTextBoxText(&MainMenu, "1");
            }
            if(Controller->Two.NewEndedDown)
            {
                AddCharTextBoxText(&MainMenu, "2");
            }
            if(Controller->Three.NewEndedDown)
            {
                AddCharTextBoxText(&MainMenu, "3");
            }
            if(Controller->Four.NewEndedDown)
            {
                AddCharTextBoxText(&MainMenu, "4");
            }
            if(Controller->Five.NewEndedDown)
            {
                AddCharTextBoxText(&MainMenu, "5");
            }
            if(Controller->Six.NewEndedDown)
            {
                AddCharTextBoxText(&MainMenu, "6");
            }
            if(Controller->Seven.NewEndedDown)
            {
                AddCharTextBoxText(&MainMenu, "7");
            }
            if(Controller->Eight.NewEndedDown)
            {
                AddCharTextBoxText(&MainMenu, "8");
            }
            if(Controller->Nine.NewEndedDown)
            {
                AddCharTextBoxText(&MainMenu, "9");
            }
            if(Controller->Period.NewEndedDown)
            {
                AddCharTextBoxText(&MainMenu, ".");
            }
            if(Controller->Back.NewEndedDown)
            {
                RemoveCharTextBoxText(&MainMenu);
            }
            
        }
    }
    
    // TODO(casey): Allow sample offsets here for more robust platform options
    //GameOutputSound(SoundBuffer, GameState->ToneHz);
    
    if (btnPress == GameStart)
    {
        GameState->Menu = 0;
    }
    else if (btnPress == Quit)
    {
        Input->quit = 1;
    }
    
    if (tbPress == IP)
    {
        ChangeTextBoxShowCursor(&MainMenu, IP);
    }
    if (tbPress == PORT)
    {
        ChangeTextBoxShowCursor(&MainMenu, PORT);
    }
    if (GameState->Menu == 2)
    {
        if (LoseMenu.Initialized == 0)
        {
            LoseMenu.Padding = 0;
            
            int Y = 0;
            
            NewText txt = 
            {
                "YOU LOST",    // Text
                Btn1,       // ID
                &Faune100,   // Font
                0xFF000000, // TextColor
            };
            AddNewText(&LoseMenu, 0, Y++,  &txt);
            
            InitializeNewGUI(&LoseMenu);
            MainMenu.Initialized = 1;
        }
        
        CheckButtonsHover(&LoseMenu, Input->MouseX, Input->MouseY);
        
        //ClearScreen();
        RenderNewGUI(Buffer, &LoseMenu);
    }
    else if (GameState->Menu == 1)
    {
        
        if (MainMenu.Initialized == 0)
        {
            MainMenu.Padding = 10;
            
            int Y = 0;
            
            NewText TXT = {};
            NewButton btn = {};
            NewTextBox tb = {};
            
            TXT.Text = "SINGLEPLAYER";
            TXT.ID = Btn1;
            TXT.FontType = &Faune100;
            TXT.TextColor = 0xFF000000;
            AddNewText(&MainMenu, 0, Y++,  &TXT);
            
            btn = 
            {
                0,
                0,
                "START",    // Text
                &Faune100,   // Font
                GameStart,       // ID
                0,          // Color (CurrentColor)
                0xFF32a89b, // RegularColor
                0xFFeba434, // HoverColor
                0xFFFFFFFF, // TextColor
            };
            AddNewButton(&MainMenu, 0, Y++, 300, 100, &btn);
            
            TXT = 
            {
                "0",    // Text
                Btn1,       // ID
                &Faune100,   // Font
                0xFFFFFFFF, // TextColor
            };
            AddNewText(&MainMenu, 0, Y++,  &TXT);
            
            TXT = 
            {
                "MULTIPLAYER",    // Text
                Btn1,       // ID
                &Faune100,   // Font
                0xFF000000, // TextColor
            };
            AddNewText(&MainMenu, 0, Y++,  &TXT);
            
            TXT = 
            {
                "IP:",    // Text
                Btn1,       // ID
                &Faune50,   // Font
                0xFF000000, // TextColor
            };
            AddNewText(&MainMenu, 0, Y,  &TXT);
            
            tb =
            {
                0,
                0,
                "",
                &Faune50,
                IP,
                0,
                0xFFb3b3b3,
                0xFF000000
            };
            AddNewTextBox(&MainMenu, 1, Y++, 500, 50, &tb);
            
            TXT = 
            {
                "PORT:",    // Text
                Btn1,       // ID
                &Faune50,   // Font
                0xFF000000, // TextColor
            };
            AddNewText(&MainMenu, 0, Y,  &TXT);
            
            tb =
            {
                0,
                0,
                "",
                &Faune50,
                PORT,
                0,
                0xFFb3b3b3,
                0xFF000000
            };
            AddNewTextBox(&MainMenu, 1, Y++, 500, 50, &tb);
            
            btn = 
            {
                0,
                0,
                "JOIN",    // Text
                &Faune100,   // Font
                Btn4,       // ID
                0,          // Color (CurrentColor)
                0xFF32a89b, // RegularColor
                0xFFeba434, // HoverColor
                0xFFFFFFFF, // TextColor
            };
            AddNewButton(&MainMenu, 0, Y++, 200, 100, &btn);
            
            btn = 
            {
                0,
                0,
                "QUIT",    // Text
                &Faune100,   // Font
                Quit,       // ID
                0,          // Color (CurrentColor)
                0xFF32a89b, // RegularColor
                0xFFeba434, // HoverColor
                0xFFFFFFFF, // TextColor
            };
            AddNewButton(&MainMenu, 0, Y++, 200, 100, &btn);
            
            InitializeNewGUI(&MainMenu);
            MainMenu.Initialized = 1;
        }
        
        CheckButtonsHover(&MainMenu, Input->MouseX, Input->MouseY);
        
        
#if !defined(RAYLIB_H)
        ClearScreen();
        RenderNewGUI(Buffer, &MainMenu);
#else
        BeginDrawing();
        ClearBackground(WHITE);
        RenderNewGUI(Buffer, &MainMenu);
        EndDrawing();
#endif
        //Clear();
        //RenderTriangle();
    }
    else if (GameState->Menu == 0)
    {
        if (GameInitialized == false)
        {
            NewInitializeSnake(&Player);
            GameInitialized = true;
            
            App.X = rand() % 17;
            App.Y = rand() % 17;
            
            Player.MaxSpeed = 9.0f; // m/s
            Player.Speed = Player.MaxSpeed;
            //createClient(&client, "192.168.1.75", "10109", TCP);
        }
        
        
        //Worked to send messages to the test server
        /*
        sendq(&client, "yo", 50);
        
        char buffer[500];
        memset(buffer, 0, sizeof(buffer));
        recvq(&client, buffer, 500);
        printf("%s\n", buffer);
        */
        //SnakeTimeCounter += Input->SecondsElapsed;
        
        
        NewTransitionSnake(&Player, Input->SecondsElapsed);
        if (Player.DistanceTravelled >= 1 || Player.Speed == 0)
        {
            Player.DistanceTravelled = 0;
            NewMoveSnake(&Player);
            CheckGetApple(&Player, &App);
        }
        
        CheckBounds(&Player, GRIDWIDTH, GRIDHEIGHT);
        
        
        
#if !defined(RAYLIB_H)
        ClearScreen();
        RenderBackgroundGrid(centeredX, centeredY, GRIDWIDTH, GRIDHEIGHT, GRIDSIZE, &test);
        RenderApple(&App, centeredX, centeredY, GRIDWIDTH, GRIDHEIGHT, GRIDSIZE);
        NewRenderSnake(&Player, centeredX, centeredY, GRIDWIDTH, GRIDHEIGHT, GRIDSIZE);
        PrintOnScreen(&Faune50, IntToString(App.Score), 5, 5, 0xFF000000);
#else
        BeginDrawing();
        ClearBackground(WHITE);
        RenderBackgroundGrid(centeredX, centeredY, GRIDWIDTH, GRIDHEIGHT, GRIDSIZE, &test, &testTexture);
        RenderApple(&App, centeredX, centeredY, GRIDWIDTH, GRIDHEIGHT, GRIDSIZE);
        NewRenderSnake(&Player, centeredX, centeredY, GRIDWIDTH, GRIDHEIGHT, GRIDSIZE);
        PrintOnScreen(&Faune50, IntToString(App.Score), 5, 5, 0xFF000000);
        DrawFPS(300, 10);
        EndDrawing();
#endif
    }
}
