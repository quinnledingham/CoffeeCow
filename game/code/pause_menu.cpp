global_variable GUI PauseMenu = {};

if (PauseMenu.Initialized == 0)
{
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
        "Reset",    // Text
        Faune100,   // Font
        Reset,       // ID
        0,          // Color (CurrentColor)
        0xFF32a89b, // RegularColor
        0xFFeba434, // HoverColor
        0xFFFFFFFF, // TextColor
    };
    AddButton(&PauseMenu, 0, Y++, 300, 100, &btn);
    
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
    
    PauseMenu.DefaultWidth = 1000;
    PauseMenu.DefaultHeight = 1000;
    
    InitializeGUI(&PauseMenu);
    
    PauseMenu.Initialized = 1;
}

{
    GUIEvents Events = HandleGUIEvents(&PauseMenu, &p->Input);
    if (Events.BtnPressID == Menu) {
        SetCursorMode(&p->Input, Arrow);
        memset(&GameState->Player1, 0, sizeof(CoffeeCow));
        GameState->Menu = menu::main_menu;
        return;
    }
    else if (Events.BtnPressID == Reset) {
        GameState->Menu = menu::game;
        InitializeGame(GameMode::Singleplayer, GameState);
    }
    
    platform_controller_input *Controller = &p->Input.Controllers[0];
    if (Controller->Escape.NewEndedDown)
        GameState->Menu = menu::game;
    
    UpdateGUI(&PauseMenu, p->Dimension.Width, p->Dimension.Height);
    RenderGUI(&PauseMenu);
    
    BeginMode2D(C);
    RenderPieceGroup(RenderGroup);
    EndMode2D();
}