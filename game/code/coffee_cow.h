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

void SendCoffeeCow(ServerCoffeeCow *Cow, Server *server)
{
    
}

#define SEND_BUFFER_SIZE sizeof(game_packet) + 100
#define BUF_SIZE SEND_BUFFER_SIZE + HEADER_BUFFER_SIZE


#endif //COFFEE_COW_H
