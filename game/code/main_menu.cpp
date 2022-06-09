global_variable GUI MainMenu = {};

if (MainMenu.Initialized == 0) {
    MainMenu.Padding = 10;
    MainMenu.DefaultPadding = 10;
    
    int Y = 0;
    
    Text TXT = {};
    Button btn = {};
    TextBox tb = {};
    
    TXT.Text = "SINGLEPLAYER";
    TXT.ID = Btn1;
    TXT.FontType = Faune100;
    TXT.TextColor = 0xFF000000;
    AddText(&MainMenu, 0, Y++,  &TXT);
    
    btn = 
    {
        0,
        0,
        "START",    // Text
        Faune100,   // Font
        GameStart,       // ID
        0,          // Color (CurrentColor)
        0xFF32a89b, // RegularColor
        0xFFeba434, // HoverColor
        0xFFFFFFFF, // TextColor
    };
    AddButton(&MainMenu, 0, Y++, 300, 100, &btn);
    
    TXT = 
    {
        "0",    // Text
        Btn1,       // ID
        Faune100,   // Font
        0xFFFFFFFF, // TextColor
    };
    AddText(&MainMenu, 0, Y++,  &TXT);
    
    TXT = 
    {
        "MULTIPLAYER",    // Text
        Btn1,       // ID
        Faune100,   // Font
        0xFF000000, // TextColor
    };
    AddText(&MainMenu, 0, Y++,  &TXT);
    
    TXT = 
    {
        "IP:",    // Text
        Btn1,       // ID
        Faune50,   // Font
        0xFF000000, // TextColor
    };
    AddText(&MainMenu, 0, Y, &TXT);
    
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
    AddTextBox(&MainMenu, 1, Y++, 500, 50, &tb);
    
    TXT = 
    {
        "PORT:",    // Text
        Btn1,       // ID
        Faune50,   // Font
        0xFF000000, // TextColor
    };
    AddText(&MainMenu, 0, Y,  &TXT);
    
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
    AddTextBox(&MainMenu, 1, Y++, 500, 50, &tb);
    
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
    AddButton(&MainMenu, 0, Y++, 250, 100, &btn);
    
    btn = 
    {
        0,
        0,
        "QUIT",    // Text
        Faune100,   // Font
        Quit,       // ID
        0,          // Color (CurrentColor)
        0xFF32a89b, // RegularColor
        0xFFeba434, // HoverColor
        0xFFFFFFFF, // TextColor
    };
    AddButton(&MainMenu, 0, Y++, 250, 100, &btn);
    
    //MainMenu.ClientWidth = p->Dimension.Width;
    //MainMenu.ClientHeight = p->Dimension.Height;
    MainMenu.ClientWidth = 1000;
    MainMenu.ClientHeight = 1000;
    InitializeGUI(&MainMenu);
    
    MainMenu.Initialized = 1;
}

{
    GUIEvents Events = HandleGUIEvents(&MainMenu, &p->Input);
    
    if (Events.BtnPressID == GameStart) {
        SetCursorMode(&p->Input, Arrow);
        GameState->Menu = menu::game;
        return;
    }
    else if (Events.BtnPressID == Btn4) {
        //createClient(&client, GetTextBoxText(&MainMenu, IP), GetTextBoxText(&MainMenu, PORT), TCP);
    }
    else if (Events.BtnPressID == Quit)
        p->Input.Quit = 1;
    
    BeginMode2D(C);
    UpdateGUI(&MainMenu, p->Dimension.Width, p->Dimension.Height);
    RenderGUI(&MainMenu);
    EndMode2D();
}