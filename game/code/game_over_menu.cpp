global_variable GUI GameOverMenu = {};

internal void RenderGameOverMenu(platform *p, game_state *GameState)
{
    if (GameOverMenu.Initialized == 0)
    {
        GameOverMenu.Padding = 10;
        GameOverMenu.DefaultPadding = 10;
        GameOverMenu.Components.Init(100, sizeof(GUIComponent));
        GameOverMenu.DefaultDim = v2(1000, 1000);
        GameOverMenu.Initialized = 1;
        
        int Y = 0;
        
        Text TXT = {};
        Button btn = {};
        TextBox tb = {};
        
        uint32 RegularColor = 0x96000000;
        uint32 HoverColor = 0xFF000000;
        uint32 RegularTextColor = 0xFFFFFFFF;
        uint32 HoverTextColor = 0xFFFFFFFF;
        v2 BtnDim = v2(500, 150);
        Font *MenuFont = GetFont(&GameState->Assets, FI_Faune100);
        
        {
            Text TXT = {};
            TXT.Text = "Game Over";
            TXT.ID = Btn1;
            TXT.FontType = MenuFont;
            TXT.TextColor = 0xFFFFFFFF;
            v2 GridCoords = v2(0, Y++);
            AddText(&GameOverMenu, GridCoords, TXT);
        }{
            Text TXT = {};
            TXT.Text = "";
            TXT.ID = Btn1;
            TXT.FontType = GetFont(&GameState->Assets, FI_Faune100);;
            TXT.TextColor = 0xFFFFFFFF;
            v2 GridCoords = v2(0, Y++);
            AddText(&GameOverMenu, GridCoords, TXT);
        }{
            Button BTN = {};
            BTN.Text = "Play Again";
            BTN.FontType = MenuFont;
            BTN.ID = Reset;
            BTN.RegularColor = RegularColor;
            BTN.HoverColor = HoverColor;
            BTN.RegularTextColor = RegularTextColor;
            BTN.HoverTextColor = HoverTextColor;
            v2 GridCoords = v2(0, Y++);
            v2 Dim = BtnDim;
            AddButton(&GameOverMenu, GridCoords, Dim, BTN);
        }{
            Button BTN = {};
            BTN.Text = "Main Menu";
            BTN.FontType = MenuFont;
            BTN.ID = Menu;
            BTN.RegularColor = RegularColor;
            BTN.HoverColor = HoverColor;
            BTN.RegularTextColor = RegularTextColor;
            BTN.HoverTextColor = HoverTextColor;
            v2 GridCoords = v2(0, Y++);
            v2 Dim = BtnDim;
            AddButton(&GameOverMenu, GridCoords, Dim, BTN);
        }
        
        GameOverMenu.BackgroundColor = 0x96000000;
    }
    
    {
        GUIEvents *Events = HandleGUIEvents(&GameOverMenu, &p->Input);
        if (Events->BtnPressID == Menu) {
            SetCursorMode(&p->Input, Arrow);
            GameState->Mode = game_mode::not_in_game;
            GameState->Menu = menu::main_menu;
        }
        else if (Events->BtnPressID == Reset) {
            GameState->ResetGame = true;
            GameState->Mode = game_mode::singleplayer;
            GameState->Menu = menu::not_in_menu;
        }
        
        platform_keyboard_input *Keyboard = &p->Input.Keyboard;
        if (Keyboard->Escape.NewEndedDown) {
            GameState->Mode = game_mode::singleplayer;
            GameState->Menu = menu::not_in_menu;
        }
        UpdateGUI(&GameOverMenu, v2(p->Dimension.Width, p->Dimension.Height));
        RenderGUI(&GameOverMenu);
        
        //BeginMode2D(*C);
        //RenderPieceGroup(RenderGroup);
        //EndMode2D();
    }
}

