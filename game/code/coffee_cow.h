#ifndef COFFEE_COW_H
#define COFFEE_COW_H

struct ServerCoffee
{
    v2 Coords;
    real32 Rotation;
};

struct ServerCoffeeCowNode
{
    v2 Coords;
    int CurrentDirection;
    bool32 Streak;
};

struct ServerCoffeeCow
{
    int ID;
    
    ServerCoffeeCowNode Nodes[289];
    int NumOfNodes;
    real32 TransitionAmt = 0;
    int Score;
};

struct game_packet
{
    ServerCoffeeCow Cow;
    bool32 Disconnect;
};

void SendCoffeeCow(ServerCoffeeCow *Cow, Server *server)
{
    
}

#endif //COFFEE_COW_H
