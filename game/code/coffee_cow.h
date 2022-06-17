#ifndef COFFEE_COW_H
#define COFFEE_COW_H

struct ServerCoffee
{
    v2 Coords;
    real32 Rotation;
};

struct ServerCoffeeCowNode
{
    iv2 Coords;
    int8  CurrentDirection;
    int8 Streak;
};

struct ServerCoffeeCow
{
    ServerCoffeeCowNode Nodes[10];
    int8 NumOfNodes;
    real32 TransitionAmt = 0;
    int32 Score;
};

struct game_packet
{
    ServerCoffeeCow Cow;
    int8 Disconnect;
};

enum connection_type
{
    sender,
    receiver,
};
struct packet
{
    int ConnectionType;
    int ID;
};

struct player
{
    int Sock;
    ServerCoffeeCow Cow;
    bool32 Connected;
    HANDLE cMutex;
};

struct server_state
{
    Server server;
    
    int PlayersConnected = 0;
    int MaxPlayerCount = 4;
    player Players[4];
    
    int NewPlayer = 0;
    
    platform_work_queue Queue;
};

#define SEND_BUFFER_SIZE sizeof(game_packet) + 100
#define BUF_SIZE SEND_BUFFER_SIZE + HEADER_BUFFER_SIZE

#endif //COFFEE_COW_H
