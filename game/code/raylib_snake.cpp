/*******************************************************************************************
*
*   raylib [core] example - Basic window
*
*   Welcome to raylib!
*
*   To test examples, just press F6 and execute raylib_compile_execute script
*   Note that compiled executable is placed in the same folder as .c file
*
*   You can find all basic examples on C:\raylib\raylib\examples folder or
*   raylib official webpage: www.raylib.com
*
*   Enjoy using raylib. :)
*
*   This example has been created using raylib 1.0 (www.raylib.com)
*   raylib is licensed under an unmodified zlib/libpng license (View raylib.h for details)
*
*   Copyright (c) 2013-2016 Ramon Santamaria (@raysan5)
*
********************************************************************************************/

#include "raylib/raylib.h"
#include <cstdint>
#include "snake.h"

internal Texture2D
LoadImageResize(char* FileName, int Width, int Height)
{
    Image NewImage;
    NewImage = LoadImage("grass.png");
    ImageResize(&NewImage, GRIDSIZE, GRIDSIZE);
    Texture2D NewTexture = LoadTextureFromImage(NewImage);
    UnloadImage(NewImage);
    return NewTexture;
}

internal v2
GetStringDimensions(Font *FontType, char* Txt)
{
    v2 Dim = {};
    Vector2 Dims = MeasureTextEx(*FontType, Txt, (float)FontType->baseSize, 0);
    Dim.x = Dims.x;
    Dim.y = Dims.y;
    
    return Dim;
}

internal uint32
MoveAlpha(uint32 Color)
{
    uint32 Alpha = Color;
    Alpha &= 0xFF000000;
    Alpha = Alpha / 16777216;
    uint32 RGB = Color;
    RGB &= 0x00FFFFFF;
    RGB = RGB * 256;
    uint32 RGBA = RGB + Alpha;
    
    return RGBA;
}

internal void
PrintOnScreen(Font *FontType, char *Txt, int X, int Y, uint32 TextColor)
{
    Vector2 Pos = {(float)X, (float)Y};
    Color Col = GetColor(MoveAlpha(TextColor));
    DrawTextEx(*FontType, Txt, Pos, (float)FontType->baseSize, 0, Col);
}

internal void
RenderRect(Rect *S, int fill, uint32 color)
{
    Color Col = GetColor(MoveAlpha(color));
    if (fill == FILL)
        DrawRectangle(S->x, S->y, S->width, S->height, Col);
    else if (fill == NOFILL)
        DrawRectangleLines(S->x, S->y, S->width, S->height, Col);
}

internal void
RenderRectTexture(Rect *R, Texture2D* T)
{
    Color Col = GetColor(255);
    DrawTexture(*T, R->x, R->y, WHITE);
}

internal void
RenderBackgroundGrid(int GridX, int GridY, int GridWidth, int GridHeight,
                     int GridSize, Image *image, Texture2D *texture)
{
    for (int i = 0;
         i < GridWidth;
         i++)
    {
        for (int j = 0;
             j < GridHeight;
             j++)
        {
            Rect newRect = {GridX + (i * GridSize), GridY + (j * GridSize), GridSize, GridSize};
            RenderRect(&newRect, NOFILL, 0xFF000000);
            RenderRectTexture(&newRect, texture);
        }
    }
}

/*
internal void
RenderRectImage(game_offscreen_buffer *Buffer, Rect *R, Image* I)
{
    Texture2D Img = LoadTextureFromImage(*I);
    Color Col = GetColor(0);
    DrawTexture(Img, R->x, R->y, Col);
}
*/

internal void
raylibProcessKeyboardMessage(game_button_state *NewState, bool32 IsDown)
{
    //Assert(NewState->EndedDown != IsDown);
    if (IsDown == (bool32)true && NewState->EndedDown == (bool32)false)
    {
        NewState->NewEndedDown = true;
    }
    else
    {
        NewState->NewEndedDown = false;
    }
    
    NewState->EndedDown = IsDown;
    ++NewState->HalfTransitionCount;
}

