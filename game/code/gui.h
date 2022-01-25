#ifndef GUI_H
#define GUI_H

struct GUIComponent
{
    GUIComponent* next;
    void* component;
};

struct GUI
{
    int initialized;
    GUIComponent* buttons;
    GUIComponent* Texts;
    GUIComponent* TextBoxes;
};

enum MenuButtons
{
    GameStart,
    Quit,
    Join
};
struct Button
{
    int X;
    int Y;
    int Width;
    int Height;
    char* Text;
    int ID;
    
    uint32 Color;
    uint32 RegularColor;
    uint32 HoverColor;
    uint32 TextColor;
};


enum TextBoxID
{
    IP
};
struct TextBox
{
    int X;
    int Y;
    int Width;
    int Height;
    char* Text;
    int ID;
    int ShowCursor;
    
    uint32 Color;
    uint32 TextColor;
};


struct Text
{
    int X;
    int Y;
    int Width;
    int Height;
    char* Text;
    uint32 TextColor;
};


#endif //GUI_H
