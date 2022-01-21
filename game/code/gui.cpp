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
addButton(GUI* g, int x, int y, int width, int height, uint32 color, char* text, int id)
{
    Button newBtn = {};
    newBtn.rect.x = x;
    newBtn.rect.y = y;
    newBtn.rect.width = width;
    newBtn.rect.height = height;
    newBtn.rect.color = color;
    newBtn.text = text;
    newBtn.id = id;
    
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
        RenderRect(Buffer, s, FILL, s->color);
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
            return btn->id;
        }
        
        cursor = cursor->next;
    }
    
    return -1;
}