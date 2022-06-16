#define _CRT_SECURE_NO_WARNINGS
#define WIN32_LEAN_AND_MEAN
#define WIN32_EXTRA_LEAN
#include <windows.h>
#include <iostream>

#include "qlib/types.h"
#include "qlib/socketq.cpp"
#include "qlib/data_structures.h"
#include "coffee_cow.h"

#define MAX_THREADS 4
#define BUF_SIZE 255

struct player
{
    int ID;
    bool32 Connected;
    
    int sockClient;
    ServerCoffeeCow Cow;
    HANDLE CowMutex;
};

struct game_state
{
    Server server;
    HANDLE serverMutex;
    
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

DWORD WINAPI ServerFunction(LPVOID lpParam)
{
    player *Player = (player*)lpParam;
    char Buffer[BUFFER_SIZE];
    printf("%d\n", Player->sockClient);
    
    while(1) {
        game_packet *Recv = 0;
        ServerCoffeeCow *Cow = 0;
        memset(Buffer, 0, BUFFER_SIZE);
        
        switch(WaitForSingleObject(GameState.serverMutex, INFINITE))
        {
            case WAIT_OBJECT_0: _try 
            {
                GameState.server.recvq(Player->sockClient, Buffer, BUFFER_SIZE);
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
        
        memset(Buffer, 0, BUFFER_SIZE);
        game_packet Send = {};
        int pc = GetPlayersConnected();
        for (int i = 0; i < pc; i++) {
            switch(WaitForSingleObject(GameState.Players[i].CowMutex, INFINITE))
            {
                case WAIT_OBJECT_0: _try 
                {
                    if (GameState.Players[i].ID != Player->ID) {
                        printf("pc: %d\n",GameState.Players[i].Cow.NumOfNodes);
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

int main(int argc, const char** argv)
{
    DWORD dwThreadIdArray[MAX_THREADS];
    HANDLE hThreadArray[MAX_THREADS]; 
    
    int i = sizeof(bool);
    
    GameState.serverMutex = CreateMutex(NULL, FALSE, NULL);
    GameState.CollectMutex = CreateMutex(NULL, FALSE, NULL);
    GameState.pcMutex = CreateMutex(NULL, FALSE, NULL);
    
    for (int i = 0; i < GameState.MaxPlayers; i++) {
        GameState.Players[i].ID = i;
        GameState.Players[i].CowMutex = CreateMutex(NULL, FALSE, NULL);
        GameState.Players[i].Connected = false;
    }
    
    GameState.server.create("44575", TCP);
    
    while (1) {
        while(GameState.PlayersConnected >= MAX_THREADS) {}
        
        int sock = GameState.server.waitForConnection();
        
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
        
        hThreadArray[PlyrsCncted] = CreateThread(NULL, 0, ServerFunction, Player, 0, &dwThreadIdArray[PlyrsCncted]);
        IncrementPlayersConnected();
    }
}