internal void
raylibProcessInput(game_controller_input *KeyboardController)
{
    raylibProcessKeyboardMessage(&KeyboardController->MoveUp, IsKeyDown('W'));
    raylibProcessKeyboardMessage(&KeyboardController->MoveLeft, IsKeyDown('A'));
    raylibProcessKeyboardMessage(&KeyboardController->MoveDown, IsKeyDown('S'));
    raylibProcessKeyboardMessage(&KeyboardController->MoveRight, IsKeyDown('D'));
    raylibProcessKeyboardMessage(&KeyboardController->LeftShoulder, IsKeyDown('Q'));
    raylibProcessKeyboardMessage(&KeyboardController->RightShoulder, IsKeyDown('E'));
    raylibProcessKeyboardMessage(&KeyboardController->MoveUp, IsKeyDown(KEY_UP));
    raylibProcessKeyboardMessage(&KeyboardController->MoveLeft, IsKeyDown(KEY_LEFT));
    raylibProcessKeyboardMessage(&KeyboardController->MoveDown, IsKeyDown(KEY_DOWN));
    raylibProcessKeyboardMessage(&KeyboardController->MoveRight, IsKeyDown(KEY_RIGHT));
    raylibProcessKeyboardMessage(&KeyboardController->Start, IsKeyDown(KEY_ESCAPE));
    raylibProcessKeyboardMessage(&KeyboardController->Back, IsKeyDown(KEY_SPACE));
    // Numbers
    raylibProcessKeyboardMessage(&KeyboardController->Zero, IsKeyDown('0'));
    raylibProcessKeyboardMessage(&KeyboardController->One, IsKeyDown('1'));
    raylibProcessKeyboardMessage(&KeyboardController->Two, IsKeyDown('2'));
    raylibProcessKeyboardMessage(&KeyboardController->Three, IsKeyDown('3'));
    raylibProcessKeyboardMessage(&KeyboardController->Four, IsKeyDown('4'));
    raylibProcessKeyboardMessage(&KeyboardController->Five, IsKeyDown('5'));
    raylibProcessKeyboardMessage(&KeyboardController->Six, IsKeyDown('6'));
    raylibProcessKeyboardMessage(&KeyboardController->Seven, IsKeyDown('7'));
    raylibProcessKeyboardMessage(&KeyboardController->Eight, IsKeyDown('8'));
    raylibProcessKeyboardMessage(&KeyboardController->Nine, IsKeyDown('9'));
    raylibProcessKeyboardMessage(&KeyboardController->Period, IsKeyDown(KEY_PERIOD));
    raylibProcessKeyboardMessage(&KeyboardController->Back, IsKeyPressed(KEY_BACKSPACE));
}

global_variable Texture2D testTexture;
#include "snake.cpp"

int main(void)
{
    const int screenWidth = 1000;
    const int screenHeight = 1000;
    
    InitWindow(screenWidth, screenHeight, "Snake");
    
    SetTargetFPS(144);               // Set our game to run at 60 frames-per-second
    
    game_memory MEMORY = {};
    
    game_input INPUT = {};
    //game_input Input[2] = {};
    //game_input *NewInput = &Input[0];
    //game_input *OldInput = &Input[1];
    
    game_offscreen_buffer BUFFER = {};
    game_sound_output_buffer SOUNDBUFFER = {};
    
    
    MEMORY.IsInitialized = false;
    
    MEMORY.PermanentStorageSize = Megabytes(64);
    MEMORY.TransientStorageSize = Gigabytes(1);
    
    MEMORY.PermanentStorage = MemAlloc((int)MEMORY.PermanentStorageSize);
    MEMORY.TransientStorage = MemAlloc((int)MEMORY.TransientStorageSize);
    
    /*
    NewInput->Controllers[0].IsAnalog = 0;
    NewInput->SecondsElapsed = 1;
    OldInput->Controllers[0].IsAnalog = 0;
    OldInput->SecondsElapsed = 1;
    */
    
    INPUT.Controllers[0].IsAnalog = 0;
    INPUT.SecondsElapsed = 1;
    
    BUFFER.BytesPerPixel = 4;
    BUFFER.Width = screenWidth;
    BUFFER.Height = screenHeight;
    BUFFER.Pitch = BUFFER.Width * BUFFER.BytesPerPixel;
    BUFFER.Memory = MemAlloc(BUFFER.Width * BUFFER.Height * BUFFER.BytesPerPixel); 
    
    bool GlobalRunning = true;
    
    // Main game loop
    while (!WindowShouldClose() && GlobalRunning == true) // Detect window close button or ESC key
    {
        /*
        game_controller_input *OldKeyboardController = GetController(OldInput, 0);
        game_controller_input *NewKeyboardController = GetController(NewInput, 0);
        *NewKeyboardController = {};
        NewKeyboardController->IsConnected = true;
        for(int ButtonIndex = 0;
            ButtonIndex < ArrayCount(NewKeyboardController->Buttons);
            ++ButtonIndex)
        {
            NewKeyboardController->Buttons[ButtonIndex].EndedDown =
                OldKeyboardController->Buttons[ButtonIndex].EndedDown;
        }
        
        raylibProcessInput(NewKeyboardController, OldKeyboardController);
        */
        game_controller_input *KeyboardController = GetController(&INPUT, 0);
        raylibProcessInput(KeyboardController);
        
        INPUT.MouseX = (int32)GetMouseX();
        INPUT.MouseY = (int32)GetMouseY();
        
        if(IsMouseButtonDown(0))
        {
            INPUT.MouseButtons[0].EndedDown = 1;
            ++INPUT.MouseButtons[0].HalfTransitionCount;
        }
        else
        {
            INPUT.MouseButtons[0].EndedDown = 0;
            ++INPUT.MouseButtons[0].HalfTransitionCount;
        }
        
        GameUpdateAndRender(&MEMORY, &INPUT, &BUFFER, &SOUNDBUFFER);
        INPUT.SecondsElapsed = GetFrameTime();
        
        
        if(INPUT.quit == 1)
        {
            GlobalRunning = false;
        }
        
        /*
        game_input *Temp = NewInput;
        NewInput = OldInput;
        OldInput = Temp;
*/
    }
    
    CloseWindow(); // Close window and OpenGL context
    return 0;
}