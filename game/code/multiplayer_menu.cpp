global_variable menu MultiplayerMenu = {};

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
    
    menu *Menu = &MultiplayerMenu;
    
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
            menu_text Text = {};
            Text.FontString.Text = "PORT:";
            Text.FontString.Font = Rubik;
            Text.FontString.PixelHeight = 50;
            Text.DefaultTextColor = 0xFFFFFFFF;
            v2 GridCoords = v2(0, Y + 1);
            T1 = MenuAddText(Menu, GridCoords, &Text);
        }{
            menu_text Text = {};
            Text.FontString.Text = "IP:";
            Text.FontString.Font = Rubik;
            Text.FontString.PixelHeight = 50;
            Text.DefaultTextColor = 0xFFFFFFFF;
            v2 GridCoords = v2(0, Y);
            MenuAddText(Menu, GridCoords, &Text, T1);
        }{
            menu_textbox TextBox = {};
            TextBox.FontString.Text = "";
            TextBox.FontString.Font = Rubik;
            TextBox.FontString.PixelHeight = 50;
            TextBox.CurrentColor = TextBoxDefaultColor;
            TextBox.CurrentTextColor = TextBoxDefaultTextColor;
            v2 GridCoords = v2(1, Y);
            v2 Dim = TextBoxDim;
            MenuAddTextBox(Menu, MCI_IP, GridCoords, Dim, &TextBox, 0);
        }{
            menu_textbox TextBox = {};
            TextBox.FontString.Text = "";
            TextBox.FontString.Font = Rubik;
            TextBox.FontString.PixelHeight = 50;
            TextBox.CurrentColor = TextBoxDefaultColor;
            TextBox.CurrentTextColor = TextBoxDefaultTextColor;
            v2 GridCoords = v2(1, Y + 1);
            v2 Dim = TextBoxDim;
            MenuAddTextBox(Menu, MCI_Port, GridCoords, Dim, &TextBox, 0);
        }
        Y += 2;
        {
            menu_button Button = {};
            Button.DefaultColor = ButtonDefaultColor;
            Button.HoverColor = ButtonHoverColor;
            Button.DefaultTextColor = ButtonDefaultTextColor;
            Button.HoverTextColor = ButtonHoverTextColor;
            
            Button.FontString.Text = "Join";
            Button.FontString.Font = Rubik;
            Button.FontString.PixelHeight = 50;
            
            v2 GridCoords = v2(0, Y++);
            v2 Dim = ButtonDim;
            MenuAddButton(Menu, MCI_Join, GridCoords, Dim, &Button);
        }{
            menu_button Button = {};
            Button.DefaultColor = ButtonDefaultColor;
            Button.HoverColor = ButtonHoverColor;
            Button.DefaultTextColor = ButtonDefaultTextColor;
            Button.HoverTextColor = ButtonHoverTextColor;
            
            Button.FontString.Text = "Back";
            Button.FontString.Font = Rubik;
            Button.FontString.PixelHeight = 50;
            
            v2 GridCoords = v2(0, Y++);
            v2 Dim = ButtonDim;
            MenuAddButton(Menu, MCI_Back, GridCoords, Dim, &Button);
        }
    }
    
    HandleMenuEvents(Menu, &p->Input);
    
    if (Menu->Events.ButtonClicked == MCI_Join) {
        //PlatformSetCursorMode(&p->Input, platform_cursor_mode::Arrow);
        //GameState->ResetGame = true;
        //GameState->Menu = menu_mode::not_in_menu;
        //GameState->Mode = game_mode::singleplayer;
    }
    else if (Menu->Events.ButtonClicked == MCI_Back) {
        PlatformSetCursorMode(&p->Input, platform_cursor_mode::Arrow);
        GameState->Menu = menu_mode::main_menu;
    }
    
    
    UpdateMenu(Menu, GetDim(p));
    DrawMenu(Menu, 100.0f);
    
    DrawBackground(GetTexture(&GameState->Assets, GAI_MainMenuBack), GetTopLeftCornerCoords(p), GetDim(p), 0.0f);
}