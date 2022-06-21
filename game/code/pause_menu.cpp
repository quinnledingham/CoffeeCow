internal void RenderPauseMenu(GUI *PauseMenu, platform *p, game_state *GameState)
{
    if (PauseMenu->Initialized == 0)
    {
        PauseMenu->Padding = 10;
        PauseMenu->DefaultPadding = 10;
        PauseMenu->Components.Init(100, sizeof(GUIComponent));
        PauseMenu->DefaultDim = v2(1000, 1000);
        PauseMenu->Initialized = 1;
        
        int Y = 0;
        
        uint32 RegularColor = 0xFF000000;
        uint32 HoverColor = 0xFFDEC9B3;
        uint32 RegularTextColor = 0xFFFFFFFF;
        uint32 HoverTextColor = 0xFF000000;
        
        v2 BtnDim = v2(550, 75);
        Font *MenuFont = GetFont(&GameState->Assets, FI_Faune50);
        {
            Text TXT = {};
            TXT.Text = "Paused";
            TXT.ID = Btn1;
            TXT.FontType = GetFont(&GameState->Assets, FI_Faune100);;
            TXT.TextColor = 0xFFFFFFFF;
            v2 GridCoords = v2(0, Y++);
            AddText(PauseMenu, GridCoords, TXT);
        }
        {
            Text TXT = {};
            TXT.Text = "";
            TXT.ID = Btn1;
            TXT.FontType = GetFont(&GameState->Assets, FI_Faune100);;
            TXT.TextColor = 0xFFFFFFFF;
            v2 GridCoords = v2(0, Y++);
            AddText(PauseMenu, GridCoords, TXT);
        }
        {
            Button BTN = {};
            BTN.Text = "Reset";
            BTN.FontType = MenuFont;
            BTN.ID = Reset;
            BTN.RegularColor = RegularColor;
            BTN.HoverColor = HoverColor;
            BTN.RegularTextColor = RegularTextColor;
            BTN.HoverTextColor = HoverTextColor;
            v2 GridCoords = v2(0, Y++);
            v2 Dim = BtnDim;
            AddButton(PauseMenu, GridCoords, Dim, BTN);
        }{
            Button BTN = {};
            BTN.Text = "Menu";
            BTN.FontType = MenuFont;
            BTN.ID = Menu;
            BTN.RegularColor = RegularColor;
            BTN.HoverColor = HoverColor;
            BTN.RegularTextColor = RegularTextColor;
            BTN.HoverTextColor = HoverTextColor;
            v2 GridCoords = v2(0, Y++);
            v2 Dim = BtnDim;
            AddButton(PauseMenu, GridCoords, Dim, BTN);
        }
        
        //PauseMenu->BackgroundColor = 0x96000000;
    }
    
    {
        GUIEvents *Events = HandleGUIEvents(PauseMenu, &p->Input);
        if (Events->BtnPressID == Menu) {
            if (GameState->Mode == game_mode::multiplayer) 
                GameState->Disconnect = 1;
            
            SetCursorMode(&p->Input, Arrow);
            GameState->Mode = game_mode::not_in_game;
            GameState->Menu = menu::main_menu;
        }
        else if (Events->BtnPressID == Reset) {
            SetCursorMode(&p->Input, Arrow);
            GameState->ResetGame = true;
            GameState->Mode = game_mode::singleplayer;
            GameState->Menu = menu::not_in_menu;
        }
        
        DrawBackground(GetTexture(&GameState->Assets, GAI_MainMenuBack), GetTopLeftCornerCoords(p), GetDim(p), 0.0f);
        
        UpdateGUI(PauseMenu, v2(p->Dimension.Width, p->Dimension.Height));
        RenderGUI(PauseMenu);
    }
}