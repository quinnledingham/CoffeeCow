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
    GUIComponent* text;
    GUIComponent* TextBoxes;
};

enum ButtonPress
{
    GameStart,
    Quit,
    Join
};

struct Button
{
    Rect Shape;
    uint32 RegularColor;
    uint32 HoverColor;
    
    char* Text;
    uint32 TextColor;
    int ID;
};


enum TextBoxID
{
    IP
};
struct TextBox
{
    Rect Shape;
    
    char* Text;
    uint32 TextColor;
    int ShowCursor;
    int ID;
};

#endif //GUI_H
