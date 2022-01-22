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

internal int
MouseInRect(Rect* R, int32 MouseX, int32 MouseY)
{
    if (R->x < MouseX && MouseX < (R->x + R->width) &&
        R->y < MouseY && MouseY < (R->y + R->height))
    {
        return 1;
    }
    return 0;
}

internal void
addTextBox(GUI* g, int x, int y, int width, int height, uint32 color, char* text, uint32 textcolor, int id)
{
    TextBox newTextBox = {};
    newTextBox.Shape.x = x;
    newTextBox.Shape.y = y;
    newTextBox.Shape.width = width;
    newTextBox.Shape.height = height;
    newTextBox.Shape.color = color;
    
    newTextBox.Text = text;
    newTextBox.TextColor = textcolor;
    newTextBox.ID = id;
    
    GUIComponent newComponent = {};
    newComponent.component = PermanentStorageAssign(&newTextBox, sizeof(TextBox));
    GUIComponent* memnewComponent = (GUIComponent*)PermanentStorageAssign(&newComponent, sizeof(GUIComponent));
    
    if(!addGUIComponent(g->TextBoxes, memnewComponent))
    {
        g->TextBoxes = memnewComponent;
    }
}

internal int
CheckTextBoxes(GUI* G, int32 MouseX, int32 MouseY)
{
    GUIComponent* cursor = G->buttons;
    while(cursor != 0)
    {
        Rect* s = (Rect*)cursor->component;
        if (MouseInRect(s, MouseX, MouseY))
        {
            Button* btn = (Button*)cursor->component;
            return btn->ID;
        }
        
        cursor = cursor->next;
    }
    
    return -1;
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
RenderGUI(game_offscreen_buffer *Buffer, GUI* g)
{
    // Render buttons
    GUIComponent* cursor = g->buttons;
    while(cursor != 0)
    {
        Button* b = (Button*)cursor->component;
        RenderRect(Buffer, &b->Shape, FILL, b->Shape.color);
        PrintOnScreen(Buffer, b->Text, b->Shape.x, b->Shape.y, (float)b->Shape.height, b->TextColor, &b->Shape);
        cursor = cursor->next;
    }
    cursor = 0;
    
    
    // Render TextBox
    cursor = g->TextBoxes;
    while(cursor != 0)
    {
        TextBox* b = (TextBox*)cursor->component;
        RenderRect(Buffer, &b->Shape, FILL, b->Shape.color);
        PrintOnScreen(Buffer, b->Text, b->Shape.x, b->Shape.y, (float)b->Shape.height, b->TextColor, &b->Shape);
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
        if (MouseInRect(&b->Shape, MouseX, MouseY))
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
        if (MouseInRect(s, MouseX, MouseY))
        {
            Button* btn = (Button*)cursor->component;
            return btn->ID;
        }
        
        cursor = cursor->next;
    }
    
    return -1;
}