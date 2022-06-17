#define _CRT_SECURE_NO_WARNINGS
#define WIN32_LEAN_AND_MEAN
#define WIN32_EXTRA_LEAN
#include <windows.h>
#include <iostream>
#include <stdio.h>

#include "qlib/types.h"
#include "qlib/socketq.cpp"
#include "qlib/data_structures.h"
#include "coffee_cow.h"

#define MAX_THREADS 8
/*
struct player
{
    int ID;
    bool32 Connected;
    
    HANDLE SenderThread;
    HANDLE ReceiverThread;
    
    int sockClient;
    ServerCoffeeCow Cow;
    HANDLE CowMutex;
};

struct game_state
{
    Server SendServer;
    Server RecvServer;
    
    int PlayersConnected = 0;
    HANDLE pcMutex;
    
    int MaxPlayers = 4;
    player Players[4];
    
    ServerCoffee Collect;
    HANDLE CollectMutex;
};

global_variable game_state GameState = {};

internal int
GetPlayersConnected()
{
    int r = 0;
    switch(WaitForSingleObject(GameState.pcMutex, INFINITE))
    {
        case WAIT_OBJECT_0: _try 
        {
            r = GameState.PlayersConnected;
        }
        _finally{if(!ReleaseMutex(GameState.pcMutex)){}}break;case WAIT_ABANDONED:return false;
    }
    return r;
}

internal DWORD
IncrementPlayersConnected()
{
    switch(WaitForSingleObject(GameState.pcMutex, INFINITE))
    {
        case WAIT_OBJECT_0: _try 
        {
            GameState.PlayersConnected++;
        }
        _finally{if(!ReleaseMutex(GameState.pcMutex)){}}break;case WAIT_ABANDONED:return false;
    }
    return true;
}

internal DWORD
DecrementPlayersConnected()
{
    switch(WaitForSingleObject(GameState.pcMutex, INFINITE))
    {
        case WAIT_OBJECT_0: _try 
        {
            GameState.PlayersConnected--;
        }
        _finally{if(!ReleaseMutex(GameState.pcMutex)){}}break;case WAIT_ABANDONED:return false;
    }
    return true;
}
*/
/*
DWORD WINAPI ServerFunction(LPVOID lpParam)
{
    player *Player = (player*)lpParam;
    //printf("%d\n", Player->sockClient);
    
    char Buffer[BUF_SIZE];
    while(1) {
        game_packet *Recv = 0;
        ServerCoffeeCow *Cow = 0;
        memset(Buffer, 0, BUF_SIZE);
        
        switch(WaitForSingleObject(GameState.serverMutex, INFINITE))
        {
            case WAIT_OBJECT_0: _try 
            {
                GameState.server.recvq(Player->sockClient, Buffer, BUF_SIZE);
            }
            _finally{if(!ReleaseMutex(GameState.serverMutex)){}}break;case WAIT_ABANDONED:return false;
        }
        
        Recv = (game_packet*)Buffer;
        if (Recv->Disconnect) {
            switch(WaitForSingleObject(Player->CowMutex, INFINITE))
            {
                case WAIT_OBJECT_0: _try 
                {
                    Player->Connected = false;
                    //CloseHandle(Player->CowMutex);
                    closesocket(Player->sockClient);
                }
                _finally{if(!ReleaseMutex(Player->CowMutex)){}}break;case WAIT_ABANDONED:return false;
            }
            DecrementPlayersConnected();
            return true;
        }
        Cow = &Recv->Cow;
        
        switch(WaitForSingleObject(Player->CowMutex, INFINITE))
        {
            case WAIT_OBJECT_0: _try 
            {
                Player->Cow = *Cow;
            }
            _finally{if(!ReleaseMutex(Player->CowMutex)){}}break;case WAIT_ABANDONED:return false;
        }
        
        memset(Buffer, 0, BUF_SIZE);
        game_packet Send = {};
        int pc = GetPlayersConnected();
        for (int i = 0; i < pc; i++) {
            switch(WaitForSingleObject(GameState.Players[i].CowMutex, INFINITE))
            {
                case WAIT_OBJECT_0: _try 
                {
                    if (GameState.Players[i].ID != Player->ID) {
                        //printf("pc: %d\n",GameState.Players[i].Cow.NumOfNodes);
                        Send.Cow = GameState.Players[i].Cow;
                    }
                }
                _finally{if(!ReleaseMutex(GameState.Players[i].CowMutex)){}}break;case WAIT_ABANDONED:return false;
            }
        }
        
        memcpy(Buffer, &Send, sizeof(game_packet));
        
        switch(WaitForSingleObject(GameState.serverMutex, INFINITE))
        {
            case WAIT_OBJECT_0: _try 
            {
                GameState.server.sendq(Player->sockClient, Buffer, SEND_BUFFER_SIZE);
            }
            _finally{if(!ReleaseMutex(GameState.serverMutex)){}}break;case WAIT_ABANDONED:return false;
        }
    } // while 1
}
*/

