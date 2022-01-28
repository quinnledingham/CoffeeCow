#include "gui.h"
#include "memorymanager.h"
#include "snake.h"

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
AddCharTextBoxText(NewGUI* G, char* Char)
{
    NewGUIComponent* Cursor = G->TextBoxes;
    while(Cursor != 0)
    {
        NewTextBox* TB = (NewTextBox*)Cursor->Data;
        if (TB->ShowCursor == 1)
        {
            TB->Text = StringConcat(TB->Text, Char);
        }
        
        Cursor = Cursor->Next;
    }
}

internal void
RemoveCharTextBoxText(NewGUI* G)
{
    NewGUIComponent* Cursor = G->TextBoxes;
    while(Cursor != 0)
    {
        NewTextBox* TB = (NewTextBox*)Cursor->Data;
        if (TB->ShowCursor == 1)
        {
            int Size = StringLength(TB->Text);
            char* Result = (char*)PermanentStorageAssign(TB->Text, Size);
            Result[Size - 1] = 0;
            TB->Text = Result;
        }
        
        Cursor = Cursor->Next;
    }
}



internal void
ChangeTextBoxShowCursor(NewGUI* G, int ID)
{
    NewGUIComponent* Cursor = G->TextBoxes;
    while(Cursor != 0)
    {
        NewTextBox* TB = (NewTextBox*)Cursor->Data;
        if (TB->ID == ID)
        {
            TB->ShowCursor = 1;
        }
        else
        {
            TB->ShowCursor = 0;
        }
        
        Cursor = Cursor->Next;
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
CheckButtonsHover(NewGUI* G, int32 MouseX, int32 MouseY)
{
    NewGUIComponent* Cursor = G->Buttons;
    while(Cursor != 0)
    {
        NewButton* B = (NewButton*)Cursor->Data;
        
        Rect R = {};
        R.x = Cursor->X;
        R.y = Cursor->Y;
        R.width = Cursor->Width;
        R.height = Cursor->Height;
        
        if (MouseInRect(&R, MouseX, MouseY))
        {
            B->Color = B->HoverColor;
        }
        else
        {
            B->Color = B->RegularColor;
        }
        
        Cursor = Cursor->Next;
    }
}

// Returns what button was pressed if one was.
internal int
CheckButtonsClick(NewGUI* G, int32 MouseX, int32 MouseY)
{
    NewGUIComponent* Cursor = G->Buttons;
    while(Cursor != 0)
    {
        Rect R = {};
        R.x = Cursor->X;
        R.y = Cursor->Y;
        R.width = Cursor->Width;
        R.height = Cursor->Height;
        
        if (MouseInRect(&R, MouseX, MouseY))
        {
            NewButton* btn = (NewButton*)Cursor->Data;
            return btn->ID;
        }
        
        Cursor = Cursor->Next;
    }
    
    return -1;
}

internal int
CheckTextBoxes(NewGUI* G, int32 MouseX, int32 MouseY)
{
    NewGUIComponent* Cursor = G->TextBoxes;
    while(Cursor != 0)
    {
        Rect R = {};
        R.x = Cursor->X;
        R.y = Cursor->Y;
        R.width = Cursor->Width;
        R.height = Cursor->Height;
        
        NewTextBox* btn = (NewTextBox*)Cursor->Data;
        btn->ShowCursor = 0;
        if (MouseInRect(&R, MouseX, MouseY))
        {
            return btn->ID;
        }
        
        Cursor = Cursor->Next;
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


internal int
StringLength(char* String)
{
    if(String == 0)
        return 0;
    
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

internal int
AddNewGUIComponentAll(NewGUIComponent* Head, NewGUIComponent* NewComponent)
{
    // Start linked list
    if(Head == 0)
    {
        return 0;
    }
    
    NewGUIComponent* Cursor = Head;
    while(Cursor->All != 0)
    {
        Cursor = Cursor->All;
    }
    
    Cursor->All = NewComponent;
    return 1;
}

internal void
AddNewButton(NewGUI* G, int GridX, int GridY, int Width, int Height, NewButton* B)
{
    B->Color = B->RegularColor;
    NewGUIComponent NewTemp = {};
    NewGUIComponent* NewComponent = (NewGUIComponent*)PermanentStorageAssign((void*)&NewTemp, sizeof(NewGUIComponent));
    
    NewComponent->GridX = GridX;
    NewComponent->GridY = GridY;
    NewComponent->Width = Width;
    NewComponent->Height = Height;
    NewComponent->WidthP = Width + (G->Padding * 2);
    NewComponent->HeightP = Height + (G->Padding * 2);
    NewComponent->Data = PermanentStorageAssign((void*)B, sizeof(NewButton));
    
    // Start linked list
    if(G->Buttons == 0)
    {
        G->Buttons = NewComponent;
    }
    else 
    {
        NewGUIComponent* Cursor = G->Buttons;
        while(Cursor->Next != 0)
        {
            Cursor = Cursor->Next;
        }
        Cursor->Next = NewComponent;
    }
    
    if (!AddNewGUIComponentAll(G->All, NewComponent))
    {
        G->All = NewComponent;
    }
}

internal void
AddNewText(NewGUI* G, int GridX, int GridY,  NewText* T)
{
    NewGUIComponent NewTemp = {};
    NewGUIComponent* NewComponent = (NewGUIComponent*)PermanentStorageAssign((void*)&NewTemp, sizeof(NewGUIComponent));
    
    v2 StringDimension = GetStringDimensions(T->FontType, T->Text);
    
    NewComponent->GridX = GridX;
    NewComponent->GridY = GridY;
    NewComponent->Width = (int)StringDimension.x;
    NewComponent->Height = (int)StringDimension.y;
    NewComponent->WidthP = (int)StringDimension.x + (G->Padding * 2);
    NewComponent->HeightP = (int)StringDimension.y + (G->Padding * 2);
    NewComponent->Data = PermanentStorageAssign((void*)T, sizeof(NewText));
    
    
    // Start linked list
    if(G->Texts == 0)
    {
        G->Texts = NewComponent;
    }
    else 
    {
        NewGUIComponent* Cursor = G->Texts;
        while(Cursor->Next != 0)
        {
            Cursor = Cursor->Next;
        }
        Cursor->Next = NewComponent;
    }
    
    if (!AddNewGUIComponentAll(G->All, NewComponent))
    {
        G->All = NewComponent;
    }
}

internal void
AddNewTextBox(NewGUI* G, int GridX, int GridY, int Width, int Height, NewTextBox* TB)
{
    NewGUIComponent NewTemp = {};
    NewGUIComponent* NewComponent = (NewGUIComponent*)PermanentStorageAssign((void*)&NewTemp, sizeof(NewGUIComponent));
    
    
    NewComponent->GridX = GridX;
    NewComponent->GridY = GridY;
    NewComponent->Width = Width;
    NewComponent->Height = Height;
    NewComponent->WidthP = Width + (G->Padding * 2);
    NewComponent->HeightP = Height + (G->Padding * 2);
    NewComponent->Data = PermanentStorageAssign((void*)TB, sizeof(NewTextBox));
    
    
    // Start linked list
    if(G->TextBoxes == 0)
    {
        G->TextBoxes = NewComponent;
    }
    else 
    {
        NewGUIComponent* Cursor = G->TextBoxes;
        while(Cursor->Next != 0)
        {
            Cursor = Cursor->Next;
        }
        Cursor->Next = NewComponent;
    }
    
    if (!AddNewGUIComponentAll(G->All, NewComponent))
    {
        G->All = NewComponent;
    }
}

internal void
UpdateRow(Row* R, int Width, int Height, int GridX)
{
    R->Columns[GridX].Width = Width;
    R->Width += Width;
    if (R->Height < Height)
        R->Height = Height;
}

internal void
InitializeNewGUI(NewGUI* G)
{
    NewGUIComponent* Cursor = G->All;
    while(Cursor != 0)
    {
        UpdateRow(&G->Rows[Cursor->GridY], Cursor->WidthP, Cursor->HeightP, Cursor->GridX);
        Cursor = Cursor->All;
    }
    
    for (int i = 0; i < 10; i++)
    {
        G->Height += G->Rows[i].Height;
    }
    
}

internal void
UpdateNewGUI(NewGUI* G, int BufferWidth, int BufferHeight)
{
    
    NewGUIComponent* Cursor = G->All;
    while(Cursor != 0)
    {
        Row* R = &G->Rows[Cursor->GridY];
        
        Cursor->X = ((BufferWidth - R->Width)/2);
        for (int  i = 0; i < Cursor->GridX; i++)
        {
            Column* tempC = &G->Rows[Cursor->GridY].Columns[i];
            Cursor->X += tempC->Width;
        }
        
        Cursor->Y = ((BufferHeight - G->Height)/2) + ((R->Height - Cursor->Height)/2);
        for (int i = 0; i < Cursor->GridY; i++)
        {
            Row* tempR = &G->Rows[i];
            Cursor->Y += tempR->Height;
        }
        
        Cursor = Cursor->All;
    }
}

internal void
RenderNewGUI(game_offscreen_buffer *Buffer, NewGUI* G)
{
    
    // Render buttons
    NewGUIComponent* Cursor = G->Buttons;
    while(Cursor != 0)
    {
        NewButton* b = (NewButton*)Cursor->Data;
        
        Rect R = {};
        R.x = Cursor->X;
        R.y = Cursor->Y;
        R.width = Cursor->Width;
        R.height = Cursor->Height;
        
        RenderRect(Buffer, &R, FILL, b->Color);
        PrintOnScreen(Buffer, b->FontType,  b->Text, Cursor->X, Cursor->Y, b->TextColor, &R);
        Cursor = Cursor->Next;
    }
    
    Cursor = 0;
    
    // Render TextBox
    Cursor = G->TextBoxes;
    while(Cursor != 0)
    {
        NewTextBox* b = (NewTextBox*)Cursor->Data;
        
        Rect R = {};
        R.x = Cursor->X;
        R.y = Cursor->Y;
        R.width = Cursor->Width;
        R.height = Cursor->Height;
        
        RenderRect(Buffer, &R, FILL, b->Color);
        PrintOnScreenReturn EndOfText = PrintOnScreen(Buffer, b->FontType, b->Text, Cursor->X, Cursor->Y, b->TextColor, &R);
        if (b->ShowCursor == 1)
        {
            v2 p1 = {EndOfText.Top.x, EndOfText.Top.y};
            v2 p2 = {EndOfText.Top.x, EndOfText.Top.y + EndOfText.Height};
            //RenderLine(Buffer,p1, p2);
        }
        
        Cursor = Cursor->Next;
    }
    
    
    Cursor = G->Texts;
    while(Cursor != 0)
    {
        NewText* b = (NewText*)Cursor->Data;
        
        Rect R = {};
        R.x = Cursor->X;
        R.y = Cursor->Y;
        R.width = Cursor->Width;
        R.height = Cursor->Height;
        
        //RenderRect(Buffer, &R, FILL, b->Color);
        PrintOnScreenReturn Re = PrintOnScreen(Buffer, b->FontType, b->Text, Cursor->X, Cursor->Y, b->TextColor, &R);
        //b->Height = Re.Height;
        
        Cursor = Cursor->Next;
    }
    Cursor = 0;
}