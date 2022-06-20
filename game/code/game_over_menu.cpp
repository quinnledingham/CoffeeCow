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
    TXT.FontType = GetFont(&GameState->Assets, FI_Faune100);
    TXT.TextColor = 0xFFFFFFFF;
    AddText(&GameOverMenu, v2(0, Y++),  TXT);
    
    btn = 
    {
        "Play Again",    // Text
        GetFont(&GameState->Assets, FI_Faune100),   // Font
        Reset,       // ID
        0,          // Color (CurrentColor)
        0x96000000, // RegularColor
        0xFF000000, // HoverColor
        0xFFFFFFFF, // TextColor
    };
    AddButton(&GameOverMenu, v2(0, Y++), v2(500, 150), btn);
    
    btn = 
    {
        "Main Menu",    // Text
        GetFont(&GameState->Assets, FI_Faune100),   // Font
        Menu,       // ID
        0,          // Color (CurrentColor)
        0x96000000, // RegularColor
        0xFF000000, // HoverColor
        0xFFFFFFFF, // TextColor
    };
    AddButton(&GameOverMenu, v2(0, Y++), v2(500, 150), btn);
}

{
    GUIEvents *Events = HandleGUIEvents(&GameOverMenu, &p->Input);
    if (Events->BtnPressID == Menu) {
        SetCursorMode(&p->Input, Arrow);
        GameState->Mode = game_mode::not_in_game;
        GameState->Menu = menu::main_menu;
    }
    else if (Events->BtnPressID == Reset) {
        GameState->ResetGame = true;
        GameState->Mode = game_mode::singleplayer;
        GameState->Menu = menu::not_in_menu;
    }
    
    platform_keyboard_input *Keyboard = &p->Input.Keyboard;
    if (Keyboard->Escape.NewEndedDown) {
        GameState->Mode = game_mode::singleplayer;
        GameState->Menu = menu::not_in_menu;
    }
    UpdateGUI(&GameOverMenu, v2(p->Dimension.Width, p->Dimension.Height));
    RenderGUI(&GameOverMenu);
    
    //BeginMode2D(*C);
    //RenderPieceGroup(RenderGroup);
    //EndMode2D();
}