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
    v2 padding;
    Font *font;
};

function void
menu_update_active(s32 *active, s32 lower, s32 upper, Button increase, Button decrease)
{
    if (on_down(increase))
    {
        (*active)++;
        if (*active > upper)
            *active = upper;
    }
    if (on_down(decrease))
    {
        (*active)--;
        if (*active < lower)
            *active = lower;
    }
}

function b32
menu_button(Menu *menu, v2 coords, const char *text, u32 index, u32 active, u32 press)
{
    Menu_Button *button = &menu->button;
    b32 button_pressed = false;
    
    v4 b_color = button->back_color;
    v4 t_color = button->text_color;
    
    if (index == active)
    {
        b_color = button->active_back_color;
        t_color = button->active_text_color;
        if (press)
            button_pressed = true;
    }
    
    // drawing
    draw_rect(coords, 0, button->dim, b_color);
    
    v2 text_dim = get_string_dim(menu->font, text, button->pixel_height, t_color);
    f32 text_x_coord = coords.x + (button->dim.x / 2.0f) - (text_dim.x / 2.0f);
    f32 text_y_coord = coords.y + (button->dim.y / 2.0f) + (text_dim.y / 2.0f);
    
    draw_string(menu->font, text, { text_x_coord, text_y_coord }, button->pixel_height, t_color);
    
    return button_pressed;
}