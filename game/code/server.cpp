#define _CRT_SECURE_NO_WARNINGS
#define WIN32_LEAN_AND_MEAN
#define WIN32_EXTRA_LEAN
#include <windows.h>
#include <iostream>

#include "qlib/types.h"
#include "qlib/socketq.cpp"
#include "qlib/data_structures.h"
#include "coffee_cow.h"

#define MAX_THREADS 3
#define BUF_SIZE 255

struct game_state
{
    HANDLE iMutex;
    int i = 1;
    
    HANDLE serverMutex;
    Server server;
    
    HANDLE Player1Mutex;
    ServerCoffeeCow Player1;
    
    HANDLE Player2Mutex;
    ServerCoffeeCow Player2;
    
    HANDLE CollectMutex;
    ServerCoffee Collect;
};

DWORD WINAPI ServerFunction(LPVOID lpParam)
{
    game_state *GameState = (game_state*)lpParam;
    int Player = 0;
    
    switch(WaitForSingleObject(GameState->iMutex, INFINITE))
    {
        case WAIT_OBJECT_0:
        _try {
            Player = GameState->i;
            GameState->i++;
            printf("Player %d Joined\n", Player);
        }
        _finally {
            if (!ReleaseMutex(GameState->iMutex)) {}
        }
        break;
        case WAIT_ABANDONED:
        return false;
    }
    
    while(1) {
        char Buffer[10000];
        memset(Buffer, 0, 10000);
        GameState->server.recvq(Buffer, 10000);
        ServerCoffeeCow *Cow = (ServerCoffeeCow*)Buffer;
        
        if (Player == 1) {
            switch(WaitForSingleObject(GameState->Player1Mutex, INFINITE))
            {
                case WAIT_OBJECT_0:
                _try {
                    GameState->Player1 = *Cow;
                    printf("%d %d\n", Player, GameState->Player1.Direction);
                }
                _finally {
                    if (!ReleaseMutex(GameState->Player1Mutex)) {}
                }
                break;
                case WAIT_ABANDONED:
                return false;
            }
            switch(WaitForSingleObject(GameState->Player2Mutex, INFINITE))
            {
                case WAIT_OBJECT_0:
                _try {
                    char Buffer[10000];
                    memset(Buffer, 0, 10000);
                    memcpy(Buffer, &GameState->Player2, sizeof(ServerCoffeeCow));
                    GameState->server.sendq(Buffer, 9000);
                }
                _finally {
                    if (!ReleaseMutex(GameState->Player2Mutex)) {}
                }
                break;
                case WAIT_ABANDONED:
                return false;
            }
        }
        else if (Player == 2) {
            switch(WaitForSingleObject(GameState->Player2Mutex, INFINITE))
            {
                case WAIT_OBJECT_0:
                _try {
                    GameState->Player2 = *Cow;
                    printf("%d %d\n", Player, GameState->Player2.Direction);
                }
                _finally {
                    if (!ReleaseMutex(GameState->Player2Mutex)) {}
                }
                break;
                case WAIT_ABANDONED:
                return false;
            }
            switch(WaitForSingleObject(GameState->Player1Mutex, INFINITE))
            {
                case WAIT_OBJECT_0:
                _try {
                    char Buffer[10000];
                    memset(Buffer, 0, 10000);
                    memcpy(Buffer, &GameState->Player1, sizeof(ServerCoffeeCow));
                    GameState->server.sendq(Buffer, 9000);
                }
                _finally {
                    if (!ReleaseMutex(GameState->Player1Mutex)) {}
                }
                break;
                case WAIT_ABANDONED:
                return false;
            }
        }
    } // while 1
}

int main(int argc, const char** argv)
{
    DWORD   dwThreadIdArray[MAX_THREADS];
    HANDLE  hThreadArray[MAX_THREADS]; 
    int i = 0;
    
    game_state GameState = {};
    GameState.iMutex = CreateMutex(NULL, FALSE, NULL);
    GameState.serverMutex = CreateMutex(NULL, FALSE, NULL);
    GameState.Player1Mutex = CreateMutex(NULL, FALSE, NULL);
    GameState.Player2Mutex = CreateMutex(NULL, FALSE, NULL);
    GameState.CollectMutex = CreateMutex(NULL, FALSE, NULL);
    GameState.server.create("44575", TCP);
    
    while (1) {
        while(i >= MAX_THREADS) {}
        
        GameState.server.waitForConnection();
        hThreadArray[i] = CreateThread(NULL, 0, ServerFunction, &GameState, 0, &dwThreadIdArray[i]);
        i++;
    }
}