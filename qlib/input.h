#ifndef INPUT_H
#define INPUT_H

struct Button
{
    s32 id;
    b32 current_state; 
    b32 previous_state;
};

inline void
set(Button *button, s32 id)
{
    button->id = id;
}

inline b32
on_down(Button button)
{
    if (button.current_state && button.current_state != button.previous_state) return true;
    else return false;
}

inline b32
is_down(Button button)
{
    if (button.current_state) return true;
    else return false;
}

#endif //INPUT_H
