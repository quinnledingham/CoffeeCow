GUI *MultiplayerMenu = &GameState->GUIs[1];

if (MultiplayerMenu->Initialized == 0) {
    MultiplayerMenu->Padding = 10;
    MultiplayerMenu->DefaultPadding = 10;
    MultiplayerMenu->Components.Init(100, sizeof(GUIComponent));
    MultiplayerMenu->DefaultDim = v2(1000, 1000);
    MultiplayerMenu->Initialized = 1;
    
    int Y = 0;
    
    uint32 RegularColor = 0xFF000000;
    uint32 HoverColor = 0xFFDEC9B3;
    uint32 RegularTextColor = 0xFFFFFFFF;
    uint32 HoverTextColor = 0xFF000000;
    
    uint32 TbColor = 0xFFFFFFFF;
    uint32 TbTextColor = 0xFF000000;
    
    v2 TbDim = v2(400, 50);
    v2 BtnDim = v2(250, 75);
    
    GUIComponent *T1;
    {
        Text TXT = {};
        TXT.Text = "PORT:";
        TXT.ID = Btn2;
        TXT.FontType = GetFont(&GameState->Assets, FI_Faune50);
        TXT.TextColor = 0xFFFFFFFF;
        v2 GridCoords = v2(0, Y + 1);
        T1 = AddText(MultiplayerMenu, GridCoords, TXT);
    }{
        Text TXT = {};
        TXT.Text = "IP:";
        TXT.ID = Btn1;
        TXT.FontType = GetFont(&GameState->Assets, FI_Faune50);
        TXT.TextColor = 0xFFFFFFFF;
        v2 GridCoords = v2(0, Y);
        AddText(MultiplayerMenu, GridCoords, TXT, T1);
    }{
        TextBox TB = {};
        TB.Text = "";
        TB.FontType = GetFont(&GameState->Assets, FI_Faune50);
        TB.ID = IP;
        TB.Color = TbColor;
        TB.TextColor = TbTextColor;
        v2 GridCoords = v2(1, Y);
        v2 Dim = TbDim;
        AddTextBox(MultiplayerMenu, GridCoords, Dim, TB, 0);
    }{
        TextBox TB = {};
        TB.Text = "";
        TB.FontType = GetFont(&GameState->Assets, FI_Faune50);
        TB.ID = PORT;
        TB.Color = TbColor;
        TB.TextColor = TbTextColor;
        v2 GridCoords = v2(1, Y + 1);
        v2 Dim = TbDim;
        AddTextBox(MultiplayerMenu, GridCoords, Dim, TB, 0);
    }
    Y += 2;
    {
        Button BTN = {};
        BTN.Text = "Join";
        BTN.FontType = GetFont(&GameState->Assets, FI_Faune50);
        BTN.ID = Btn4;
        BTN.RegularColor = RegularColor;
        BTN.HoverColor = HoverColor;
        BTN.RegularTextColor = RegularTextColor;
        BTN.HoverTextColor = HoverTextColor;
        v2 GridCoords = v2(0, Y++);
        v2 Dim = BtnDim;
        AddButton(MultiplayerMenu, GridCoords, Dim, BTN);
    }{
        Button BTN = {};
        BTN.Text = "Back";
        BTN.FontType = GetFont(&GameState->Assets, FI_Faune50);
        BTN.ID = Btn2;
        BTN.RegularColor = RegularColor;
        BTN.HoverColor = HoverColor;
        BTN.RegularTextColor = RegularTextColor;
        BTN.HoverTextColor = HoverTextColor;
        v2 GridCoords = v2(0, Y++);
        v2 Dim = BtnDim;
        AddButton(MultiplayerMenu, GridCoords, Dim, BTN);
    }
}

{
    GUIEvents *Events = HandleGUIEvents(MultiplayerMenu, &p->Input);
    
    if (Events->BtnPressID == Btn2) {
        SetCursorMode(&p->Input, Arrow);
        GameState->Mode = game_mode::not_in_game;
        GameState->Menu = menu::main_menu;
    }
    else if (Events->BtnPressID == Btn4) {
        SetCursorMode(&p->Input, Arrow);
        GameState->ResetGame = true;
        GameState->Mode = game_mode::multiplayer;
        GameState->Menu = menu::not_in_menu;
        GameState->IP = GetTextBoxText(MultiplayerMenu, IP);
        GameState->Port = GetTextBoxText(MultiplayerMenu, PORT);
        
    }
    
    UpdateGUI(MultiplayerMenu, v2(p->Dimension.Width, p->Dimension.Height));
    RenderGUI(MultiplayerMenu);
    
    DrawBackground(GetTexture(&GameState->Assets, GAI_MainMenuBack), GetTopLeftCornerCoords(p), GetDim(p), 0.0f);
    
    BeginMode2D(GameState->C);
    RenderPieceGroup(RenderGroup);
    EndMode2D();
}