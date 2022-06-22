global_variable menu PauseMenu = {};

internal void
DrawPauseMenu(platform *p, game_state *GameState)
{
    enum menu_component_id
    {
        MCI_Reset,
        MCI_Menu,
    };
    
    menu *Menu = &PauseMenu;
    
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
            menu_text Text = {};
            Text.FontString.Text = "Paused";
            Text.FontString.Font = Rubik;
            Text.FontString.PixelHeight = 100;
            Text.DefaultTextColor = 0xFFFFFFFF;
            v2 GridCoords = v2(0, Y++);
            MenuAddText(Menu, GridCoords, &Text);
        }{
            menu_text Text = {};
            Text.FontString.Text = "";
            Text.FontString.Font = Rubik;
            Text.FontString.PixelHeight = 100;
            Text.DefaultTextColor = 0xFFFFFFFF;
            v2 GridCoords = v2(0, Y++);
            MenuAddText(Menu, GridCoords, &Text);
        }{
            menu_button Button = {};
            Button.DefaultColor = DefaultColor;
            Button.HoverColor = HoverColor;
            Button.DefaultTextColor = DefaultTextColor;
            Button.HoverTextColor = HoverTextColor;
            
            Button.FontString.Text = "Reset";
            Button.FontString.Font = Rubik;
            Button.FontString.PixelHeight = 50;
            
            v2 GridCoords = v2(0, Y++);
            v2 Dim = ButtonDim;
            MenuAddButton(Menu, MCI_Reset, GridCoords, Dim, &Button);
        }{
            menu_button Button = {};
            Button.DefaultColor = DefaultColor;
            Button.HoverColor = HoverColor;
            Button.DefaultTextColor = DefaultTextColor;
            Button.HoverTextColor = HoverTextColor;
            
            Button.FontString.Text = "Main Menu";
            Button.FontString.Font = Rubik;
            Button.FontString.PixelHeight = 50;
            
            v2 GridCoords = v2(0, Y++);
            v2 Dim = ButtonDim;
            MenuAddButton(Menu, MCI_Menu, GridCoords, Dim, &Button);
        }
        //PauseMenu->BackgroundColor = 0x96000000;
    }
    
    HandleMenuEvents(Menu, &p->Input);
    
    if (Menu->Events.ButtonClicked == MCI_Menu) {
        PlatformSetCursorMode(&p->Input, platform_cursor_mode::Arrow);
        GameState->Game = game_mode::not_in_game;
        GameState->Menu = menu_mode::main_menu;
    }
    else if (Menu->Events.ButtonClicked == MCI_Reset) {
        GameState->ResetGame = true;
        GameState->Game = game_mode::singleplayer;
        GameState->Menu = menu_mode::not_in_menu;
    }
    
    DrawBackground(GetTexture(&GameState->Assets, GAI_MainMenuBack), GetTopLeftCornerCoords(p), GetDim(p), 0.0f);
    
    UpdateMenu(Menu, GetDim(p));
    DrawMenu(Menu, 100.0f);
}