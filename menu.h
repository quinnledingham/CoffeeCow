#ifndef MENU_H
#define MENU_H

struct Menu_Button
{
    v2 dim;
    v4 back_color;
    v4 active_back_color;
    v4 text_color;
    v4 active_text_color;
    f32 pixel_height;
};

struct Menu
{
    Menu_Button button;
    
    Menu_Button buttons[10];
    u32 num_of_buttons;
    
    v2 padding;
    v2 dim;
    Font *font;
};

function void
menu_add_button(Menu *menu, Menu_Button *button)
{
    
}

#endif //MENU_H
