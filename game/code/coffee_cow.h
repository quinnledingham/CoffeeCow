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
#define BUFFER_SIZE sizeof(game_packet) + HEADER_BUFFER_SIZE
#define SEND_BUFFER_SIZE sizeof(game_packet)

#endif //COFFEE_COW_H
