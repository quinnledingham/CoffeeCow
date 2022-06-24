internal void 
DrawGameOverMenu(platform *p, game_state *GameState)
{
    enum menu_component_id
    {
        MCI_Reset,
        MCI_Menu,
    };
    
    menu *Menu = GetMenu(GameState, menu_mode::game_over_menu);
    
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
            menu_component_text Text = {};
            FontStringSetText(&Text.FontString, "Game Over");
            Text.FontString.Font = Rubik;
            Text.FontString.PixelHeight = TextPixelHeight;
            Text.DefaultTextColor = 0xFFFFFFFF;
            v2 GridCoords = v2(0, Y++);
            MenuAddText(Menu, GridCoords, &Text);
        }
        {
            menu_component_text Text = {};
            FontStringSetText(&Text.FontString, "");
            Text.FontString.Font = Rubik;
            Text.FontString.PixelHeight = TextPixelHeight;
            Text.DefaultTextColor = 0xFFFFFFFF;
            v2 GridCoords = v2(0, Y++);
            MenuAddText(Menu, GridCoords, &Text);
        }
        {
            menu_component_button Button = {};
            Button.DefaultColor = ButtonDefaultColor;
            Button.HoverColor = ButtonHoverColor;
            Button.DefaultTextColor = ButtonDefaultTextColor;
            Button.HoverTextColor = ButtonHoverTextColor;
            
            FontStringSetText(&Button.FontString, "Play Again");
            Button.FontString.Font = Rubik;
            Button.FontString.PixelHeight = TextPixelHeight;
            
            v2 GridCoords = v2(0, Y++);
            v2 Dim = ButtonDim;
            MenuAddButton(Menu, MCI_Reset, GridCoords, Dim, &Button);
        }
        {
            menu_component_button Button = {};
            Button.DefaultColor = ButtonDefaultColor;
            Button.HoverColor = ButtonHoverColor;
            Button.DefaultTextColor = ButtonDefaultTextColor;
            Button.HoverTextColor = ButtonHoverTextColor;
            
            FontStringSetText(&Button.FontString, "Main Menu");
            Button.FontString.Font = Rubik;
            Button.FontString.PixelHeight = TextPixelHeight;
            
            v2 GridCoords = v2(0, Y++);
            v2 Dim = ButtonDim;
            MenuAddButton(Menu, MCI_Menu, GridCoords, Dim, &Button);
        }
        
        Menu->BackgroundColor = 0x32000000;
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
    DrawMenu(Menu, 100.0f);
}

