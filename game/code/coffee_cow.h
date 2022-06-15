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
    int NextDirection;
    bool32 Streak;
};

struct ServerCoffeeCow
{
    ServerCoffeeCowNode Nodes[289];
    int NumOfNodes;
    real32 TransitionAmt = 0;
    real32 Speed;
    int Direction;
    int Score;
};

void SendCoffeeCow(ServerCoffeeCow *Cow, Server *server)
{
    
}

#endif //COFFEE_COW_H
