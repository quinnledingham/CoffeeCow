global_variable GUI GameOverMenu = {};

if (GameOverMenu.Initialized == 0)
{
    GameOverMenu.Padding = 10;
    GameOverMenu.DefaultPadding = 10;
    GameOverMenu.Components.Init(100, sizeof(GUIComponent));
    GameOverMenu.DefaultDim = v2(1000, 1000);
    GameOverMenu.Initialized = 1;
    
    int Y = 0;
    
    Text TXT = {};
    Button btn = {};
    TextBox tb = {};
    
    TXT.Text = "Game Over";
    TXT.ID = Btn1;
    TXT.FontType = &GameState->Faune100;
    TXT.TextColor = 0xFF000000;
    AddText(&GameOverMenu, v2(0, Y++),  TXT);
    
    btn = 
    {
        "Play Again",    // Text
        &GameState->Faune100,   // Font
        Reset,       // ID
        0,          // Color (CurrentColor)
        0xFF32a89b, // RegularColor
        0xFFeba434, // HoverColor
        0xFFFFFFFF, // TextColor
    };
    AddButton(&GameOverMenu, v2(0, Y++), v2(600, 150), btn);
    
    btn = 
    {
        "Main Menu",    // Text
        &GameState->Faune100,   // Font
        Menu,       // ID
        0,          // Color (CurrentColor)
        0xFF32a89b, // RegularColor
        0xFFeba434, // HoverColor
        0xFFFFFFFF, // TextColor
    };
    AddButton(&GameOverMenu, v2(0, Y++), v2(600, 150), btn);
}

{
    GUIEvents Events = HandleGUIEvents(&GameOverMenu, &p->Input);
    if (Events.BtnPressID == Menu) {
        SetCursorMode(&p->Input, Arrow);
        GameState->Menu = menu::main_menu;
        GameState->Player1.Initialized = false;
    }
    else if (Events.BtnPressID == Reset) {
        GameState->Menu = menu::game;
        InitializeGame(GameMode::Singleplayer, GameState);
    }
    
    platform_keyboard_input *Keyboard = &p->Input.Keyboard;
    if (Keyboard->Escape.NewEndedDown)
        GameState->Menu = menu::game;
    
    UpdateGUI(&GameOverMenu, v2(p->Dimension.Width, p->Dimension.Height));
    RenderGUI(&GameOverMenu);
    
    BeginMode2D(*C);
    RenderPieceGroup(RenderGroup);
    EndMode2D();
}