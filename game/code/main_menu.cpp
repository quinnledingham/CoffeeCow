GUI *MainMenu = &GameState->GUIs[0];

if (MainMenu->Initialized == 0) {
    MainMenu->Padding = 10;
    MainMenu->DefaultPadding = 10;
    MainMenu->Components.Init(100, sizeof(GUIComponent));
    MainMenu->DefaultDim = v2(1000, 1000);
    MainMenu->Initialized = 1;
    
    int Y = 0;
    uint32 RegularColor = 0xFF32a89b;
    uint32 HoverColor = 0xFFeba434;
    uint32 TextColor = 0xFFFFFFFF;
    v2 BtnDim = v2(600, 150);
    
    {
        Text TXT = {};
        TXT.Text = "COFFEE COW";
        TXT.ID = Btn1;
        TXT.FontType = &GameState->Faune100;
        TXT.TextColor = 0xFF000000;
        v2 GridCoords = v2(0, Y++);
        AddText(MainMenu, GridCoords, TXT);
    }{
        Button BTN = {};
        BTN.Text = "Singleplayer";
        BTN.FontType = &GameState->Faune100;
        BTN.ID = GameStart;
        BTN.RegularColor = RegularColor;
        BTN.HoverColor = HoverColor;
        BTN.TextColor = TextColor;
        v2 GridCoords = v2(0, Y++);
        v2 Dim = BtnDim;
        AddButton(MainMenu, GridCoords, Dim, BTN);
    }{
        Button BTN = {};
        BTN.Text = "Multiplayer";
        BTN.FontType = &GameState->Faune100;
        BTN.ID = Multiplayer;
        BTN.RegularColor = RegularColor;
        BTN.HoverColor = HoverColor;
        BTN.TextColor = TextColor;
        v2 GridCoords = v2(0, Y++);
        v2 Dim = BtnDim;
        AddButton(MainMenu, GridCoords, Dim, BTN);
    }{
        Button BTN = {};
        BTN.Text = "Quit";
        BTN.FontType = &GameState->Faune100;
        BTN.ID = Quit;
        BTN.RegularColor = RegularColor;
        BTN.HoverColor = HoverColor;
        BTN.TextColor = TextColor;
        v2 GridCoords = v2(0, Y++);
        v2 Dim = BtnDim;
        AddButton(MainMenu, GridCoords, Dim, BTN);
    }
}

{
    GUIEvents Events = HandleGUIEvents(MainMenu, &p->Input);
    
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
    
    UpdateGUI(MainMenu, v2(p->Dimension.Width, p->Dimension.Height));
    RenderGUI(MainMenu);
    
    BeginMode2D(GameState->C);
    RenderPieceGroup(RenderGroup);
    EndMode2D();
}