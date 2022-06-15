GUI *PauseMenu = &GameState->GUIs[2];

if (PauseMenu->Initialized == 0)
{
    PauseMenu->Padding = 10;
    PauseMenu->DefaultPadding = 10;
    PauseMenu->Components.Init(100, sizeof(GUIComponent));
    PauseMenu->DefaultDim = v2(1000, 1000);
    PauseMenu->Initialized = 1;
    
    int Y = 0;
    
    Text TXT = {};
    Button btn = {};
    TextBox tb = {};
    
    TXT.Text = "Paused";
    TXT.ID = Btn1;
    TXT.FontType = &GameState->Faune100;
    TXT.TextColor = 0xFF000000;
    AddText(PauseMenu, v2(0, Y++), TXT);
    
    btn = 
    {
        "Reset",    // Text
        &GameState->Faune100,   // Font
        Reset,       // ID
        0,          // Color (CurrentColor)
        0xFF32a89b, // RegularColor
        0xFFeba434, // HoverColor
        0xFFFFFFFF, // TextColor
    };
    AddButton(PauseMenu, v2(0, Y++), v2(600, 150), btn);
    
    btn = 
    {
        "Menu",    // Text
        &GameState->Faune100,   // Font
        Menu,       // ID
        0,          // Color (CurrentColor)
        0xFF32a89b, // RegularColor
        0xFFeba434, // HoverColor
        0xFFFFFFFF, // TextColor
    };
    AddButton(PauseMenu, v2(0, Y++), v2(600, 150), btn);
}

{
    GUIEvents Events = HandleGUIEvents(PauseMenu, &p->Input);
    if (Events.BtnPressID == Menu) {
        SetCursorMode(&p->Input, Arrow);
        GameState->Player1.Initialized = false;
        GameState->Menu = menu::main_menu;
    }
    else if (Events.BtnPressID == Reset) {
        GameState->Menu = menu::game;
        InitializeGame(GameMode::Singleplayer, GameState);
    }
    
    platform_keyboard_input *Controller = &p->Input.Keyboard;
    if (Controller->Escape.NewEndedDown)
        GameState->Menu = menu::game;
    
    UpdateGUI(PauseMenu, v2(p->Dimension.Width, p->Dimension.Height));
    RenderGUI(PauseMenu);
    
    BeginMode2D(GameState->C);
    RenderPieceGroup(RenderGroup);
    EndMode2D();
}