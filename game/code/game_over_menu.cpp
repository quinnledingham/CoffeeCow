global_variable menu GameOverMenu = {};

internal void 
DrawGameOverMenu(platform *p, game_state *GameState)
{
    enum menu_component_id
    {
        MCI_Reset,
        MCI_Menu,
    };
    
    menu *Menu = &GameOverMenu;
    
    if (!Menu->Initialized)
    {
        int Y = 0;
        
        uint32 ButtonDefaultColor = 0x96000000;
        uint32 ButtonHoverColor = 0xFF000000;
        uint32 ButtonDefaultTextColor = 0xFFFFFFFF;
        uint32 ButtonHoverTextColor = 0xFFFFFFFF;
        real32 TextPixelHeight = 100;
        v2 ButtonDim = v2(500, 150);
        
        font *Rubik = GetFont(&GameState->Assets, GAFI_Rubik);
        
        MenuInit(Menu, v2(1000, 1000), 10);
        {
            menu_text Text = {};
            Text.FontString.Text = "Game Over";
            Text.FontString.Font = Rubik;
            Text.FontString.PixelHeight = TextPixelHeight;
            Text.DefaultTextColor = 0xFFFFFFFF;
            v2 GridCoords = v2(0, Y++);
            MenuAddText(Menu, GridCoords, &Text);
        }
        {
            menu_text Text = {};
            Text.FontString.Text = "";
            Text.FontString.Font = Rubik;
            Text.FontString.PixelHeight = TextPixelHeight;
            Text.DefaultTextColor = 0xFFFFFFFF;
            v2 GridCoords = v2(0, Y++);
            MenuAddText(Menu, GridCoords, &Text);
        }
        {
            menu_button Button = {};
            Button.DefaultColor = ButtonDefaultColor;
            Button.HoverColor = ButtonHoverColor;
            Button.DefaultTextColor = ButtonDefaultTextColor;
            Button.HoverTextColor = ButtonHoverTextColor;
            
            Button.FontString.Text = "Play Again";
            Button.FontString.Font = Rubik;
            Button.FontString.PixelHeight = TextPixelHeight;
            
            v2 GridCoords = v2(0, Y++);
            v2 Dim = ButtonDim;
            MenuAddButton(Menu, MCI_Reset, GridCoords, Dim, &Button);
        }
        {
            menu_button Button = {};
            Button.DefaultColor = ButtonDefaultColor;
            Button.HoverColor = ButtonHoverColor;
            Button.DefaultTextColor = ButtonDefaultTextColor;
            Button.HoverTextColor = ButtonHoverTextColor;
            
            Button.FontString.Text = "Main Menu";
            Button.FontString.Font = Rubik;
            Button.FontString.PixelHeight = TextPixelHeight;
            
            v2 GridCoords = v2(0, Y++);
            v2 Dim = ButtonDim;
            MenuAddButton(Menu, MCI_Menu, GridCoords, Dim, &Button);
        }
        
        GameOverMenu.BackgroundColor = 0x96000000;
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
    
    UpdateMenu(Menu, GetDim(p));
    DrawMenu(Menu, 100.0f);
}

