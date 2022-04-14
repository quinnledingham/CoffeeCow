#include "qlib/qlib.cpp"

#include "snake.h"

internal void
RenderBackgroundGrid(int GridX, int GridY, int GridWidth, int GridHeight, int GridSize)
{
    uint32 Color = 0xFF0000FF;
    for (int i = 0; i < GridWidth; i++)
    {
        for (int j = 0; j < GridHeight; j++)
        {
            Color += 50;
            DrawRect(GridX + (i * GridSize), GridY + (j * GridSize), GridSize, GridSize, Color);
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

Image test;
Texture TexTest;

void UpdateRender(platform* p)
{
    game_state *GameState = (game_state*)p->Memory.PermanentStorageSize;
    
    if (!p->Initialized)
    {
        qalloc(sizeof(game_state));
        
        C.FOV = 180.01f;
        C.F = 0.01f;
        p->Initialized = true;
        
#if SAVE_IMAGES
        test = LoadImage("grass2.png");
        TexTest.Init(&test);
#else
        
#endif
        
    }
    
    
    float FPS = 0;
    if (p->Input.dt != 0)
        FPS = 1 / p->Input.dt;
    PrintqDebug(S() + (int)FPS + "\n");
    
    platform_controller_input *Controller = &p->Input.Controllers[0];
    real32 Speed = 1000.0f *p->Input.dt;
    if(Controller->MoveLeft.EndedDown)
    {
        C.F -= Speed;
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
    
    BeginMode2D(C);
    RenderBackgroundGrid(-HalfGridX, -HalfGridY, GRIDWIDTH, GRIDHEIGHT, GRIDSIZE);
    
    
    
    EndMode2D();
}
