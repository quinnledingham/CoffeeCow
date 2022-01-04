#include <stdint.h>
#define internal static
#define local_persist static
#define global_variable static

#define Pi32 3.14159265359f

typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;

typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;

typedef int32 bool32;

typedef float real32;
typedef double real64;

// TODO(casey): Implement sine ourselves 
#include <math.h>

#include "snake.h"
#include "snake.cpp"

#include <SDL.h>
//#include <stdio.h>
//#include <unistd.h>
//#include <fcntl.h>
//#include <sys/types.h>
//#include <sys/stat.h>
#include <sys/mman.h>
#include <x86intrin.h>

#include "sdl_snake.h"

// NOTE: MAP_ANONYMOUS is not defined on Mac OS X and some other UNIX systems.
// On the vast majority of those systems, one can use MAP_ANON instead.
// Huge thanks to Adam Rosenfield for investigating this, and suggesting this
// workaround:
#ifndef MAP_ANONYMOUS
#define MAP_ANONYMOUS MAP_ANON
#endif

global_variable sdl_offscreen_buffer GlobalBackbuffer;

#define MAX_CONTROLLERS 4
SDL_GameController *ControllerHandles[MAX_CONTROLLERS];
SDL_Haptic *RumbleHandles[MAX_CONTROLLERS];

sdl_audio_ring_buffer AudioRingBuffer;

internal real32
SDLGetSecondsElapsed(uint64 OldCounter, uint64 CurrentCounter)
{
    return ((real32)(CurrentCounter - OldCounter) / (real32)(SDL_GetPerformanceFrequency()));
}

internal void
SDLUpdateWindow(SDL_Window *Window, SDL_Renderer *Renderer, sdl_offscreen_buffer *Buffer)
{
    SDL_UpdateTexture(Buffer->Texture,
                      0,
                      Buffer->Memory,
                      Buffer->Pitch);
    
    SDL_RenderCopy(Renderer,
                   Buffer->Texture,
                   0,
                   0);
    
    SDL_RenderPresent(Renderer);
}

internal void
SDLProcessKeyPress(game_button_state *NewState, bool32 IsDown)
{
    Assert(NewState->EndedDown != IsDown);
    NewState->EndedDown = IsDown;
    ++NewState->HalfTransitionCount;
}

