internal void
DrawMultiplayerMenu(platform *p, game_state *GameState)
{
    enum menu_component_id
    {
        MCI_IP,
        MCI_Port,
        MCI_Join,
        MCI_Back,
    };
    
    menu *Menu = GetMenu(GameState, menu_mode::multiplayer_menu);
    
    if (!Menu->Initialized)
    {
        int Y = 0;
        
        uint32 ButtonDefaultColor = 0xFF000000;
        uint32 ButtonHoverColor = 0xFFDEC9B3;
        uint32 ButtonDefaultTextColor = 0xFFFFFFFF;
        uint32 ButtonHoverTextColor = 0xFF000000;
        
        uint32 TextBoxDefaultColor = 0xFFFFFFFF;
        uint32 TextBoxDefaultTextColor = 0xFF000000;
        
        v2 TextBoxDim = v2(400, 50);
        v2 ButtonDim = v2(250, 75);
        
        font *Rubik = GetFont(&GameState->Assets, GAFI_Rubik);
        
        MenuInit(Menu, v2(1000, 1000), 10);
        
        menu_component *T1;
        {
            menu_component_text Text = {};
            FontStringSetText(&Text.FontString, "PORT:");
            Text.FontString.Font = Rubik;
            Text.FontString.PixelHeight = 50;
            Text.DefaultTextColor = 0xFFFFFFFF;
            v2 GridCoords = v2(0, Y + 1);
            T1 = MenuAddText(Menu, GridCoords, &Text);
        }{
            menu_component_text Text = {};
            FontStringSetText(&Text.FontString, "IP:");
            Text.FontString.Font = Rubik;
            Text.FontString.PixelHeight = 50;
            Text.DefaultTextColor = 0xFFFFFFFF;
            v2 GridCoords = v2(0, Y);
            MenuAddText(Menu, GridCoords, &Text, T1);
        }{
            menu_component_textbox TextBox = {};
            FontStringSetText(&TextBox.FontString, "");
            TextBox.FontString.Font = Rubik;
            TextBox.FontString.PixelHeight = 50;
            TextBox.CurrentColor = TextBoxDefaultColor;
            TextBox.CurrentTextColor = TextBoxDefaultTextColor;
            v2 GridCoords = v2(1, Y);
            v2 Dim = TextBoxDim;
            MenuAddTextBox(Menu, MCI_IP, GridCoords, Dim, &TextBox);
        }{
            menu_component_textbox TextBox = {};
            FontStringSetText(&TextBox.FontString, "");
            TextBox.FontString.Font = Rubik;
            TextBox.FontString.PixelHeight = 50;
            TextBox.CurrentColor = TextBoxDefaultColor;
            TextBox.CurrentTextColor = TextBoxDefaultTextColor;
            v2 GridCoords = v2(1, Y + 1);
            v2 Dim = TextBoxDim;
            MenuAddTextBox(Menu, MCI_Port, GridCoords, Dim, &TextBox);
        }
        Y += 2;
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
            MenuAddButton(Menu, MCI_Join, GridCoords, Dim, &Button);
        }{
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
    if (Menu->Events.ButtonClicked == MCI_Join) {
        PlatformSetCursorMode(&p->Input.Mouse, platform_cursor_mode::Arrow);
        GameState->ResetGame = true;
        SetMenu(GameState, menu_mode::not_in_menu);
        GameState->Game = game_mode::multiplayer;
        
        GameState->IP = MenuGetTextBoxText(Menu, MCI_IP);
        GameState->Port = MenuGetTextBoxText(Menu, MCI_Port);
    }
    else if (Menu->Events.ButtonClicked == MCI_Back) {
        SetMenu(GameState, menu_mode::main_menu);
    }
    
    UpdateMenu(Menu, GetDim(p));
    DrawMenu(Menu, 100.0f);
    DrawBackground(GetTexture(&GameState->Assets, GAI_MainMenuBack), GetTopLeftCornerCoords(p), GetDim(p), 0.0f);
}