/*
DWORD WINAPI ReceiverFunction(LPVOID lpParam)
{
    player *Player = (player*)lpParam;
    
    char Buffer[BUF_SIZE];
    while (1) {
        game_packet *Recv = 0;
        ServerCoffeeCow *Cow = 0;
        memset(Buffer, 0, BUF_SIZE);
        GameState.server.recvq(Player->sockClient, Buffer, BUF_SIZE);
        Cow = &Recv->Cow;
        
        Recv = (game_packet*)Buffer;
        if (Recv->Disconnect) {
            switch(WaitForSingleObject(Player->CowMutex, INFINITE))
            {
                case WAIT_OBJECT_0: _try 
                {
                    Player->Connected = false;
                    //CloseHandle(Player->CowMutex);
                    closesocket(Player->sockClient);
                }
                _finally{if(!ReleaseMutex(Player->CowMutex)){}}break;case WAIT_ABANDONED:return false;
            }
            DecrementPlayersConnected();
            return true;
        }
    }
}
*/
/*
int main(int argc, const char** argv)
{
    //DWORD dwThreadIdArray[MAX_THREADS];
    //HANDLE hThreadArray[MAX_THREADS]; 
    
    //GameState.serverMutex = CreateMutex(NULL, FALSE, NULL);
    //GameState.CollectMutex = CreateMutex(NULL, FALSE, NULL);
    GameState.pcMutex = CreateMutex(NULL, FALSE, NULL);
    
    for (int i = 0; i < GameState.MaxPlayers; i++) {
        GameState.Players[i].ID = i;
        GameState.Players[i].CowMutex = CreateMutex(NULL, FALSE, NULL);
        GameState.Players[i].Connected = false;
    }
    
    IncrementPlayersConnected();
    //hThreadArray[0] = CreateThread(NULL, 0, SenderFunction, 0, 0, &dwThreadIdArray[0]);
    
    GameState.SendServer.create("44575", UDP);
    char Buffer[BUF_SIZE];
    while(1) {
        memset(Buffer, 0, BUF_SIZE);
        game_packet Send = {};
        Send.Disconnect = true;
        int pc = GetPlayersConnected();
        for (int i = 0; i < pc; i++) {
            switch(WaitForSingleObject(GameState.Players[i].CowMutex, INFINITE))
            {
                case WAIT_OBJECT_0: _try 
                {
                    Send.Cow = GameState.Players[i].Cow;
                }
                _finally{if(!ReleaseMutex(GameState.Players[i].CowMutex)){}}break;case WAIT_ABANDONED:return false;
            }
        }
        memcpy(Buffer, &Send, sizeof(game_packet));
        GameState.SendServer.sendq(0, Buffer, SEND_BUFFER_SIZE);
    }
    
    //WaitForMultipleObjects(1, hThreadArray, TRUE, INFINITE);
    
    //GameState.server.create("44575", TCP);
    
    /*
    while (1) {
        while(GameState.PlayersConnected >= MAX_THREADS) {}
        
        //int sock = GameState.server.waitForConnection();
        
        int PlyrsCncted = GetPlayersConnected();
        player *Player = 0;
        
        int i = 0;
        while(Player == 0) {
            switch(WaitForSingleObject(GameState.Players[i].CowMutex, INFINITE))
            {
                case WAIT_OBJECT_0: _try 
                {
                    if (!GameState.Players[i].Connected) {
                        Player = &GameState.Players[i];
                        Player->sockClient = sock;
                        Player->Connected = true;
                    }
                }
                _finally{if(!ReleaseMutex(GameState.Players[i].CowMutex)){}}break;case WAIT_ABANDONED:return false;
            }
            i++;
        }
        
        hThreadArray[PlyrsCncted] = CreateThread(NULL, 0, SenderFunction, Player, 0, &dwThreadIdArray[PlyrsCncted]);
        IncrementPlayersConnected();
    }

}
*/

global_variable server_state ServerState = {};

DWORD WINAPI SenderFunction(LPVOID lpParam)
{
    player *Player = (player*)lpParam;
    
    char Buffer[BUF_SIZE];
    while(1) {
        game_packet Packet = {};
        Packet.Disconnect = 1;
        memset(Buffer, 0, BUF_SIZE);
        memcpy(Buffer, &Packet, sizeof(game_packet));
        ServerState.server.sendq(Player->ReceiverSock, Buffer, SEND_BUFFER_SIZE);
        //printf("%d\n", Player->ID);
    }
}

DWORD WINAPI ReceiverFunction(LPVOID lpParam)
{
    player *Player = (player*)lpParam;
    while(1) {
        printf("%d\n", Player->ID);
    }
}


int main(int argc, const char** argv)
{
    DWORD dwThreadIdArray[MAX_THREADS];
    //HANDLE hThreadArray[MAX_THREADS]; 
    
    ServerState.server.create("44575", TCP);
    
    char Buffer[BUF_SIZE];
    while (1) {
        int NewSock = ServerState.server.waitForConnection();
        memset(Buffer, 0, BUF_SIZE);
        ServerState.server.recvq(NewSock, Buffer, BUF_SIZE);
        
        packet *Packet = (packet*)Buffer; // First thing sent after connection is what type of thread to open
        player *NewPlayer = 0;
        
        // Look to see if player is already connected in some way
        for (int i = 0; i < ServerState.MaxPlayerCount; i++) {
            if (ServerState.Players[i].Connected) {
                if (ServerState.Players[i].ID == Packet->ID) {
                    NewPlayer = &ServerState.Players[i];
                }
            }
        }
        int i = 0;
        while (NewPlayer == 0) {
            if (!ServerState.Players[i].Connected) {
                NewPlayer = &ServerState.Players[i];
                NewPlayer->ID = Packet->ID;
                NewPlayer->Connected = true;
            }
            i++;
        }
        
        if (Packet->ConnectionType == sender) {
            NewPlayer->SenderSock = NewSock;
            NewPlayer->ReceiverThread = CreateThread(NULL, 0, ReceiverFunction, NewPlayer, 
                                                     0, &dwThreadIdArray[ServerState.ThreadCount]);
        }
        else if (Packet->ConnectionType == receiver) {
            NewPlayer->ReceiverSock = NewSock;
            NewPlayer->SenderThread = CreateThread(NULL, 0, SenderFunction, NewPlayer, 
                                                   0, &dwThreadIdArray[ServerState.ThreadCount]);
        }
    }
}