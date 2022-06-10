global_variable GUI MultiplayerMenu = {};

if (MultiplayerMenu.Initialized == 0) {
    MultiplayerMenu.Padding = 10;
    MultiplayerMenu.DefaultPadding = 10;
    
    int Y = 0;
    
    Text TXT = {};
    Button btn = {};
    TextBox tb = {};
    
    TXT = 
    {
        "IP:",    // Text
        Btn1,       // ID
        Faune50,   // Font
        0xFF000000, // TextColor
    };
    AddText(&MultiplayerMenu, 0, Y, &TXT);
    
    tb =
    {
        0,
        0,
        "",
        &Faune50,
        IP,
        0,
        0xFFb3b3b3,
        0xFF000000
    };
    AddTextBox(&MultiplayerMenu, 1, Y++, 500, 50, &tb);
    
    TXT = 
    {
        "PORT:",    // Text
        Btn1,       // ID
        Faune50,   // Font
        0xFF000000, // TextColor
    };
    AddText(&MultiplayerMenu, 0, Y,  &TXT);
    
    tb =
    {
        0,
        0,
        "",
        &Faune50,
        PORT,
        0,
        0xFFb3b3b3,
        0xFF000000
    };
    AddTextBox(&MultiplayerMenu, 1, Y++, 500, 50, &tb);
    
    btn = 
    {
        0,
        0,
        "JOIN",    // Text
        Faune100,   // Font
        Btn4,       // ID
        0,          // Color (CurrentColor)
        0xFF32a89b, // RegularColor
        0xFFeba434, // HoverColor
        0xFFFFFFFF, // TextColor
    };
    AddButton(&MultiplayerMenu, 0, Y++, 250, 100, &btn);
    
    btn = 
    {
        0,
        0,
        "Back",    // Text
        Faune100,   // Font
        Btn2,       // ID
        0,          // Color (CurrentColor)
        0xFF32a89b, // RegularColor
        0xFFeba434, // HoverColor
        0xFFFFFFFF, // TextColor
    };
    AddButton(&MultiplayerMenu, 0, Y++, 250, 100, &btn);
    
    MultiplayerMenu.DefaultWidth = 1000;
    MultiplayerMenu.DefaultHeight = 1000;
    InitializeGUI(&MultiplayerMenu);
    
    MultiplayerMenu.Initialized = 1;
}

{
    GUIEvents Events = HandleGUIEvents(&MultiplayerMenu, &p->Input);
    
    if (Events.BtnPressID == Btn2) {
        SetCursorMode(&p->Input, Arrow);
        GameState->Menu = menu::main_menu;
    }
    
    UpdateGUI(&MultiplayerMenu, p->Dimension.Width, p->Dimension.Height);
    RenderGUI(&MultiplayerMenu);
    
    BeginMode2D(C);
    RenderPieceGroup(RenderGroup);
    EndMode2D();
}