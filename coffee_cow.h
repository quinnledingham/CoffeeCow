#ifndef COFFEE_COW_H
#define COFFEE_COW_H

struct Coffee_Cow_Node
{
    v2s coords;
    v2s direction;
    v2s last_direction;
};

enum Asset_Tags
{
    ASSET_COW_HEAD,
    ASSET_COW_HEAD_OUTLINE,
    ASSET_COW_CIRCLE,
    ASSET_COW_CIRCLE_OUTLINE,
};

struct Coffee_Cow_Design
{
    Bitmap *bitmaps[4];
    v4 color;
    v4 outline_color;
};

struct Coffee_Cow
{
    // game info
    Coffee_Cow_Node nodes[400];  // 0 = head
    u32 num_of_nodes;
    r32 transition; // 0.0f - 1.0f
    v2s direction;
    
    // input
    Controller *controller;
    v2s inputs[5];
    s32 num_of_inputs;
    b8 first_input_of_transition;
    
    Coffee_Cow_Design design;
};

enum Direction
{
    RIGHT,
    UP,
    LEFT,
    DOWN
};

#define RIGHT_V v2s{ 1, 0 }
#define UP_V v2s{ 0, -1 }
#define LEFT_V v2s{ -1, 0 }
#define DOWN_V v2s{ 0, 1 }

#endif //COFFEE_COW_H
