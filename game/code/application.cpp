#include "qlib/qlib.cpp"

#include "snake.h"

Image test;
Texture TexTest;
Font Faune50 = {};
Font Faune100 = {};

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
        
        GameState->Menu = 1;
        
#if SAVE_IMAGES
        test = LoadImage("grass2.png");
        test = ResizeImage(test, GRIDSIZE, GRIDSIZE);
        SaveImage(&test, "grass2.h");
        Faune50 = LoadEntireFont("Faune-TextRegular.otf", 50);
        Faune100 = LoadEntireFont("Faune-TextRegular.otf", 100);
#else
        LoadImageFromgrass2_h(&test);
#endif
        TexTest.Init(&test);
    }
    
    
    float FPS = 0;
    if (p->Input.dt != 0)
        FPS = 1 / p->Input.dt;
    PrintqDebug(S() + (int)FPS + "\n");
    
    platform_controller_input *Controller = &p->Input.Controllers[0];
    real32 Speed = 1000.0f *p->Input.dt;
    if(Controller->MoveLeft.EndedDown)
    {
        C.Position.x -= Speed;
    }
    
    if(Controller->MoveRight.EndedDown)
    {
        C.F += Speed;
    }
    
    if(Controller->MoveDown.EndedDown)
    {
        C.FOV -=  Speed;
    }
    
    if(Controller->MoveUp.EndedDown)
    {
        C.FOV +=  Speed;
    }
    
    int HalfGridX = (GRIDWIDTH * GRIDSIZE) / 2;
    int HalfGridY = (GRIDHEIGHT * GRIDSIZE) / 2;
    
    C.Dimension = p->Dimension;
    
    if (GameState->Menu == 1)
    {
        if (MainMenu.Initialized == 0)
        {
#include "main_menu.cpp"
            MainMenu.Initialized = 1;
        }
        
        BeginMode2D(C);
        UpdateGUI(&MainMenu, p->Dimension.Width, p->Dimension.Height);
        ClearScreen();
        RenderGUI(&MainMenu);
        EndMode2D();
    }
    else
    {
        BeginMode2D(C);
        RenderBackgroundGrid(-HalfGridX, -HalfGridY, GRIDWIDTH, GRIDHEIGHT, GRIDSIZE);
        EndMode2D();
    }
}