bool HandleEvent(SDL_Event *Event, game_controller_input *NewKeyboardController)
{
    bool ShouldQuit = false;
    switch(Event->type)
    {
        case SDL_QUIT:
        {
            printf("SDL_QUIT\n");
            ShouldQuit = true;
        } break;
        
        case SDL_KEYDOWN:
        case SDL_KEYUP:
        {
            SDL_Keycode KeyCode = Event->key.keysym.sym;
            bool IsDown = (Event->key.state == SDL_PRESSED);
            bool WasDown = false;
            if (Event->key.state == SDL_RELEASED)
            {
                WasDown = true;
            }
            else if (Event->key.repeat != 0)
            {
                WasDown = true;
            }
            
            // NOTE: In the windows version, we used "if (IsDown != WasDown)"
            // to detect key repeats. SDL has the 'repeat' value, though,
            // which we'll use.
            if (Event->key.repeat == 0)
            {
                if(KeyCode == SDLK_w)
                {
                    SDLProcessKeyPress(&NewKeyboardController->MoveUp, IsDown);
                }
                else if(KeyCode == SDLK_a)
                {
                    SDLProcessKeyPress(&NewKeyboardController->MoveLeft, IsDown);
                }
                else if(KeyCode == SDLK_s)
                {
                    SDLProcessKeyPress(&NewKeyboardController->MoveDown, IsDown);
                }
                else if(KeyCode == SDLK_d)
                {
                    SDLProcessKeyPress(&NewKeyboardController->MoveRight, IsDown);
                }
                else if(KeyCode == SDLK_q)
                {
                    SDLProcessKeyPress(&NewKeyboardController->LeftShoulder, IsDown);
                }
                else if(KeyCode == SDLK_e)
                {
                    SDLProcessKeyPress(&NewKeyboardController->RightShoulder, IsDown);
                }
                else if(KeyCode == SDLK_UP)
                {
                    SDLProcessKeyPress(&NewKeyboardController->ActionUp, IsDown);
                }
                else if(KeyCode == SDLK_LEFT)
                {
                    SDLProcessKeyPress(&NewKeyboardController->ActionLeft, IsDown);
                }
                else if(KeyCode == SDLK_DOWN)
                {
                    SDLProcessKeyPress(&NewKeyboardController->ActionDown, IsDown);
                }
                else if(KeyCode == SDLK_RIGHT)
                {
                    SDLProcessKeyPress(&NewKeyboardController->ActionRight, IsDown);
                }
                else if(KeyCode == SDLK_ESCAPE)
                {
                    printf("ESCAPE: ");
                    if(IsDown)
                    {
                        printf("IsDown ");
                    }
                    if(WasDown)
                    {
                        printf("WasDown");
                    }
                    printf("\n");
                }
                else if(KeyCode == SDLK_SPACE)
                {
                }
            }
            
            bool AltKeyWasDown = (Event->key.keysym.mod & KMOD_ALT);
            if (KeyCode == SDLK_F4 && AltKeyWasDown)
            {
                ShouldQuit = true;
            }
            
        } break;
        
        case SDL_WINDOWEVENT:
        {
            switch(Event->window.event)
            {
                case SDL_WINDOWEVENT_SIZE_CHANGED:
                {
                    SDL_Window *Window = SDL_GetWindowFromID(Event->window.windowID);
                    SDL_Renderer *Renderer = SDL_GetRenderer(Window);
                    printf("SDL_WINDOWEVENT_SIZE_CHANGED (%d, %d)\n", Event->window.data1, Event->window.data2);
                } break;
                
                case SDL_WINDOWEVENT_FOCUS_GAINED:
                {
                    printf("SDL_WINDOWEVENT_FOCUS_GAINED\n");
                } break;
                
                case SDL_WINDOWEVENT_EXPOSED:
                {
                    SDL_Window *Window = SDL_GetWindowFromID(Event->window.windowID);
                    SDL_Renderer *Renderer = SDL_GetRenderer(Window);
                    SDLUpdateWindow(Window, Renderer, &GlobalBackbuffer);
                } break;
            }
        } break;
    }
    
    return(ShouldQuit);
}

internal void
SDLFillSoundBuffer(sdl_sound_output *SoundOutput, int ByteToLock, int BytesToWrite, game_sound_output_buffer *SoundBuffer)
{
    printf("yo");
    void *Region1 = (uint8*)AudioRingBuffer.Data + ByteToLock;
    int Region1Size = BytesToWrite;
    if (Region1Size + ByteToLock > SoundOutput->SecondaryBufferSize)
    {
        Region1Size = SoundOutput->SecondaryBufferSize - ByteToLock;
    }
    void *Region2 = AudioRingBuffer.Data;
    int Region2Size = BytesToWrite - Region1Size;
    int Region1SampleCount = Region1Size/SoundOutput->BytesPerSample;
    int16 *SampleOut = (int16 *)Region1;
    int16 *Samples = SoundBuffer->Samples;
    for(int SampleIndex = 0;
        SampleIndex < Region1SampleCount;
        ++SampleIndex)
    {
        *SampleOut++ = *Samples++;
        *SampleOut++ = *Samples++;
        ++SoundOutput->RunningSampleIndex;
    }
    
    int Region2SampleCount = Region2Size/SoundOutput->BytesPerSample;
    SampleOut = (int16 *)Region2;
    for(int SampleIndex = 0;
        SampleIndex < Region2SampleCount;
        ++SampleIndex)
    {
        *SampleOut++ = *Samples++;
        *SampleOut++ = *Samples++;
        ++SoundOutput->RunningSampleIndex;
    }
    //printf("RunningSampleIndex: %d %d\n", AudioRingBuffer.Size, AudioRingBuffer.PlayCursor);
    
}

