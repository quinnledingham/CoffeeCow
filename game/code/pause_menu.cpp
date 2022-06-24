internal void
DrawPauseMenu(platform *p, game_state *GameState)
{
    enum menu_component_id
    {
        MCI_Reset,
        MCI_Menu,
    };
    
    menu *Menu = GetMenu(GameState, menu_mode::pause_menu);
    
    if (!Menu->Initialized)
    {
        int Y = 0;
        
        uint32 DefaultColor = 0xFF000000;
        uint32 HoverColor = 0xFFDEC9B3;
        uint32 DefaultTextColor = 0xFFFFFFFF;
        uint32 HoverTextColor = 0xFF000000;
        
        v2 ButtonDim = v2(550, 75);
        font *Rubik = GetFont(&GameState->Assets, GAFI_Rubik);
        
        MenuInit(Menu, v2(1000, 1000), 10);
        {
            menu_component_text Text = {};
            FontStringSetText(&Text.FontString, "Paused");
            Text.FontString.Font = Rubik;
            Text.FontString.PixelHeight = 100;
            Text.DefaultTextColor = 0xFFFFFFFF;
            v2 GridCoords = v2(0, Y++);
            MenuAddText(Menu, GridCoords, &Text);
        }{
            menu_component_text Text = {};
            FontStringSetText(&Text.FontString, "");
            Text.FontString.Font = Rubik;
            Text.FontString.PixelHeight = 100;
            Text.DefaultTextColor = 0xFFFFFFFF;
            v2 GridCoords = v2(0, Y++);
            MenuAddText(Menu, GridCoords, &Text);
        }{
            menu_component_button Button = {};
            Button.DefaultColor = DefaultColor;
            Button.HoverColor = HoverColor;
            Button.DefaultTextColor = DefaultTextColor;
            Button.HoverTextColor = HoverTextColor;
            
            FontStringSetText(&Button.FontString, "Reset");
            Button.FontString.Font = Rubik;
            Button.FontString.PixelHeight = 50;
            
            v2 GridCoords = v2(0, Y++);
            v2 Dim = ButtonDim;
            MenuAddButton(Menu, MCI_Reset, GridCoords, Dim, &Button);
        }{
            menu_component_button Button = {};
            Button.DefaultColor = DefaultColor;
            Button.HoverColor = HoverColor;
            Button.DefaultTextColor = DefaultTextColor;
            Button.HoverTextColor = HoverTextColor;
            
            FontStringSetText(&Button.FontString, "Main Menu");
            Button.FontString.Font = Rubik;
            Button.FontString.PixelHeight = 50;
            
            v2 GridCoords = v2(0, Y++);
            v2 Dim = ButtonDim;
            MenuAddButton(Menu, MCI_Menu, GridCoords, Dim, &Button);
        }
        //PauseMenu->BackgroundColor = 0x96000000;
        MenuSortActiveComponents(Menu);
    }
    else if (Menu->Reset) {
        PlatformSetCursorMode(p->Input, platform_cursor_mode::Arrow);
        MenuReset(Menu);
    }
    
    HandleMenuEvents(Menu, p->Input);
    if (Menu->Events.ButtonClicked == MCI_Menu) {
        GameState->Game = game_mode::not_in_game;
        SetMenu(GameState, menu_mode::main_menu);
    }
    else if (Menu->Events.ButtonClicked == MCI_Reset) {
        GameState->ResetGame = true;
        GameState->Game = game_mode::singleplayer;
        SetMenu(GameState, menu_mode::not_in_menu);
    }
    
    UpdateMenu(Menu, GetDim(p));
    DrawBackground(GetTexture(&GameState->Assets, GAI_MainMenuBack), GetTopLeftCornerCoords(p), GetDim(p), 0.0f);
    DrawMenu(Menu, 100.0f);
}