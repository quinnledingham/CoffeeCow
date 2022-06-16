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

struct game_state
{
    HANDLE serverMutex;
    Server server;
    
    int PlayersConnected = 0;
    int MaxPlayers = 4;
    ServerCoffeeCow Players[4];
    HANDLE PlayerMutexes[4];
    
    HANDLE CollectMutex;
    ServerCoffee Collect;
};

struct client_info
{
    int sockClient;
    ServerCoffeeCow *Cow;
    HANDLE *CowMutex;
};
/*
internal ServerCoffeeCow
GetPlayer(game_state *GameState, int i)
{
    ServerCoffeeCow *Cow = 0;
    switch(WaitForSingleObject(GameState->PlayerMutexes[i], INFINITE))
    {
        case WAIT_OBJECT_0: _try 
        {
            //Cow =
            int j = 0;
        }
        _finally{if(!ReleaseMutex(GameState->PlayerMutexes[i])){}}break;case WAIT_ABANDONED:return false;
    }
    return *Cow;
}
*/
global_variable game_state GameState = {};

DWORD WINAPI ServerFunction(LPVOID lpParam)
{
    client_info *Info = (client_info*)lpParam;
    char Buffer[10000];
    
    while(1) {
        game_packet *Recv = 0;
        ServerCoffeeCow *Cow = 0;
        memset(Buffer, 0, 10000);
        
        switch(WaitForSingleObject(GameState.serverMutex, INFINITE))
        {
            case WAIT_OBJECT_0: _try 
            {
                GameState.server.recvq(Info->sockClient, Buffer, 10000);
            }
            _finally{if(!ReleaseMutex(GameState.serverMutex)){}}break;case WAIT_ABANDONED:return false;
        }
        
        Recv = (game_packet*)Buffer;
        if (Recv->Disconnect) {
            return true;
        }
        Cow = &Recv->Cow;
        
        switch(WaitForSingleObject(*Info->CowMutex, INFINITE))
        {
            case WAIT_OBJECT_0: _try 
            {
                *Info->Cow = *Cow;
            }
            _finally{if(!ReleaseMutex(*Info->CowMutex)){}}break;case WAIT_ABANDONED:return false;
        }
        
        memset(Buffer, 0, 10000);
        game_packet Send = {};
        
        for (int i = 0; i < GameState.PlayersConnected; i++) {
            switch(WaitForSingleObject(GameState.PlayerMutexes[i], INFINITE))
            {
                case WAIT_OBJECT_0: _try 
                {
                    if (GameState.Players[i].ID != Info->Cow->ID) {
                        Send.Cow = GameState.Players[i];
                    }
                }
                _finally{if(!ReleaseMutex(GameState.PlayerMutexes[i])){}}break;case WAIT_ABANDONED:return false;
            }
        }
        
        memcpy(Buffer, &Send, sizeof(game_packet));
        
        switch(WaitForSingleObject(GameState.serverMutex, INFINITE))
        {
            case WAIT_OBJECT_0: _try 
            {
                GameState.server.sendq(Info->sockClient, Buffer, 9000);
            }
            _finally{if(!ReleaseMutex(GameState.serverMutex)){}}break;case WAIT_ABANDONED:return false;
        }
    } // while 1
}

int main(int argc, const char** argv)
{
    DWORD dwThreadIdArray[MAX_THREADS];
    HANDLE hThreadArray[MAX_THREADS]; 
    
    GameState.serverMutex = CreateMutex(NULL, FALSE, NULL);
    GameState.CollectMutex = CreateMutex(NULL, FALSE, NULL);
    
    for (int i = 0; i < GameState.MaxPlayers; i++) {
        GameState.PlayerMutexes[i] = CreateMutex(NULL, FALSE, NULL);
        GameState.Players[i] = {};
        GameState.Players[i].ID = i;
    }
    
    GameState.server.create("44575", TCP);
    
    client_info Info = {};
    
    while (1) {
        while(GameState.PlayersConnected >= MAX_THREADS) {}
        
        int PlyrsCncted = GameState.PlayersConnected;
        Info.sockClient = GameState.server.waitForConnection();
        Info.Cow = &GameState.Players[PlyrsCncted];
        Info.CowMutex = &GameState.PlayerMutexes[PlyrsCncted];
        
        hThreadArray[PlyrsCncted] = CreateThread(NULL, 0, ServerFunction, &Info, 0, &dwThreadIdArray[PlyrsCncted]);
        
        GameState.PlayersConnected++;
    }
}