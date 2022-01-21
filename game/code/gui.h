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
};

enum ButtonPress
{
    GameStart,
    Quit
};

struct Button
{
    Rect rect;
    char* text;
    int id;
};

#endif //GUI_H
