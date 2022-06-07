GameOverMenu.Padding = 10;
GameOverMenu.DefaultPadding = 10;

int Y = 0;

Text TXT = {};
Button btn = {};
TextBox tb = {};

TXT.Text = "Game Over";
TXT.ID = Btn1;
TXT.FontType = Faune100;
TXT.TextColor = 0xFF000000;
AddText(&GameOverMenu, 0, Y++,  &TXT);

btn = 
{
    0,
    0,
    "Play Again",    // Text
    Faune100,   // Font
    Reset,       // ID
    0,          // Color (CurrentColor)
    0xFF32a89b, // RegularColor
    0xFFeba434, // HoverColor
    0xFFFFFFFF, // TextColor
};
AddButton(&GameOverMenu, 0, Y++, 600, 150, &btn);

btn = 
{
    0,
    0,
    "Main Menu",    // Text
    Faune100,   // Font
    Menu,       // ID
    0,          // Color (CurrentColor)
    0xFF32a89b, // RegularColor
    0xFFeba434, // HoverColor
    0xFFFFFFFF, // TextColor
};
AddButton(&GameOverMenu, 0, Y++, 600, 150, &btn);

GameOverMenu.ClientWidth = 1000;
GameOverMenu.ClientHeight = 1000;

InitializeGUI(&GameOverMenu);