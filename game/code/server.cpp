#define _CRT_SECURE_NO_WARNINGS
#define WIN32_LEAN_AND_MEAN
#define WIN32_EXTRA_LEAN

#include <windows.h>
#include <iostream>
#include <stdio.h>
#include <intrin.h>

#include "qlib/types.h"
//#include "qlib/win32_application.cpp"
#include "qlib/socketq.cpp"
#include "qlib/data_structures.h"

#pragma comment(linker, "/subsystem:console")
#define MAX_THREADS 8

// Multithreading
struct platform_work_queue;
#define PLATFORM_WORK_QUEUE_CALLBACK(name) void name(platform_work_queue *Queue, void *Data)
typedef PLATFORM_WORK_QUEUE_CALLBACK(platform_work_queue_callback);

typedef void platform_add_entry(platform_work_queue *Queue, platform_work_queue_callback *Callback, void *Data);
typedef void platform_complete_all_work(platform_work_queue *Queue);

struct platform_work_queue_entry
{
    platform_work_queue_callback *Callback;
    void *Data;
};

struct platform_work_queue
{
    uint32 volatile CompletionGoal;
    uint32 volatile CompletionCount;
    
    uint32 volatile NextEntryToWrite;
    uint32 volatile NextEntryToRead;
    HANDLE SemaphoreHandle;
    
    platform_work_queue_entry Entries[256];
};

#include "coffee_cow.h"

internal void
Win32AddEntry(platform_work_queue *Queue, platform_work_queue_callback *Callback, void *Data)
{
    // TODO(casey): Switch to InterlockedCompareExchange eventually
    // so that any thread can add?
    uint32 NewNextEntryToWrite = (Queue->NextEntryToWrite + 1) % ArrayCount(Queue->Entries);
    Assert(NewNextEntryToWrite != Queue->NextEntryToRead);
    platform_work_queue_entry *Entry = Queue->Entries + Queue->NextEntryToWrite;
    Entry->Callback = Callback;
    Entry->Data = Data;
    ++Queue->CompletionGoal;
    _WriteBarrier();
    _mm_sfence();
    Queue->NextEntryToWrite = NewNextEntryToWrite;
    ReleaseSemaphore(Queue->SemaphoreHandle, 1, 0);
}

internal bool32
Win32DoNextWorkQueueEntry(platform_work_queue *Queue)
{
    bool32 WeShouldSleep = false;
    
    uint32 OriginalNextEntryToRead = Queue->NextEntryToRead;
    uint32 NewNextEntryToRead = (OriginalNextEntryToRead + 1) % ArrayCount(Queue->Entries);
    if(OriginalNextEntryToRead != Queue->NextEntryToWrite)
    {
        uint32 Index = InterlockedCompareExchange((LONG volatile *)&Queue->NextEntryToRead,
                                                  NewNextEntryToRead,
                                                  OriginalNextEntryToRead);
        if(Index == OriginalNextEntryToRead)
        {        
            platform_work_queue_entry Entry = Queue->Entries[Index];
            Entry.Callback(Queue, Entry.Data);
            InterlockedIncrement((LONG volatile *)&Queue->CompletionCount);
        }
    }
    else
    {
        WeShouldSleep = true;
    }
    
    return(WeShouldSleep);
}

internal void
Win32CompleteAllWork(platform_work_queue *Queue)
{
    while(Queue->CompletionGoal != Queue->CompletionCount)
    {
        Win32DoNextWorkQueueEntry(Queue);
    }
    
    Queue->CompletionGoal = 0;
    Queue->CompletionCount = 0;
}

internal void
Win32WaitForAllWork(platform_work_queue *Queue)
{
    while(Queue->CompletionGoal != Queue->CompletionCount){}
    
    Queue->CompletionGoal = 0;
    Queue->CompletionCount = 0;
}

internal void
Win32CancelAllWork(platform_work_queue *Queue)
{
    Queue->CompletionGoal = 0;
    Queue->CompletionCount = 0;
}

struct win32_thread_info
{
    int LogicalThreadIndex;
    platform_work_queue *Queue;
};
DWORD WINAPI
ThreadProc(LPVOID lpParameter)
{
    win32_thread_info *ThreadInfo = (win32_thread_info *)lpParameter;
    
    for(;;)
    {
        if(Win32DoNextWorkQueueEntry(ThreadInfo->Queue))
        {
            WaitForSingleObjectEx(ThreadInfo->Queue->SemaphoreHandle, INFINITE, FALSE);
        }
    }
    
    //    return(0);
}

internal PLATFORM_WORK_QUEUE_CALLBACK(DoWorkerWork)
{
    char Buffer[256];
    wsprintf(Buffer, "Thread %u: %s\n", GetCurrentThreadId(), (char *)Data);
    OutputDebugStringA(Buffer);
}
// End of Multithreading

global_variable server_state ServerState = {};

internal bool32
GetConnected(player *Player)
{
    bool32 c = 0;
    switch(WaitForSingleObject(Player->cMutex, INFINITE))
    {
        case WAIT_OBJECT_0: _try 
        {
            c = Player->Connected;
        }
        _finally{if(!ReleaseMutex(Player->cMutex)){}}break;case WAIT_ABANDONED:return false;
    }
    return c;
}
internal int
SetConnected(player *Player, int8 i)
{
    switch(WaitForSingleObject(Player->cMutex, INFINITE))
    {
        case WAIT_OBJECT_0: _try 
        {
            Player->Connected = i;
        }
        _finally{if(!ReleaseMutex(Player->cMutex)){}}break;case WAIT_ABANDONED:return false;
    }
    return true;
}

