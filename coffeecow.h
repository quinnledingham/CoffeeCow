#ifndef COFFEECOW_H
#define COFFEECOW_H

#define QLIB_CONTROLLER
struct Controller
{
    union
    {
        struct
        {
            Button right;
            Button up;
            Button left;
            Button down;
            Button select;
        };
        Button buttons[5];
    };
};

#define QLIB_STORAGE
struct Storage
{
    Mesh rect;
    
    u32 game_mode;
    s32 active;
};

#endif //COFFEECOW_H
