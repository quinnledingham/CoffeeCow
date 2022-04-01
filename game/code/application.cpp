#include "qlib/qlib.cpp"

#include "snakev2.h"

Camera C =
{
    v3(0, 0, -1),
    v3(0, 0, 0),
    v3(0, 1, 0),
    0,
    0,
};

void Update(platform* p)
{
    game_state *GameState = (game_state*)p->Memory.PermanentStorageSize;
    
    if (!p->Initialized)
    {
        qalloc(sizeof(game_state));
        
        //OpenGL.BasicProg = OpenGLCreateProgram(HeaderCode, VertexCode, FragmentCode);
        C.FOV = 180.01f;
        C.F = 0.01f;
        p->Initialized = true;
    }
    
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
    
}

void Render(float inAspectRatio)
{
    C.inAspectRatio = inAspectRatio;
    BeginMode3D(C);
    DrawRect(0, 0, 100, 100, NOFILL, 0xFF00FF00);
    EndMode3D();
}