internal int
GetSock(player *Player)
{
    int c = 0;
    switch(WaitForSingleObject(Player->cMutex, INFINITE))
    {
        case WAIT_OBJECT_0: _try 
        {
            c = Player->Sock;
        }
        _finally{if(!ReleaseMutex(Player->cMutex)){}}break;case WAIT_ABANDONED:return false;
    }
    return c;
}
internal int
SetSock(player *Player, int Sock)
{
    switch(WaitForSingleObject(Player->cMutex, INFINITE))
    {
        case WAIT_OBJECT_0: _try 
        {
            Player->Sock = Sock;
        }
        _finally{if(!ReleaseMutex(Player->cMutex)){}}break;case WAIT_ABANDONED:return false;
    }
    return true;
}


internal PLATFORM_WORK_QUEUE_CALLBACK(SendData)
{
    player *Player = (player*)Data;
    
    if (GetConnected(Player)) {
        game_packet Packet = {};
        Packet.Disconnect = 0;
        
        for (int i = 0; i < ServerState.MaxPlayerCount; i++) {
            player *OtherPlayer = &ServerState.Players[i];
            if (GetConnected(OtherPlayer) && GetSock(OtherPlayer) != GetSock(Player)) {
                Packet.Cow = OtherPlayer->Cow;
            }
        }
        
        char Buffer[BUF_SIZE];
        memset(Buffer, 0, BUF_SIZE);
        memcpy(Buffer, &Packet, sizeof(game_packet));
        ServerState.server.sendq(Player->Sock, Buffer, SEND_BUFFER_SIZE);
    }
}

internal PLATFORM_WORK_QUEUE_CALLBACK(RecvData)
{
    player *Player = (player*)Data;
    
    if (GetConnected(Player)) {
        char Buffer[BUF_SIZE];
        memset(Buffer, 0, BUF_SIZE);
        ServerState.server.recvq(GetSock(Player), Buffer, BUF_SIZE);
        game_packet *Recv = (game_packet*)Buffer;
        
        /*
        char Buffer2[256];
        wsprintf(Buffer2, "%f\n", Recv->Cow.TransitionAmt);
        OutputDebugStringA(Buffer);
        */
        //printf("%f\n", Recv->Cow.TransitionAmt);
        
        if (Recv->Disconnect == 1) {
            SetConnected(Player, 0);
            closesocket(GetSock(Player));
        }
        else if (!Recv->Disconnect) {
            for (int i = 0; i < ServerState.MaxPlayerCount; i++) {
                player *OtherPlayer = &ServerState.Players[i];
                if (OtherPlayer->Connected && GetSock(OtherPlayer) == GetSock(Player)) {
                    OtherPlayer->Cow = Recv->Cow; 
                }
            }
        } 
    } // Player->Connected
}

internal PLATFORM_WORK_QUEUE_CALLBACK(AddPlayer)
{
    
}

DWORD WINAPI SendRecvFunction(LPVOID lpParam)
{
    while(1) {
        for (int i = 0; i < ServerState.MaxPlayerCount; i++) {
            player *Player = &ServerState.Players[i];
            if (GetConnected(Player)) {
                Win32AddEntry(&ServerState.Queue, SendData, Player);
                Win32AddEntry(&ServerState.Queue, RecvData, Player);
            }
        }
        Win32CompleteAllWork(&ServerState.Queue);
        if (ServerState.NewPlayer) {
            int i = 0;
            player *Player = 0;
            while(Player == 0) {
                if (!GetConnected(&ServerState.Players[i]))
                    Player = &ServerState.Players[i];
                i++;
            }
            SetConnected(Player, 1);
            SetSock(Player, ServerState.NewPlayer);
            ServerState.NewPlayer = 0;
        }
    }
}

int main(int argc, const char** argv)
{
    win32_thread_info ThreadInfo[7];
    
    uint32 InitialCount = 0;
    uint32 ThreadCount = ArrayCount(ThreadInfo);
    ServerState.Queue.SemaphoreHandle = CreateSemaphoreEx(0, InitialCount, ThreadCount, 0, 0, SEMAPHORE_ALL_ACCESS);
    for(uint32 ThreadIndex = 0; ThreadIndex < ThreadCount; ++ThreadIndex) {
        win32_thread_info *Info = ThreadInfo + ThreadIndex;
        Info->Queue = &ServerState.Queue;
        Info->LogicalThreadIndex = ThreadIndex;
        
        DWORD ThreadID;
        HANDLE ThreadHandle = CreateThread(0, 0, ThreadProc, Info, 0, &ThreadID);
        CloseHandle(ThreadHandle);
    }
    
    DWORD ThreadID;
    HANDLE ThreadHandle = CreateThread(0, 0, SendRecvFunction, 0, 0, &ThreadID);
    CloseHandle(ThreadHandle);
    
    ServerState.server.create("44575", TCP);
    
    for (int i = 0; i < ServerState.MaxPlayerCount; i++) {
        player *Player = &ServerState.Players[i];
        Player->cMutex = CreateMutex(NULL, FALSE, NULL);
    }
    
    int NewSock = 0;
    while (1) {
        ServerState.NewPlayer = ServerState.server.waitForConnection();
    }
}