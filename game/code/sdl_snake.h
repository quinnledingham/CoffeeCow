#ifndef SDL_SNAKE_H
#define SDL_SNAKE_H

struct sdl_offscreen_buffer
{
    // NOTE(casey): Pixels are always 32-bits wide, Memory Order BB GG RR XX
    SDL_Texture *Texture;
    void *Memory;
    int Width;
    int Height;
    int Pitch;
    int BytesPerPixel;
};

struct sdl_window_dimension
{
    int Width;
    int Height;
};

struct sdl_audio_ring_buffer
{
    int Size;
    int WriteCursor;
    int PlayCursor;
    void *Data;
};

struct sdl_sound_output
{
    int SamplesPerSecond;
    uint32 RunningSampleIndex;
    int BytesPerSample;
    int SecondaryBufferSize;
    int LatencySampleCount;
};

struct sdl_debug_time_marker
{
    int PlayCursor;
    int WriteCursor;
};

#endif //SDL_SNAKE_H
