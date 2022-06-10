global_variable GUI MainMenu = {};

if (MainMenu.Initialized == 0) {
    MainMenu.Padding = 10;
    MainMenu.DefaultPadding = 10;
    
    int Y = 0;
    
    Text TXT = {};
    Button btn = {};
    TextBox tb = {};
    
    TXT.Text = "COFFEE COW";
    TXT.ID = Btn1;
    TXT.FontType = Faune100;
    TXT.TextColor = 0xFF000000;
    AddText(&MainMenu, 0, Y++,  &TXT);
    
    btn = 
    {
        0,
        0,
        "Singleplayer",    // Text
        Faune100,   // Font
        GameStart,       // ID
        0,          // Color (CurrentColor)
        0xFF32a89b, // RegularColor
        0xFFeba434, // HoverColor
        0xFFFFFFFF, // TextColor
    };
    AddButton(&MainMenu, 0, Y++, 600, 150, &btn);
    
    btn = 
    {
        0,
        0,
        "Multiplayer",    // Text
        Faune100,   // Font
        Multiplayer,       // ID
        0,          // Color (CurrentColor)
        0xFF32a89b, // RegularColor
        0xFFeba434, // HoverColor
        0xFFFFFFFF, // TextColor
    };
    AddButton(&MainMenu, 0, Y++, 600, 150, &btn);
    
    btn = 
    {
        0,
        0,
        "Quit",    // Text
        Faune100,   // Font
        Quit,       // ID
        0,          // Color (CurrentColor)
        0xFF32a89b, // RegularColor
        0xFFeba434, // HoverColor
        0xFFFFFFFF, // TextColor
    };
    AddButton(&MainMenu, 0, Y++, 600, 150, &btn);
    
    MainMenu.DefaultWidth = 1000;
    MainMenu.DefaultHeight = 1000;
    InitializeGUI(&MainMenu);
    
    MainMenu.Initialized = 1;
}

{
    GUIEvents Events = HandleGUIEvents(&MainMenu, &p->Input);
    
    if (Events.BtnPressID == GameStart) {
        SetCursorMode(&p->Input, Arrow);
        GameState->Menu = menu::game;
    }
    else if (Events.BtnPressID == Multiplayer) {
        SetCursorMode(&p->Input, Arrow);
        GameState->Menu = menu::multiplayer_menu;
    }
    else if (Events.BtnPressID == Btn4) {
        //createClient(&client, GetTextBoxText(&MainMenu, IP), GetTextBoxText(&MainMenu, PORT), TCP);
    }
    else if (Events.BtnPressID == Quit)
        p->Input.Quit = 1;
    
    UpdateGUI(&MainMenu, p->Dimension.Width, p->Dimension.Height);
    RenderGUI(&MainMenu);
    
    BeginMode2D(C);
    RenderPieceGroup(RenderGroup);
    EndMode2D();
}