global_variable uint8 *audio_pos; // global pointer to the audio buffer to be played
global_variable uint32 audio_len; // remaining length of the sample we have to play

internal void
SDLAudioCallback(void *UserData, uint8 *AudioData, int Length)
{
    /*
    sdl_audio_ring_buffer *RingBuffer = (sdl_audio_ring_buffer *)UserData;
    
    int Region1Size = Length;
    int Region2Size = 0;
    if ((RingBuffer->PlayCursor + Length) > RingBuffer->Size)
    {
        Region1Size = RingBuffer->Size - RingBuffer->PlayCursor;
        Region2Size = Length - Region1Size;
    }
    memcpy(AudioData, (uint8*)(RingBuffer->Data) + RingBuffer->PlayCursor, Region1Size);
    memcpy(&AudioData[Region1Size], RingBuffer->Data, Region2Size);
    RingBuffer->PlayCursor = (RingBuffer->PlayCursor + Length) % RingBuffer->Size;
    RingBuffer->WriteCursor = (RingBuffer->PlayCursor + Length) % RingBuffer->Size;
    */
    //printf("AudioRingBuffer: %d\n", AudioRingBuffer.PlayCursor);
    printf("yo2 %d\n", audio_len);
    if (audio_len ==0)
		return;
	
    Length = ( Length > audio_len ? audio_len : Length );
	//SDL_memcpy (AudioData, audio_pos, Length); 					// simply copy from one buffer into the other
	SDL_memset(AudioData, 0, Length);
    SDL_MixAudio(AudioData, audio_pos, Length, SDL_MIX_MAXVOLUME);// mix from one buffer into another
    audio_pos += Length;
	audio_len -= Length;
}

internal void
SDLInitAudio(int32 SamplesPerSecond, int32 BufferSize)
{
    /*
    SDL_AudioSpec AudioSettings = {};
    
    AudioSettings.freq = SamplesPerSecond;
    AudioSettings.format = AUDIO_S16LSB;
    AudioSettings.channels = 2;
    AudioSettings.samples = 512;
    AudioSettings.callback = &SDLAudioCallback;
    AudioSettings.userdata = &AudioRingBuffer;
    */
    AudioRingBuffer.Size = BufferSize;
    AudioRingBuffer.Data = calloc(BufferSize, 1);
    AudioRingBuffer.PlayCursor = AudioRingBuffer.WriteCursor = 0;
    /*
    SDL_OpenAudio(&AudioSettings, 0);
    //printf("BufferSize: %d\n", BufferSize);
    printf("Initialised an Audio device at frequency %d Hz, %d Channels, buffer size %d\n",
           AudioSettings.freq, AudioSettings.channels, AudioSettings.size);
    
    if (AudioSettings.format != AUDIO_S16LSB)
    {
        printf("Oops! We didn't get AUDIO_S16LSB as our sample format!\n");
        SDL_CloseAudio();
    }
*/
    
    SDL_AudioSpec want, have;
    SDL_AudioDeviceID dev;
    
    SDL_zero(want);
    want.freq = 48000;
    want.format = AUDIO_F32;
    want.channels = 2;
    want.samples = 4096;
    want.callback = &SDLAudioCallback;
    
    dev = SDL_OpenAudioDevice(NULL, 0, &want, &have, SDL_AUDIO_ALLOW_FORMAT_CHANGE);
    
    if (dev == 0) {
        SDL_Log("Failed to open audio: %s", SDL_GetError());
    } else {
        if (have.format != want.format) { /* we let this one thing change. */
            SDL_Log("We didn't get Float32 audio format.");
        }
        printf("Opened Audio Device\n");
        SDL_PauseAudioDevice(dev, 0); /* start audio playing. */
    }
}

