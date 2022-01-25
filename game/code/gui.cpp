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

internal int
StringLength(char* String)
{
    int i = 0;
    while(String[i] != 0)
    {
        i++;
    }
    
    return i;
}

internal char*
StringConcat(char* Source, char* Add)
{
    int Size = StringLength(Source) + StringLength(Add) + 1;
    char* Result = (char*)PermanentStorageAssign(Source, Size);
    
    int i = StringLength(Source);
    int j = 0;
    while(Add[j] != 0)
    {
        Result[i] = Add[j];
        i++;
        j++;
    }
    Result[i] = 0;
    return Result;
}


internal void
AddCharTextBoxText(GUI* G, char* Char)
{
    GUIComponent* cursor = G->TextBoxes;
    while(cursor != 0)
    {
        TextBox* TB = (TextBox*)cursor->component;
        if (TB->ShowCursor == 1)
        {
            TB->Text = StringConcat(TB->Text, Char);
        }
        
        cursor = cursor->next;
    }
}

internal void
RemoveCharTextBoxText(GUI* G)
{
    GUIComponent* cursor = G->TextBoxes;
    while(cursor != 0)
    {
        TextBox* TB = (TextBox*)cursor->component;
        if (TB->ShowCursor == 1)
        {
            int Size = StringLength(TB->Text);
            char* Result = (char*)PermanentStorageAssign(TB->Text, Size);
            Result[Size - 1] = 0;
            TB->Text = Result;
        }
        
        cursor = cursor->next;
    }
}

internal void
ChangeTextBoxShowCursor(GUI* G, int ID)
{
    GUIComponent* cursor = G->TextBoxes;
    while(cursor != 0)
    {
        TextBox* TB = (TextBox*)cursor->component;
        if (TB->ID == ID)
        {
            TB->ShowCursor = 1;
        }
        else
        {
            TB->ShowCursor = 0;
        }
        
        cursor = cursor->next;
    }
}

internal void
AddTextBox(GUI* g, TextBox* Source)
{
    GUIComponent newComponent = {};
    newComponent.component = PermanentStorageAssign(Source, sizeof(TextBox));
    GUIComponent* memnewComponent = (GUIComponent*)PermanentStorageAssign(&newComponent, sizeof(GUIComponent));
    
    if(!addGUIComponent(g->TextBoxes, memnewComponent))
    {
        g->TextBoxes = memnewComponent;
    }
}

internal int
CheckTextBoxes(GUI* G, int32 MouseX, int32 MouseY)
{
    GUIComponent* cursor = G->TextBoxes;
    while(cursor != 0)
    {
        Rect* s = (Rect*)cursor->component;
        TextBox* btn = (TextBox*)cursor->component;
        btn->ShowCursor = 0;
        if (MouseInRect(s, MouseX, MouseY))
        {
            return btn->ID;
        }
        
        cursor = cursor->next;
    }
    
    return -1;
}

internal void
addButton(GUI* g, Button* Source)
{
    GUIComponent newComponent = {};
    newComponent.component = PermanentStorageAssign(Source, sizeof(Button));
    GUIComponent* memnewComponent = (GUIComponent*)PermanentStorageAssign(&newComponent, sizeof(GUIComponent));
    
    if(!addGUIComponent(g->buttons, memnewComponent))
    {
        g->buttons = memnewComponent;
    }
}

internal void
RenderLine(game_offscreen_buffer *Buffer, v2 Point1, v2 Point2)
{
    real32 Slope = (Point2.y - Point1.y) / (Point2.x - Point1.x);
    real32 B = Point1.y - (Slope * Point1.x);
    
    for (int X = (int)Point1.x; X <= Point2.x; X++)
    {
        uint8 *Pixel = ((uint8 *)Buffer->Memory + X * Buffer->BytesPerPixel + (int)Point1.y*Buffer->Pitch);
        
        for (int Y = (int)Point1.y; Y <= Point2.y; Y++)
        {
            
            *(uint32 *)Pixel = 0xFF000000;
            
            Pixel += Buffer->Pitch;
        }
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
        
        Rect R = {};
        R.x = b->X;
        R.y = b->Y;
        R.width = b->Width;
        R.height = b->Height;
        
        RenderRect(Buffer, &R, FILL, b->Color);
        PrintOnScreen(Buffer, b->FontType,  b->Text, b->X, b->Y, b->TextColor, &R);
        //PrintOnScreen(Buffer, b->Text, b->X, b->Y, (float)b->Height, b->TextColor, &R);
        cursor = cursor->next;
    }
    cursor = 0;
    
    // Render TextBox
    cursor = g->TextBoxes;
    while(cursor != 0)
    {
        TextBox* b = (TextBox*)cursor->component;
        
        Rect R = {};
        R.x = b->X;
        R.y = b->Y;
        R.width = b->Width;
        R.height = b->Height;
        
        RenderRect(Buffer, &R, FILL, b->Color);
        Cursor EndOfText = PrintOnScreen(Buffer, b->FontType, b->Text, b->X, b->Y, b->TextColor, &R);
        if (b->ShowCursor == 1)
        {
            v2 p1 = {EndOfText.Top.x, EndOfText.Top.y};
            v2 p2 = {EndOfText.Top.x, EndOfText.Top.y + EndOfText.Height};
            RenderLine(Buffer,p1, p2);
        }
        
        cursor = cursor->next;
    }
    
    cursor = g->Texts;
    while(cursor != 0)
    {
        Text* b = (Text*)cursor->component;
        
        Rect R = {};
        R.x = b->X;
        R.y = b->Y;
        R.width = b->Width;
        R.height = b->Height;
        
        //RenderRect(Buffer, &R, FILL, b->Color);
        PrintOnScreen(Buffer, b->FontType, b->Text, b->X, b->Y, b->TextColor, &R);
        
        cursor = cursor->next;
    }
    cursor = 0;
}

internal void
CheckButtonsHover(GUI* g, int32 MouseX, int32 MouseY)
{
    GUIComponent* cursor = g->buttons;
    while(cursor != 0)
    {
        Button* b = (Button*)cursor->component;
        
        Rect R = {};
        R.x = b->X;
        R.y = b->Y;
        R.width = b->Width;
        R.height = b->Height;
        
        if (MouseInRect(&R, MouseX, MouseY))
        {
            b->Color = b->HoverColor;
        }
        else
        {
            b->Color = b->RegularColor;
        }
        
        cursor = cursor->next;
    }
}

// Returns what button was pressed if one was.
internal int
CheckButtonsClick(GUI* g, int32 MouseX, int32 MouseY)
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

internal bool
CompareStrings(char *c1, char* c2)
{
    int i = 0;
    while(c1[i] != 0 && c2[i] != 0)
    {
        if (c1[i] != c2[i])
        {
            return false;
        }
        
        i++;
    }
    
    return true;
}

internal void
AddText(GUI* g, Text* Source)
{
    GUIComponent newComponent = {};
    newComponent.component = PermanentStorageAssign(Source, sizeof(Text));
    GUIComponent* memnewComponent = (GUIComponent*)PermanentStorageAssign(&newComponent, sizeof(GUIComponent));
    
    if(!addGUIComponent(g->Texts, memnewComponent))
    {
        g->Texts = memnewComponent;
    }
}
