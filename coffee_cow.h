#ifndef COFFEE_COW_H
#define COFFEE_COW_H

struct Coffee_Cow_Node
{
    v2s coords;
    v2s direction;
    v2s last_direction;
    u32 spot; // 0, 1, 2 are spots. 2 > is no spot
    
    b8 max_transition;
};

enum Asset_Tags
{
    ASSET_COW_HEAD, // 0
    ASSET_COW_HEAD_OUTLINE,
    ASSET_COW_CIRCLE,
    ASSET_COW_CIRCLE_OUTLINE,
    ASSET_COW_MOUTH,
    ASSET_COW_TAIL,
    ASSET_COW_SPOT, // 6 - 8
};

struct Coffee_Cow_Design
{
    Bitmap *bitmaps[9];
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
    
    u32 score;
    b8 dead;
    
    // input
    Controller *controller;
    v2s inputs[5];
    s32 num_of_inputs;
    
    b32 add;
    v2s add_coords;
    v2s add_direction;
    
    b8 open_mouth;
    r32 mouth_transition; // 0.0f - 1.0f

    b8 tail_dir;
    r32 tail_wag;

    Coffee_Cow_Design design;
};

struct Coffee
{
    v2s coords;
    r32 rotation;
    b32 consumed;
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
