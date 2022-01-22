#include "gui.h"
#include "memorymanager.h"
#include "snake.h"

internal int
addGUIComponent(GUIComponent* head, GUIComponent* newComponent)
{
    // Start linked list
    if(head == 0)
    {
        //head = newComponent;
        return 0;
    }
    
    GUIComponent* cursor = head;
    while(cursor->next != 0)
    {
        cursor = cursor->next;
    }
    
    cursor->next = newComponent;
    return 1;
}

internal void
addButton(GUI* g, int x, int y, int width, int height,
          uint32 color, uint32 hovercolor, char* text, uint32 textcolor, int id)
{
    Button newBtn = {};
    newBtn.Shape.x = x;
    newBtn.Shape.y = y;
    newBtn.Shape.width = width;
    newBtn.Shape.height = height;
    newBtn.Shape.color = color;
    
    newBtn.RegularColor = newBtn.Shape.color;
    newBtn.HoverColor = hovercolor;
    newBtn.Text = text;
    newBtn.TextColor = textcolor;
    newBtn.ID = id;
    
    GUIComponent newComponent = {};
    newComponent.component = PermanentStorageAssign(&newBtn, sizeof(Button));
    GUIComponent* memnewComponent = (GUIComponent*)PermanentStorageAssign(&newComponent, sizeof(GUIComponent));
    
    if(!addGUIComponent(g->buttons, memnewComponent))
    {
        g->buttons = memnewComponent;
    }
}

internal void
RenderButtons(game_offscreen_buffer *Buffer, GUI* g)
{
    GUIComponent* cursor = g->buttons;
    while(cursor != 0)
    {
        Rect* s = (Rect*)cursor->component;
        Button* b = (Button*)cursor->component;
        RenderRect(Buffer, s, FILL, s->color);
        PrintOnScreen(Buffer, b->Text, s->x, s->y, (float)s->height, b->TextColor, s);
        cursor = cursor->next;
    }
}

internal void
CheckButtonsHover(GUI* g, int32 MouseX, int32 MouseY)
{
    GUIComponent* cursor = g->buttons;
    while(cursor != 0)
    {
        Button* b = (Button*)cursor->component;
        if (b->Shape.x < MouseX && MouseX < (b->Shape.x + b->Shape.width) &&
            b->Shape.y < MouseY && MouseY < (b->Shape.y + b->Shape.height))
        {
            b->Shape.color = b->HoverColor;
        }
        else
        {
            b->Shape.color = b->RegularColor;
        }
        
        cursor = cursor->next;
    }
}

internal int
CheckButtons(GUI* g, int32 MouseX, int32 MouseY)
{
    GUIComponent* cursor = g->buttons;
    while(cursor != 0)
    {
        Rect* s = (Rect*)cursor->component;
        if (s->x < MouseX && MouseX < (s->x + s->width) &&
            s->y < MouseY && MouseY < (s->y + s->height))
        {
            Button* btn = (Button*)cursor->component;
            return btn->ID;
        }
        
        cursor = cursor->next;
    }
    
    return -1;
}