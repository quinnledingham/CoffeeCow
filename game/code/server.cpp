#define QLIB_CONSOLE_APPLICATION
#include "qlib/application.h"

#include "qlib/data_structures.h"
#include "qlib/socketq.h"

#include "coffee_cow.h"

global_variable server_state ServerState = {};

internal PLATFORM_WORK_QUEUE_CALLBACK(SendData)
{
    player *Player = (player*)Data;
    
    if (Player->Connected) {
        game_packet Packet = {};
        Packet.Disconnect = 0;
        
        for (int i = 0; i < ServerState.MaxPlayerCount; i++) {
            player *OtherPlayer = &ServerState.Players[i];
            if (OtherPlayer->Connected && OtherPlayer->Sock != Player->Sock) {
                Packet.Cow = OtherPlayer->Cow;
            }
        }
        
        char Buffer[BUF_SIZE];
        memset(Buffer, 0, BUF_SIZE);
        memcpy(Buffer, &Packet, sizeof(game_packet));
        SocketqSend(&ServerState.Server, Player->Sock, Buffer, SEND_BUFFER_SIZE);
    }
}

internal PLATFORM_WORK_QUEUE_CALLBACK(RecvData)
{
    player *Player = (player*)Data;
    
    if (Player->Connected) {
        char Buffer[BUF_SIZE];
        memset(Buffer, 0, BUF_SIZE);
        SocketqRecv(&ServerState.Server, Player->Sock, Buffer, BUF_SIZE);
        game_packet *PacketRecv = (game_packet*)Buffer;
        
        printf("%f\n", PacketRecv->Cow.TransitionAmt);
        
        if (PacketRecv->Disconnect == 1) {
            Player->ToBeDisconnected = 1;
            
        }
        else if (PacketRecv->Disconnect == 0) {
            for (int i = 0; i < ServerState.MaxPlayerCount; i++) {
                player *OtherPlayer = &ServerState.Players[i];
                if (OtherPlayer->Connected && OtherPlayer->Sock == Player->Sock) {
                    OtherPlayer->Cow = PacketRecv->Cow; 
                }
            }
        } 
    }
}

DWORD WINAPI WaitForConnectionFunction(LPVOID lpParam)
{
    while (1) {
        ServerState.NewPlayer = SocketqWaitForConnection(&ServerState.Server);
    }
}

void Update(platform *p)
{
    if (!ServerState.Initialized) {
        SocketqInit(&ServerState.Server, "44575", TCP);
        
        for (int i = 0; i < ServerState.MaxPlayerCount; i++) {
            player *Player = &ServerState.Players[i];
            Player->cMutex = CreateMutex(NULL, FALSE, NULL);
        }
        
        DWORD ThreadID;
        HANDLE ThreadHandle = CreateThread(0, 0, WaitForConnectionFunction, 0, 0, &ThreadID);
        CloseHandle(ThreadHandle);
        
        ServerState.Initialized = true;
    }
    
    // Send and Recv From Players
    for (int i = 0; i < ServerState.MaxPlayerCount; i++) {
        player *Player = &ServerState.Players[i];
        if (Player->Connected) {
            Win32AddEntry(&ServerState.Queue, SendData, Player);
            Win32AddEntry(&ServerState.Queue, RecvData, Player);
        }
    }
    Sleep(1);
    Win32CompleteAllWork(&ServerState.Queue);
    
    // Add new player if one connected
    if (ServerState.NewPlayer) {
        int i = 0;
        player *Player = 0;
        while(Player == 0) {
            if (!ServerState.Players[i].Connected)
                Player = &ServerState.Players[i];
            i++;
        }
        Player->Connected = true;
        Player->Sock = ServerState.NewPlayer;
        ServerState.NewPlayer = 0;
        
    }
    
    // Disconnect player if they were flagged to be disconnected
    for (int i = 0; i < ServerState.MaxPlayerCount; i++) {
        player *Player = &ServerState.Players[i];
        if (Player->ToBeDisconnected == 1) {
            closesocket(Player->Sock);
            Player->Connected = false;
            Player->ToBeDisconnected = false;
        }
    }
}