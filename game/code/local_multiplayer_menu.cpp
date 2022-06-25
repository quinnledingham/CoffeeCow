internal void
DrawLocalMultiplayerMenu(platform *p, game_state *GameState)
{
    enum player_id
    {
        PI_Player1,
        PI_Player2,
        PI_Player3,
        PI_Player4,
        
        PI_Count
    };
    
    enum menu_component_id
    {
        MCI_Default,
        MCI_Start,
        MCI_Back,
    };
    
    menu *Menu = GetMenu(GameState, menu_mode::local_multiplayer_menu);
    
    if (!Menu->Initialized)
    {
        int Y = 0;
        
        uint32 ButtonDefaultColor = 0xFF000000;
        uint32 ButtonHoverColor = 0xFFDEC9B3;
        uint32 ButtonDefaultTextColor = 0xFFFFFFFF;
        uint32 ButtonHoverTextColor = 0xFF000000;
        Texture *DefaultTexture = GetTexture(&GameState->Assets, GAI_Join);
        Texture *ActiveTexture = GetTexture(&GameState->Assets, GAI_JoinHover);
        Texture *ClickedTexture = GetTexture(&GameState->Assets, GAI_JoinAlt);
        Texture *ActiveClickedTexture = GetTexture(&GameState->Assets, GAI_JoinAltHover);
        
        font *Rubik = GetFont(&GameState->Assets, GAFI_Rubik);
        
        v2 CharDim = v2(200, 400);
        v2 ButtonDim = v2(250, 75);
        
        MenuInit(Menu, v2(1000, 1000), 10);
        
        {
            menu_component_checkbox CheckBox = {};
            
            CheckBox.DefaultTexture = DefaultTexture;
            CheckBox.ActiveTexture = ActiveTexture;
            CheckBox.ClickedTexture = ClickedTexture;
            CheckBox.ActiveClickedTexture = ActiveClickedTexture;
            
            v2 GridCoords = v2(0, 0);
            v2 Dim = CharDim;
            MenuAddCheckBox(Menu, PI_Player1, GridCoords, Dim, &CheckBox);
        }
        {
            menu_component_checkbox CheckBox = {};
            
            CheckBox.DefaultTexture = DefaultTexture;
            CheckBox.ActiveTexture = ActiveTexture;
            CheckBox.ClickedTexture = ClickedTexture;
            CheckBox.ActiveClickedTexture = ActiveClickedTexture;
            
            v2 GridCoords = v2(1, 0);
            v2 Dim = CharDim;
            MenuAddCheckBox(Menu, PI_Player2, GridCoords, Dim, &CheckBox);
        }
        {
            menu_component_button Button = {};
            menu_component_checkbox CheckBox = {};
            
            CheckBox.DefaultTexture = DefaultTexture;
            CheckBox.ActiveTexture = ActiveTexture;
            CheckBox.ClickedTexture = ClickedTexture;
            CheckBox.ActiveClickedTexture = ActiveClickedTexture;
            
            v2 GridCoords = v2(2, 0);
            v2 Dim = CharDim;
            MenuAddCheckBox(Menu, PI_Player3, GridCoords, Dim, &CheckBox);
        }
        {
            menu_component_checkbox CheckBox = {};
            
            CheckBox.DefaultTexture = DefaultTexture;
            CheckBox.ActiveTexture = ActiveTexture;
            CheckBox.ClickedTexture = ClickedTexture;
            CheckBox.ActiveClickedTexture = ActiveClickedTexture;
            
            v2 GridCoords = v2(3, Y++);
            v2 Dim = CharDim;
            MenuAddCheckBox(Menu, PI_Player4, GridCoords, Dim, &CheckBox);
        }
        
        /*
        {
            menu_component_logo Logo = {};
            Logo.Tex = GetTexture(&GameState->Assets, GAI_Join);
            Logo.Alt = GetTexture(&GameState->Assets, GAI_JoinAlt);
            v2 Dim = CharDim;
            v2 GridCoords = v2(0, 0);
            MenuAddLogo(Menu, GridCoords, Dim, &Logo);
        }
        {
            menu_component_logo Logo = {};
            Logo.Tex = GetTexture(&GameState->Assets, GAI_Join);
            Logo.Alt = GetTexture(&GameState->Assets, GAI_JoinAlt);
            v2 Dim = CharDim;
            v2 GridCoords = v2(1, 0);
            MenuAddLogo(Menu, GridCoords, Dim, &Logo);
        }
        {
            menu_component_logo Logo = {};
            Logo.Tex = GetTexture(&GameState->Assets, GAI_Join);
            Logo.Alt = GetTexture(&GameState->Assets, GAI_JoinAlt);
            v2 Dim = CharDim;
            v2 GridCoords = v2(2, 0);
            MenuAddLogo(Menu, GridCoords, Dim, &Logo);
        }
        {
            menu_component_logo Logo = {};
            Logo.Tex = GetTexture(&GameState->Assets, GAI_Join);
            Logo.Alt = GetTexture(&GameState->Assets, GAI_JoinAlt);
            v2 Dim = CharDim;
            v2 GridCoords = v2(3, Y++);
            MenuAddLogo(Menu, GridCoords, Dim, &Logo);
        }
        */
        {
            menu_component_button Button = {};
            Button.DefaultColor = ButtonDefaultColor;
            Button.HoverColor = ButtonHoverColor;
            Button.DefaultTextColor = ButtonDefaultTextColor;
            Button.HoverTextColor = ButtonHoverTextColor;
            
            FontStringSetText(&Button.FontString, "Join");
            Button.FontString.Font = Rubik;
            Button.FontString.PixelHeight = 50;
            
            v2 GridCoords = v2(0, Y++);
            v2 Dim = ButtonDim;
            MenuAddButton(Menu, MCI_Start, GridCoords, Dim, &Button);
        }
        {
            menu_component_button Button = {};
            Button.DefaultColor = ButtonDefaultColor;
            Button.HoverColor = ButtonHoverColor;
            Button.DefaultTextColor = ButtonDefaultTextColor;
            Button.HoverTextColor = ButtonHoverTextColor;
            
            FontStringSetText(&Button.FontString, "Back");
            Button.FontString.Font = Rubik;
            Button.FontString.PixelHeight = 50;
            
            v2 GridCoords = v2(0, Y++);
            v2 Dim = ButtonDim;
            MenuAddButton(Menu, MCI_Back, GridCoords, Dim, &Button);
        }
        
        MenuSortActiveComponents(Menu);
    }
    else if (Menu->Reset) {
        PlatformSetCursorMode(&p->Input.Mouse, platform_cursor_mode::Arrow);
        MenuReset(Menu);
    }
    
    HandleMenuEvents(Menu, &p->Input);
    
    if (Menu->Events.ButtonClicked == MCI_Start) {
        for (int i = 0; i < PI_Count; i++) {
            menu_component *Player = MenuGetComponent(Menu->CheckBoxes, i);
            menu_component_checkbox *CheckBox = (menu_component_checkbox*)Player->Data;
            GameState->Players[i].Input = CheckBox->Controller;
        }
        
        PlatformSetCursorMode(&p->Input.Mouse, platform_cursor_mode::Arrow);
        SetMenu(GameState, menu_mode::not_in_menu);
        GameState->ResetGame = true;
        GameState->Game = game_mode::local_multiplayer;
        
        
    }
    else if (Menu->Events.ButtonClicked == MCI_Back) {
        SetMenu(GameState, menu_mode::main_menu);
    }
    
    UpdateMenu(Menu, GetDim(p));
    DrawMenu(Menu, 100.0f);
    DrawBackground(GetTexture(&GameState->Assets, GAI_MainMenuBack), GetTopLeftCornerCoords(p), GetDim(p), 0.0f);
}