sdl_window_dimension
SDLGetWindowDimension(SDL_Window *Window)
{
    sdl_window_dimension Result;
    SDL_GetWindowSize(Window, &Result.Width, &Result.Height);
    return(Result);
}

internal int
SDLGetWindowRefreshRate(SDL_Window *Window)
{
    SDL_DisplayMode Mode;
    int DisplayIndex = SDL_GetWindowDisplayIndex(Window);
    // If we can't find the refresh rate, we'lll return this:
    int DefaultRefreshRate = 60;
    if (SDL_GetDesktopDisplayMode(DisplayIndex, &Mode) != 0)
    {
        return DefaultRefreshRate;
    }
    if (Mode.refresh_rate == 0)
    {
        return DefaultRefreshRate;
    }
    return Mode.refresh_rate;
}

internal void
SDLOpenGameControllers()
{
    for (int ControllerIndex = 0;
         ControllerIndex < MAX_CONTROLLERS;
         ++ControllerIndex)
    {
        if (ControllerHandles[ControllerIndex])
        {
            if (RumbleHandles[ControllerIndex])
            {
                SDL_HapticClose(RumbleHandles[ControllerIndex]);
            }
            
            SDL_GameControllerClose(ControllerHandles[ControllerIndex]);
        }
    }
}

internal void
SDLResizeTexture(sdl_offscreen_buffer *Buffer, SDL_Renderer *Renderer, int Width, int Height)
{
    int BytesPerPixel = 4;
    if (Buffer->Memory)
    {
        munmap(Buffer->Memory, Buffer->Width * Buffer->Height * BytesPerPixel);
    }
    if (Buffer->Texture)
    {
        SDL_DestroyTexture(Buffer->Texture);
    }
    
    Buffer->Texture = SDL_CreateTexture(Renderer,
                                        SDL_PIXELFORMAT_ARGB8888,
                                        SDL_TEXTUREACCESS_STREAMING,
                                        Width,
                                        Height);
    Buffer->Width = Width;
    Buffer->Height = Height;
    Buffer->Pitch = Width * BytesPerPixel;
    Buffer->BytesPerPixel = BytesPerPixel;
    Buffer->Memory = mmap(0,
                          Width * Height * BytesPerPixel,
                          PROT_READ | PROT_WRITE,
                          MAP_PRIVATE | MAP_ANONYMOUS,
                          -1,
                          0);
}

internal void
SDLProcessGameControllerButton(game_button_state *OldState,
                               game_button_state *NewState,
                               bool Value)
{
    NewState->EndedDown = Value;
    NewState->HalfTransitionCount += ((NewState->EndedDown == OldState->EndedDown)?0:1);
}

internal real32
SDLProcessGameControllerAxisValue(int16 Value, int16 DeadZoneThreshold)
{
    real32 Result = 0;
    
    if(Value < -DeadZoneThreshold)
    {
        Result = (real32)((Value + DeadZoneThreshold) / (32768.0f - DeadZoneThreshold));
    }
    else if(Value > DeadZoneThreshold)
    {
        Result = (real32)((Value - DeadZoneThreshold) / (32767.0f - DeadZoneThreshold));
    }
    
    return(Result);
}

internal void
SDLCloseGameControllers()
{
    for(int ControllerIndex = 0; ControllerIndex < MAX_CONTROLLERS; ++ControllerIndex)
    {
        if (ControllerHandles[ControllerIndex])
        {
            if (RumbleHandles[ControllerIndex])
                SDL_HapticClose(RumbleHandles[ControllerIndex]);
            SDL_GameControllerClose(ControllerHandles[ControllerIndex]);
        }
    }
}

