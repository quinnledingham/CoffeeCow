GUI *MainMenu = &GameState->GUIs[0];

if (MainMenu->Initialized == 0) {
    MainMenu->Padding = 10;
    MainMenu->DefaultPadding = 10;
    MainMenu->Components.Init(100, sizeof(GUIComponent));
    MainMenu->DefaultDim = v2(1000, 1000);
    MainMenu->Initialized = 1;
    
    int Y = 0;
    uint32 RegularColor = 0xFF000000;
    uint32 HoverColor = 0xFFDEC9B3;
    uint32 RegularTextColor = 0xFFFFFFFF;
    uint32 HoverTextColor = 0xFF000000;
    v2 BtnDim = v2(550, 75);
    Font *BtnFont = GetFont(&GameState->Assets, FI_Faune50);
    
    {
        gui_logo LOGO = {};
        LOGO.Tex = GetTexture(&GameState->Assets, GAI_Miz);
        v2 Dim = v2(550, 274);
        v2 GridCoords = v2(0, Y++);
        GUIAddLogo(MainMenu, GridCoords, Dim, LOGO);
    }{
        Button BTN = {};
        BTN.Text = "Singleplayer";
        BTN.FontType = BtnFont;
        BTN.ID = GameStart;
        BTN.RegularColor = RegularColor;
        BTN.HoverColor = HoverColor;
        BTN.RegularTextColor = RegularTextColor;
        BTN.HoverTextColor = HoverTextColor;
        v2 GridCoords = v2(0, Y++);
        v2 Dim = BtnDim;
        AddButton(MainMenu, GridCoords, Dim, BTN);
    }{
        Button BTN = {};
        BTN.Text = "Multiplayer";
        BTN.FontType = BtnFont;
        BTN.ID = Multiplayer;
        BTN.RegularColor = RegularColor;
        BTN.HoverColor = HoverColor;
        BTN.RegularTextColor = RegularTextColor;
        BTN.HoverTextColor = HoverTextColor;
        v2 GridCoords = v2(0, Y++);
        v2 Dim = BtnDim;
        AddButton(MainMenu, GridCoords, Dim, BTN);
    }{
        Button BTN = {};
        BTN.Text = "Quit";
        BTN.FontType = BtnFont;
        BTN.ID = Quit;
        BTN.RegularColor = RegularColor;
        BTN.HoverColor = HoverColor;
        BTN.RegularTextColor = RegularTextColor;
        BTN.HoverTextColor = HoverTextColor;
        v2 GridCoords = v2(0, Y++);
        v2 Dim = BtnDim;
        AddButton(MainMenu, GridCoords, Dim, BTN);
    }
    
    //MainMenu->BackgroundColor = 0x96000000;
}

{
    GUIEvents *Events = HandleGUIEvents(MainMenu, &p->Input);
    
    if (Events->BtnPressID == GameStart) {
        SetCursorMode(&p->Input, Arrow);
        GameState->ResetGame = true;
        GameState->Menu = menu::not_in_menu;
        GameState->Mode = game_mode::singleplayer;
    }
    else if (Events->BtnPressID == Multiplayer) {
        SetCursorMode(&p->Input, Arrow);
        GameState->Menu = menu::multiplayer_menu;
    }
    else if (Events->BtnPressID == Btn4) {
        //createClient(&client, GetTextBoxText(&MainMenu, IP), GetTextBoxText(&MainMenu, PORT), TCP);
    }
    else if (Events->BtnPressID == Quit)
        p->Input.Quit = 1;
    
    UpdateGUI(MainMenu, v2(p->Dimension.Width, p->Dimension.Height));
    RenderGUI(MainMenu);
    
    DrawBackground(GetTexture(&GameState->Assets, GAI_MainMenuBack), GetTopLeftCornerCoords(p), GetDim(p), 0.0f);
    
    BeginMode2D(GameState->C);
    RenderPieceGroup(RenderGroup);
    EndMode2D();
}