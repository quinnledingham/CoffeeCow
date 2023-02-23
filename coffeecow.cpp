#include <gl.h>
#include <gl.c>
#include <SDL.h>

#include "qlib/types.h"
#include "qlib/log.h"
#include "qlib/math.h"
#include "qlib/input.h"
#include "qlib/sdl_keycodes.h"

struct Controller
{
    union
    {
        struct
        {
            Button left;
        };
        Button buttons[1];
    };
};

#include "qlib/application.h"
#include "qlib/sdl_application.cpp"
#include "qlib/opengl_debug.cpp"

function void
do_one_frame(Application *app)
{
    if (on_down(app->controller.left))
        printf("yo\n");
    
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
}

function void
init_controller(Controller *controller)
{
    set(&controller->left, KEY_A);
}

function void
init_app(Application *app)
{
    init_controller(&app->controller);
    
    // Default OpenGL Settings
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);
    glEnable(GL_CULL_FACE);
    glPointSize(5.0f);
    glPatchParameteri(GL_PATCH_VERTICES, 4);
    
    glEnable(GL_DEBUG_OUTPUT);
    glDebugMessageCallback(opengl_debug_message_callback, 0);
    
    glViewport(0, 0, app->window_dim.Width, app->window_dim.Height);
}