int main (int argc, char *argv[])
{
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER | SDL_INIT_HAPTIC | SDL_INIT_AUDIO);
    uint64 PerfCountFrequency = SDL_GetPerformanceFrequency();
    
    // Initialise our Game Controllers:
    SDLOpenGameControllers();
    
    // Create our window.
    SDL_Window *Window = SDL_CreateWindow("Snake",
                                          SDL_WINDOWPOS_UNDEFINED,
                                          SDL_WINDOWPOS_UNDEFINED,
                                          1280,
                                          720,
                                          SDL_WINDOW_RESIZABLE);
    
    if (Window)
    {
        // Create a "Renderer" for our window
        SDL_Renderer *Renderer = SDL_CreateRenderer(Window,
                                                    -1,
                                                    SDL_RENDERER_PRESENTVSYNC);
        //SDL_RenderSetLogicalSize(Renderer, 1280, 720);
        printf("Refresh rate is %d Hz\n", SDLGetWindowRefreshRate(Window));
        int GameUpdateHz = 30;
        real32 TargetSecondsPerFrame = 1.0f / (real32)GameUpdateHz;
        if (Renderer)
        {
            bool Running = true;
            sdl_window_dimension Dimension = SDLGetWindowDimension(Window);
            SDLResizeTexture(&GlobalBackbuffer, Renderer, Dimension.Width, Dimension.Height);
            
            game_input Input[2] = {};
            game_input *NewInput = &Input[0];
            game_input *OldInput = &Input[1];
            
            /*
            // Setting up sdl sound output
            sdl_sound_output SoundOutput = {};
            SoundOutput.SamplesPerSecond = 48000;
            SoundOutput.RunningSampleIndex = 0;
            SoundOutput.BytesPerSample = sizeof(int16) * 2;
            SoundOutput.SecondaryBufferSize = SoundOutput.SamplesPerSecond * SoundOutput.BytesPerSample;
            SoundOutput.LatencySampleCount = SoundOutput.SamplesPerSecond / 15;
            
            // Open our audio device:
            SDLInitAudio(SoundOutput.SamplesPerSecond, SoundOutput.SecondaryBufferSize);
            
            // NOTE: calloc() allocates memory and clears it to zero. It accepts the number of things being allocated and their size.
            int16 *Samples = (int16 *)calloc(SoundOutput.SamplesPerSecond, SoundOutput.BytesPerSample);
            SDL_PauseAudio(0);
            */
            
            /*
            local_persist uint32 wav_length; // length of our sample
            local_persist uint8 *wav_buffer; // buffer containing our audio file
            local_persist SDL_AudioSpec wav_spec; // the specs of our piece of music;
            
            // Load the WAV
            // the specs, length and buffer of our wav are filled
            if (SDL_LoadWAV("../snake/code/ahem_x.wav", &wav_spec, &wav_buffer, &wav_length) == NULL)
            {
                printf("main: SDL_LOADWAV Could not find wav file.\n");
                return 1;
            }
            
            // set the callback function
            wav_spec.callback = &SDLAudioCallback;
            wav_spec.userdata = wav_buffer;
            // set our global static variables
            audio_pos = wav_buffer; // copy sound buffer
            audio_len = wav_length; // copy file length
            
             // Open the audio device
            if (SDL_OpenAudio(&wav_spec, NULL) < 0){
                fprintf(stderr, "Couldn't open audio: %s\n", SDL_GetError());
                exit(-1);
            }
            
            // Start playing
            SDL_PauseAudio(0);
            */
            
#if SNAKE_INTERNAL
            // TODO: This will fail gently on 32-bit at the moment, but we should probably fix it.
            void *BaseAddress = (void *)Terabytes(2);
#else
            void *BaseAddress = (void *)(0);
#endif
            
            game_memory GameMemory = {};
            GameMemory.PermanentStorageSize = Megabytes(64);
            GameMemory.TransientStorageSize = Gigabytes(4);
            
            uint64 TotalStorageSize = GameMemory.PermanentStorageSize + GameMemory.TransientStorageSize;
            
            GameMemory.PermanentStorage = mmap(BaseAddress, TotalStorageSize,
                                               PROT_READ | PROT_WRITE,
                                               MAP_ANON | MAP_PRIVATE,
                                               -1, 0);
            
            Assert(GameMemory.PermanentStorage != MAP_FAILED);
            
            GameMemory.TransientStorage = (uint8*)(GameMemory.PermanentStorage) + GameMemory.PermanentStorageSize;
            
            int DebugTimeMarkerIndex = 0;
            sdl_debug_time_marker DebugTimeMarkers[GameUpdateHz / 2] = {0};
            
            uint64 LastCounter = SDL_GetPerformanceCounter();
            uint64 LastCycleCount = _rdtsc();
            
            while(Running)
            {
                game_controller_input *OldKeyboardController = GetController(OldInput,0);
                game_controller_input *NewKeyboardController = GetController(NewInput,0);
                *NewKeyboardController = {};
                for(int ButtonIndex = 0;
                    ButtonIndex < ArrayCount(NewKeyboardController->Buttons);
                    ++ButtonIndex)
                {
                    NewKeyboardController->Buttons[ButtonIndex].EndedDown =
                        OldKeyboardController->Buttons[ButtonIndex].EndedDown;
                }
                
                SDL_Event Event;
                while(SDL_PollEvent(&Event))
                {
                    if (HandleEvent(&Event, NewKeyboardController))
                    {
                        Running = false;
                    }
                }
                
                // Poll our controllers for input.
                for (int ControllerIndex = 0;
                     ControllerIndex < MAX_CONTROLLERS;
                     ++ControllerIndex)
                {
                    if(ControllerHandles[ControllerIndex] != 0 && SDL_GameControllerGetAttached(ControllerHandles[ControllerIndex]))
                    {
                        game_controller_input *OldController = GetController(OldInput,ControllerIndex+1);
                        game_controller_input *NewController = GetController(NewInput,ControllerIndex+1);
                        
                        NewController->IsConnected = true;
                        
                        //TODO: Do something with the DPad, Start and Selected?
                        bool Up = SDL_GameControllerGetButton(ControllerHandles[ControllerIndex], SDL_CONTROLLER_BUTTON_DPAD_UP);
                        bool Down = SDL_GameControllerGetButton(ControllerHandles[ControllerIndex], SDL_CONTROLLER_BUTTON_DPAD_DOWN);
                        bool Left = SDL_GameControllerGetButton(ControllerHandles[ControllerIndex], SDL_CONTROLLER_BUTTON_DPAD_LEFT);
                        bool Right = SDL_GameControllerGetButton(ControllerHandles[ControllerIndex], SDL_CONTROLLER_BUTTON_DPAD_RIGHT);
                        bool Start = SDL_GameControllerGetButton(ControllerHandles[ControllerIndex], SDL_CONTROLLER_BUTTON_START);
                        bool Back = SDL_GameControllerGetButton(ControllerHandles[ControllerIndex], SDL_CONTROLLER_BUTTON_BACK);
                        
                        SDLProcessGameControllerButton(&(OldController->LeftShoulder),
                                                       &(NewController->LeftShoulder),
                                                       SDL_GameControllerGetButton(ControllerHandles[ControllerIndex], SDL_CONTROLLER_BUTTON_LEFTSHOULDER));
                        
                        SDLProcessGameControllerButton(&(OldController->RightShoulder),
                                                       &(NewController->RightShoulder),
                                                       SDL_GameControllerGetButton(ControllerHandles[ControllerIndex], SDL_CONTROLLER_BUTTON_RIGHTSHOULDER));
                        
                        SDLProcessGameControllerButton(&(OldController->ActionDown),
                                                       &(NewController->ActionDown),
                                                       SDL_GameControllerGetButton(ControllerHandles[ControllerIndex], SDL_CONTROLLER_BUTTON_A));
                        
                        SDLProcessGameControllerButton(&(OldController->ActionRight),
                                                       &(NewController->ActionRight),
                                                       SDL_GameControllerGetButton(ControllerHandles[ControllerIndex], SDL_CONTROLLER_BUTTON_B));
                        
                        SDLProcessGameControllerButton(&(OldController->ActionLeft),
                                                       &(NewController->ActionLeft),
                                                       SDL_GameControllerGetButton(ControllerHandles[ControllerIndex], SDL_CONTROLLER_BUTTON_X));
                        
                        SDLProcessGameControllerButton(&(OldController->ActionUp),
                                                       &(NewController->ActionUp),
                                                       SDL_GameControllerGetButton(ControllerHandles[ControllerIndex], SDL_CONTROLLER_BUTTON_Y));
                        
                        NewController->StickAverageX = SDLProcessGameControllerAxisValue(SDL_GameControllerGetAxis(ControllerHandles[ControllerIndex], SDL_CONTROLLER_AXIS_LEFTX), 1);
                        NewController->StickAverageY = -SDLProcessGameControllerAxisValue(SDL_GameControllerGetAxis(ControllerHandles[ControllerIndex], SDL_CONTROLLER_AXIS_LEFTY), 1);
                        if((NewController->StickAverageX != 0.0f) ||
                           (NewController->StickAverageY != 0.0f))
                        {
                            NewController->IsAnalog = true;
                        }
                        
                        if(SDL_GameControllerGetButton(ControllerHandles[ControllerIndex], SDL_CONTROLLER_BUTTON_DPAD_UP))
                        {
                            NewController->StickAverageY = 1.0f;
                            NewController->IsAnalog = false;
                        }
                        
                        if(SDL_GameControllerGetButton(ControllerHandles[ControllerIndex], SDL_CONTROLLER_BUTTON_DPAD_DOWN))
                        {
                            NewController->StickAverageY = -1.0f;
                            NewController->IsAnalog = false;
                        }
                        
                        if(SDL_GameControllerGetButton(ControllerHandles[ControllerIndex], SDL_CONTROLLER_BUTTON_DPAD_LEFT))
                        {
                            NewController->StickAverageX = -1.0f;
                            NewController->IsAnalog = false;
                        }
                        
                        if(SDL_GameControllerGetButton(ControllerHandles[ControllerIndex], SDL_CONTROLLER_BUTTON_DPAD_RIGHT))
                        {
                            NewController->StickAverageX = 1.0f;
                            NewController->IsAnalog = false;
                        }
                        
                        real32 Threshold = 0.5f;
                        SDLProcessGameControllerButton(&(OldController->MoveLeft),
                                                       &(NewController->MoveLeft),
                                                       NewController->StickAverageX < -Threshold);
                        SDLProcessGameControllerButton(&(OldController->MoveRight),
                                                       &(NewController->MoveRight),
                                                       NewController->StickAverageX > Threshold);
                        SDLProcessGameControllerButton(&(OldController->MoveUp),
                                                       &(NewController->MoveUp),
                                                       NewController->StickAverageY < -Threshold);
                        SDLProcessGameControllerButton(&(OldController->MoveDown),
                                                       &(NewController->MoveDown),
                                                       NewController->StickAverageY > Threshold);
                        
                    }
                    else
                    {
                        // TODO: This controller is not plugged in.
                    }
                }
                
                /*
                // Sound output test
                SDL_LockAudio();
                int ByteToLock = (SoundOutput.RunningSampleIndex*SoundOutput.BytesPerSample) % SoundOutput.SecondaryBufferSize;
                int TargetCursor = ((AudioRingBuffer.PlayCursor +
                                     (SoundOutput.LatencySampleCount*SoundOutput.BytesPerSample)) %
                                    SoundOutput.SecondaryBufferSize);
                int BytesToWrite;;
                if(ByteToLock > TargetCursor)
                {
                    BytesToWrite = (SoundOutput.SecondaryBufferSize - ByteToLock);
                    BytesToWrite += TargetCursor;
                }
                else
                {
                    BytesToWrite = TargetCursor - ByteToLock;
                }
                
                SDL_UnlockAudio();
                */
                game_sound_output_buffer SoundBuffer = {};
                /*
SoundBuffer.SamplesPerSecond = SoundOutput.SamplesPerSecond;
                SoundBuffer.SampleCount = BytesToWrite / SoundOutput.BytesPerSample;
                SoundBuffer.Samples = Samples;
                //printf("Samples: %d\n", BytesToWrite);
*/
                
                // wait until we're done playing
                /*
                if (audio_len < 1) {
                    SDL_CloseAudio();
                    SDL_FreeWAV(wav_buffer);
                    audio_len = 5;
                }
*/
                
                game_offscreen_buffer Buffer = {};
                Buffer.Memory = GlobalBackbuffer.Memory;
                Buffer.Width = GlobalBackbuffer.Width; 
                Buffer.Height = GlobalBackbuffer.Height;
                Buffer.Pitch = GlobalBackbuffer.Pitch; 
                
                GameUpdateAndRender(&GameMemory, NewInput, &Buffer, &SoundBuffer);
                
                game_input *Temp = NewInput;
                NewInput = OldInput;
                OldInput = Temp;
                
                //SDLFillSoundBuffer(&SoundOutput, ByteToLock, BytesToWrite, &SoundBuffer); 
                
                //printf("%d\n", SDL_GetQueuedAudioSize, )
                
                if (SDLGetSecondsElapsed(LastCounter, SDL_GetPerformanceCounter()) < TargetSecondsPerFrame)
                {
                    int32 TimeToSleep = ((TargetSecondsPerFrame - SDLGetSecondsElapsed(LastCounter, SDL_GetPerformanceCounter())) * 1000) - 1;
                    if (TimeToSleep > 0)
                    {
                        SDL_Delay(TimeToSleep);
                    }
                    Assert(SDLGetSecondsElapsed(LastCounter, SDL_GetPerformanceCounter()) < TargetSecondsPerFrame)
                        while (SDLGetSecondsElapsed(LastCounter, SDL_GetPerformanceCounter()) < TargetSecondsPerFrame)
                    {
                        // Waiting...
                    }
                }
                
                // Get this before SDLUpdateWindow() so that we don't keep missing VBlanks.
                uint64 EndCounter = SDL_GetPerformanceCounter();
                
#if HANDMADE_INTERNAL
                SDLDebugSyncDisplay(&GlobalBackbuffer, ArrayCount(DebugTimeMarkers), DebugTimeMarkers,
                                    &SoundOutput, TargetSecondsPerFrame);
#endif
                
                SDLUpdateWindow(Window, Renderer, &GlobalBackbuffer);
                
#if HANDMADE_INTERNAL
                // NOTE(casey): This is debug code
                {
                    sdl_debug_time_marker *Marker = &DebugTimeMarkers[DebugTimeMarkerIndex++];
                    if(DebugTimeMarkerIndex > ArrayCount(DebugTimeMarkers))
                    {
                        DebugTimeMarkerIndex = 0;
                    }
                    Marker->PlayCursor = AudioRingBuffer.PlayCursor;
                    Marker->WriteCursor = AudioRingBuffer.WriteCursor;
                }
#endif
                uint64 EndCycleCount = _rdtsc();
                uint64 CounterElapsed = EndCounter - LastCounter;
                uint64 CyclesElapsed = EndCycleCount - LastCycleCount;
                
                real64 MSPerFrame = (((1000.0f * (real64)CounterElapsed) / (real64)PerfCountFrequency));
                real64 FPS = (real64)PerfCountFrequency / (real64)CounterElapsed;
                real64 MCPF = ((real64)CyclesElapsed / (1000.0f * 1000.0f));
                
                //printf("%.02fms/f, %.02f/s, %.02fmc/f\n", MSPerFrame, FPS, MCPF);
                
                LastCycleCount = EndCycleCount;
                LastCounter = EndCounter;
            }
        }
        else
        {
            // TODO(casey): Logging 
        }
    }
    else
    {
        // TODO(casey): Logging
    }
    
    SDLCloseGameControllers();
    SDL_Quit();
    return(0);
}