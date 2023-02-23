void GLAPIENTRY
opengl_debug_message_callback(GLenum source, GLenum type, GLuint id, GLenum severity,
                              GLsizei length, const GLchar* message, const void* userParam )
{
    SDL_Log("GL CALLBACK:");
    SDL_Log("message: %s\n", message);
    switch (type)
    {
        case GL_DEBUG_TYPE_ERROR: SDL_Log("type: ERROR"); break;
        case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: SDL_Log("type: DEPRECATED_BEHAVIOR"); break;
        case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR: SDL_Log("type: UNDEFINED_BEHAVIOR"); break;
        case GL_DEBUG_TYPE_PORTABILITY: SDL_Log("type: PORTABILITY"); break;
        case GL_DEBUG_TYPE_PERFORMANCE: SDL_Log("type: PERFORMANCE"); break;
        case GL_DEBUG_TYPE_OTHER: SDL_Log("type: OTHER"); break;
    }
    SDL_Log("id: %d", id);
    switch(severity)
    {
        case GL_DEBUG_SEVERITY_LOW: SDL_Log("severity: LOW\n"); break;
        case GL_DEBUG_SEVERITY_MEDIUM: SDL_Log("severity: MEDIUM\n"); break;
        case GL_DEBUG_SEVERITY_HIGH: SDL_Log("severity: HIGH\n"); break;
    }
}

function void
opengl_debug(u32 type, u32 id)
{
    GLint length;
    glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length);
    if (length > 0)
    {
        GLchar info_log[512];
        GLint size;
        
        switch(type)
        {
            case GL_SHADER: glGetShaderInfoLog(id, 512, &size, info_log); break;
            case GL_PROGRAM: glGetProgramInfoLog(id, 512, &size, info_log); break;
        }
        
        log(info_log);
    }
}
