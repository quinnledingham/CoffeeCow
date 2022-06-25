internal void
DrawMainMenu(platform *p, game_state *GameState)
{
    enum menu_component_id
    {
        MCI,
        MCI_Singleplayer,
        MCI_LocalMultiplayer,
        MCI_Multiplayer,
        MCI_Quit,
    };
    
    menu *Menu = GetMenu(GameState, menu_mode::main_menu);
    
    if (!Menu->Initialized)
    {
        int Y = 0;
        
        uint32 DefaultColor = 0xFF000000;
        uint32 HoverColor = 0xFFDEC9B3;
        uint32 DefaultTextColor = 0xFFFFFFFF;
        uint32 HoverTextColor = 0xFF000000;
        v2 BtnDim = v2(550, 75);
        font *ButtonFont = GetFont(&GameState->Assets, GAFI_Rubik);
        
        MenuInit(Menu, v2(1000, 1000), 10);
        
        {
            menu_component_logo Logo = {};
            Logo.Tex = GetTexture(&GameState->Assets, GAI_Miz);
            v2 Dim = v2(550, 274);
            v2 GridCoords = v2(0, Y++);
            MenuAddLogo(Menu, GridCoords, Dim, &Logo);
        }
        {
            menu_component_button Button = {};
            Button.DefaultColor = DefaultColor;
            Button.HoverColor = HoverColor;
            Button.DefaultTextColor = DefaultTextColor;
            Button.HoverTextColor = HoverTextColor;
            
            FontStringSetText(&Button.FontString, "Singleplayer");
            Button.FontString.Font = ButtonFont;
            Button.FontString.PixelHeight = 50;
            
            v2 GridCoords = v2(0, Y++);
            v2 Dim = BtnDim;
            MenuAddButton(Menu, MCI_Singleplayer, GridCoords, BtnDim, &Button);
        }
        {
            menu_component_button Button = {};
            Button.DefaultColor = DefaultColor;
            Button.HoverColor = HoverColor;
            Button.DefaultTextColor = DefaultTextColor;
            Button.HoverTextColor = HoverTextColor;
            
            FontStringSetText(&Button.FontString, "Local Multiplayer");
            Button.FontString.Font = ButtonFont;
            Button.FontString.PixelHeight = 50;
            
            v2 GridCoords = v2(0, Y++);
            v2 Dim = BtnDim;
            MenuAddButton(Menu, MCI_LocalMultiplayer, GridCoords, BtnDim, &Button);
        }
        {
            menu_component_button Button = {};
            Button.DefaultColor = DefaultColor;
            Button.HoverColor = HoverColor;
            Button.DefaultTextColor = DefaultTextColor;
            Button.HoverTextColor = HoverTextColor;
            
            FontStringSetText(&Button.FontString, "Online Multiplayer");
            Button.FontString.Font = ButtonFont;
            Button.FontString.PixelHeight = 50;
            
            v2 GridCoords = v2(0, Y++);
            v2 Dim = BtnDim;
            MenuAddButton(Menu, MCI_Multiplayer, GridCoords, BtnDim, &Button);
        }
        {
            menu_component_button Button = {};
            Button.DefaultColor = DefaultColor;
            Button.HoverColor = HoverColor;
            Button.DefaultTextColor = DefaultTextColor;
            Button.HoverTextColor = HoverTextColor;
            
            FontStringSetText(&Button.FontString, "Quit");
            Button.FontString.Font = ButtonFont;
            Button.FontString.PixelHeight = 50;
            
            v2 GridCoords = v2(0, Y++);
            v2 Dim = BtnDim;
            MenuAddButton(Menu, MCI_Quit, GridCoords, BtnDim, &Button);
        }
        MenuSortActiveComponents(Menu);
    }
    else if (Menu->Reset) {
        PlatformSetCursorMode(&p->Input.Mouse, platform_cursor_mode::Arrow);
        MenuReset(Menu);
    }
    
    HandleMenuEvents(Menu, &p->Input);
    if (Menu->Events.ButtonClicked == MCI_Singleplayer) {
        PlatformSetCursorMode(&p->Input.Mouse, platform_cursor_mode::Arrow);
        SetMenu(GameState, menu_mode::not_in_menu);
        GameState->ResetGame = true;
        GameState->Game = game_mode::singleplayer;
    }
    else if (Menu->Events.ButtonClicked == MCI_Multiplayer) {
        SetMenu(GameState, menu_mode::multiplayer_menu);
    }
    else if (Menu->Events.ButtonClicked == MCI_LocalMultiplayer) {
        for (int i = 0; i < ArrayCount(p->Input.Controllers); i++) {
            platform_controller_input *Controller = &p->Input.Controllers[i];
            Controller->IgnoreInputs = false;
        }
        SetMenu(GameState, menu_mode::local_multiplayer_menu);
    }
    else if (Menu->Events.ButtonClicked == MCI_Quit)
        p->Input.Quit = 1;
    
    
    UpdateMenu(Menu, GetDim(p));
    DrawMenu(Menu, 100.0f);
    DrawBackground(GetTexture(&GameState->Assets, GAI_MainMenuBack), GetTopLeftCornerCoords(p), GetDim(p), 0.0f);
}