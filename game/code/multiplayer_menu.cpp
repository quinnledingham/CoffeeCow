GUI *MultiplayerMenu = &GameState->GUIs[1];

if (MultiplayerMenu->Initialized == 0) {
    MultiplayerMenu->Padding = 10;
    MultiplayerMenu->DefaultPadding = 10;
    MultiplayerMenu->Components.Init(100, sizeof(GUIComponent));
    MultiplayerMenu->DefaultDim = v2(1000, 1000);
    MultiplayerMenu->Initialized = 1;
    
    int Y = 0;
    
    uint32 RegularColor = 0xFF32a89b;
    uint32 HoverColor = 0xFFeba434;
    uint32 TextColor = 0xFFFFFFFF;
    uint32 TbColor = 0xFFb3b3b3;
    uint32 TbTextColor = 0xFF000000;
    
    {
        Text TXT = {};
        TXT.Text = "IP:";
        TXT.ID = Btn1;
        TXT.FontType = &GameState->Faune50;
        TXT.TextColor = 0xFF000000;
        v2 GridCoords = v2(0, Y);
        AddText(MultiplayerMenu, GridCoords, TXT);
    }{
        TextBox TB = {};
        TB.Text = "";
        TB.FontType = &GameState->Faune50;
        TB.ID = IP;
        TB.Color = TbColor;
        TB.TextColor = 0xFF000000;
        v2 GridCoords = v2(1, Y++);
        v2 Dim = v2(500, 50);
        AddTextBox(MultiplayerMenu, GridCoords, Dim, TB);
    }{
        Text TXT = {};
        TXT.Text = "PORT:";
        TXT.ID = Btn1;
        TXT.FontType = &GameState->Faune50;
        TXT.TextColor = TbTextColor;
        v2 GridCoords = v2(0, Y);
        AddText(MultiplayerMenu, GridCoords, TXT);
    }{
        TextBox TB = {};
        TB.Text = "";
        TB.FontType = &GameState->Faune50;
        TB.ID = PORT;
        TB.Color = TbColor;
        TB.TextColor = TbTextColor;
        v2 GridCoords = v2(1, Y++);
        v2 Dim = v2(500, 50);
        AddTextBox(MultiplayerMenu, GridCoords, Dim, TB);
    }{
        Button BTN = {};
        BTN.Text = "Join";
        BTN.FontType = &GameState->Faune100;
        BTN.ID = Btn4;
        BTN.RegularColor = RegularColor;
        BTN.HoverColor = HoverColor;
        BTN.TextColor = TextColor;
        v2 GridCoords = v2(0, Y++);
        v2 Dim = v2(250, 100);
        AddButton(MultiplayerMenu, GridCoords, Dim, BTN);
    }{
        Button BTN = {};
        BTN.Text = "Back";
        BTN.FontType = &GameState->Faune100;
        BTN.ID = Btn2;
        BTN.RegularColor = RegularColor;
        BTN.HoverColor = HoverColor;
        BTN.TextColor = TextColor;
        v2 GridCoords = v2(0, Y++);
        v2 Dim = v2(250, 100);
        AddButton(MultiplayerMenu, GridCoords, Dim, BTN);
    }
}

{
    GUIEvents Events = HandleGUIEvents(MultiplayerMenu, &p->Input);
    
    if (Events.BtnPressID == Btn2) {
        SetCursorMode(&p->Input, Arrow);
        GameState->Menu = menu::main_menu;
    }
    
    UpdateGUI(MultiplayerMenu, v2(p->Dimension.Width, p->Dimension.Height));
    RenderGUI(MultiplayerMenu);
    
    BeginMode2D(GameState->C);
    RenderPieceGroup(RenderGroup);
    EndMode2D();
}