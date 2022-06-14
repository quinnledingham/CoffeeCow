global_variable GUI GameOverMenu = {};

if (GameOverMenu.Initialized == 0)
{
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
        "Main Menu",    // Text
        Faune100,   // Font
        Menu,       // ID
        0,          // Color (CurrentColor)
        0xFF32a89b, // RegularColor
        0xFFeba434, // HoverColor
        0xFFFFFFFF, // TextColor
    };
    AddButton(&GameOverMenu, 0, Y++, 600, 150, &btn);
    
    GameOverMenu.DefaultDim = v2(1000, 1000);
    InitializeGUI(&GameOverMenu);
    
    GameOverMenu.Initialized = 1;
}

{
    GUIEvents Events = HandleGUIEvents(&GameOverMenu, &p->Input);
    if (Events.BtnPressID == Menu) {
        SetCursorMode(&p->Input, Arrow);
        memset(&GameState->Player1, 0, sizeof(CoffeeCow));
        GameState->Menu = menu::main_menu;
    }
    else if (Events.BtnPressID == Reset) {
        GameState->Menu = menu::game;
        InitializeGame(GameMode::Singleplayer, GameState);
    }
    
    platform_keyboard_input *Keyboard = &p->Input.Keyboard;
    if (Keyboard->Escape.NewEndedDown)
        GameState->Menu = menu::game;
    
    UpdateGUI(&GameOverMenu, p->Dimension.Width, p->Dimension.Height);
    RenderGUI(&GameOverMenu);
    
    BeginMode2D(C);
    RenderPieceGroup(RenderGroup);
    EndMode2D();
}