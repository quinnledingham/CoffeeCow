#define _CRT_SECURE_NO_WARNINGS
#define WIN32_LEAN_AND_MEAN
#define WIN32_EXTRA_LEAN

#include <windows.h>
#include <iostream>
#include <stdio.h>
#include <intrin.h>

#include "qlib/types.h"
#include "qlib/platform.h"
#include "qlib/application.h"
#include "qlib/win32_application.cpp"
#include "qlib/data_structures.h"
#include "qlib/strinq.h"
#include "qlib/strinq.cpp"
#include "qlib/socketq.h"
#include "qlib/win32_thread.h"

#include "coffee_cow.h"

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
    //fprintf(stderr, "Start SendData");
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
        //ServerState.Server.sendq(Player->Sock, Buffer, SEND_BUFFER_SIZE);
        SocketqSend(&ServerState.Server, Player->Sock, Buffer, SEND_BUFFER_SIZE);
    }
    //fprintf(stderr, "End SendData");
}

internal PLATFORM_WORK_QUEUE_CALLBACK(RecvData)
{
    player *Player = (player*)Data;
    
    if (GetConnected(Player)) {
        char Buffer[BUF_SIZE];
        memset(Buffer, 0, BUF_SIZE);
        //ServerState.server.recvq(GetSock(Player), Buffer, BUF_SIZE);
        SocketqRecv(&ServerState.Server, GetSock(Player), Buffer, BUF_SIZE);
        game_packet *PacketRecv = (game_packet*)Buffer;
        
        /*
        char Buffer2[256];
        wsprintf(Buffer2, "%f\n", Recv->Cow.TransitionAmt);
        OutputDebugStringA(Buffer);
        */
        printf("%f\n", PacketRecv->Cow.TransitionAmt);
        
        //fprintf(stderr, "Start RecvData\n");
        if (PacketRecv->Disconnect == 1) {
            Player->ToBeDisconnected = 1;
            
        }
        else if (PacketRecv->Disconnect == 0) {
            for (int i = 0; i < ServerState.MaxPlayerCount; i++) {
                player *OtherPlayer = &ServerState.Players[i];
                if (OtherPlayer->Connected && GetSock(OtherPlayer) == GetSock(Player)) {
                    OtherPlayer->Cow = PacketRecv->Cow; 
                }
            }
        } 
        //fprintf(stderr, "End RecvData\n");
    } // Player->Connected
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
        Sleep(1);
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
        
        for (int i = 0; i < ServerState.MaxPlayerCount; i++) {
            player *Player = &ServerState.Players[i];
            if (Player->ToBeDisconnected == 1) {
                SetConnected(Player, 0);
                closesocket(GetSock(Player));
                Player->ToBeDisconnected = 0;
            }
        }
        
        char CharBuffer[OUTPUTBUFFER_SIZE];
        _snprintf_s(CharBuffer, sizeof(CharBuffer), "%s", GlobalDebugBuffer.Data);
        OutputDebugStringA(CharBuffer);
        
        GlobalDebugBuffer = {};
        memset(GlobalDebugBuffer.Data, 0, GlobalDebugBuffer.Size);
        GlobalDebugBuffer.Next = GlobalDebugBuffer.Data;
    }
}

void Update(platform *p)
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
    
    //ServerState.server.create("44575", TCP);
    SocketqInit(&ServerState.Server, "44575", TCP);
    
    for (int i = 0; i < ServerState.MaxPlayerCount; i++) {
        player *Player = &ServerState.Players[i];
        Player->cMutex = CreateMutex(NULL, FALSE, NULL);
    }
    
    int NewSock = 0;
    while (1) {
        ServerState.NewPlayer = SocketqWaitForConnection(&ServerState.Server);
    }
}