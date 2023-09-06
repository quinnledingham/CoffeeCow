struct Menu_Button
{
    v2 dim;
    v4 back_color;
    v4 active_back_color;
    v4 text_color;
    v4 active_text_color;
    f32 pixel_height;
};

struct Menu_Bitmap
{
    v2 dim;
};

struct Menu
{
    Menu_Button button;
    Menu_Bitmap bitmap;
    v2 padding;
    
    r32 window_percent;
    r32 button_percent;
    r32 padding_percent;
    r32 pixel_height_percent;
    
    Font *font;
    Rect rect;
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
menu_button(Menu *menu, const char *text, u32 index, u32 active, u32 press)
{
    Menu_Button *button = &menu->button;
    v2 coords = menu->rect.coords;
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
    
    menu->rect.coords.y += menu->button.dim.y + menu->padding.y;
    
    return button_pressed;
}

function void
resize_menu(Menu *menu, 
            Rect window_rect, 
            v2s bitmap_dim,
            u32 num_of_bitmaps,
            u32 num_of_buttons)
{
    Rect bounds = get_centered_square(window_rect, menu->window_percent);
    menu->padding = bounds.dim * menu->padding_percent;
    
    if (num_of_bitmaps != 0) menu->bitmap.dim = { bounds.dim.width, bitmap_dim.height * (bounds.dim.width / bitmap_dim.width) };
    
    menu->button.dim = { bounds.dim.width, bounds.dim.height * menu->button_percent };
    menu->button.pixel_height = menu->button.dim.height * menu->pixel_height_percent;
    
    menu->rect.dim.x = menu->button.dim.width;
    menu->rect.dim.y = (menu->bitmap.dim.y * (r32)num_of_bitmaps) + (menu->button.dim.y * (r32)num_of_buttons) + (menu->padding.y * (r32)(num_of_buttons - 1));
    menu->rect.coords = get_centered(menu->rect, window_rect);
}

function b32
menu_multiplayer_selector(Menu *menu, u32 index, u32 active, u32 press,
                          v2 *coords, v2 dim, Controller *controller, Controller *saved_controller,
                          Bitmap *regular, Bitmap *hover, Bitmap *selected, Bitmap *hover_selected)
{
    b32 button_pressed = false;

    if (index == active)
    {
        if (press) button_pressed = true;
    }

    if (saved_controller == 0)
    {
        if      (index != active) draw_rect(*coords, 0, dim, regular);
        else if (index == active) draw_rect(*coords, 0, dim, hover);
    }
    else
    {
        if      (index != active) draw_rect(*coords, 0, dim, selected);
        else if (index == active) draw_rect(*coords, 0, dim, hover_selected);
    }
    coords->x += dim.x;

    return button_pressed;
}