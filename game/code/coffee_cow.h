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
    int ID;
    
    HANDLE SenderThread; // Thread that sends updated data of players
    int SenderSock;
    
    HANDLE ReceiverThread; // Thread that gets data from player
    int ReceiverSock;
    
    ServerCoffeeCow *Cow;
    bool32 Connected;
};

struct server_state
{
    Server server;
    
    int MaxPlayerCount = 4;
    player Players[4];
    
    int ThreadCount = 0;
};

#define SEND_BUFFER_SIZE sizeof(game_packet) + 100
#define BUF_SIZE SEND_BUFFER_SIZE + HEADER_BUFFER_SIZE

#endif //COFFEE_COW_H
