PauseMenu.Padding = 10;
PauseMenu.DefaultPadding = 10;

int Y = 0;

Text TXT = {};
Button btn = {};
TextBox tb = {};

TXT.Text = "Paused";
TXT.ID = Btn1;
TXT.FontType = Faune100;
TXT.TextColor = 0xFF000000;
AddText(&PauseMenu, 0, Y++,  &TXT);

btn = 
{
    0,
    0,
    "Menu",    // Text
    Faune100,   // Font
    Menu,       // ID
    0,          // Color (CurrentColor)
    0xFF32a89b, // RegularColor
    0xFFeba434, // HoverColor
    0xFFFFFFFF, // TextColor
};
AddButton(&PauseMenu, 0, Y++, 300, 100, &btn);

PauseMenu.ClientWidth = p->Dimension.Width;
PauseMenu.ClientHeight = p->Dimension.Height;

InitializeGUI(&PauseMenu);