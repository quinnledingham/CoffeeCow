/*
enum menu_component_id
{
    MCI_Test,
    MCI_Test2,
};

menu_component But = {};
But.ID = MCI_Test;

GUI *MainMenu = &GameState->GUIs[0];

{
    menu_text TXT = {};
    TXT.DefaultTextColor = 0xFFFFFFFF;
    TXT.FontString.Text = "Paused";
    TXT.FontString.Font = GameState->Assets.Fontss[GAFI_Rubik];
    TXT.FontString.PixelHeight = 100;
    //MenuAddText(PauseMenu);
}

if (MainMenu->Initialized == 0) {
    MainMenu->Padding = 10;
    MainMenu->DefaultPadding = 10;
    MainMenu->Components.Init(100, sizeof(GUIComponent));
    MainMenu->DefaultDim = v2(1000, 1000);
    MainMenu->Initialized = 1;
    
    int Y = 0;
    uint32 RegularColor = 0xFF000000;
    uint32 HoverColor = 0xFFDEC9B3;
    uint32 RegularTextColor = 0xFFFFFFFF;
    uint32 HoverTextColor = 0xFF000000;
    v2 BtnDim = v2(550, 75);
    Font *BtnFont = GetFont(&GameState->Assets, FI_Faune50);
    
    {
        gui_logo LOGO = {};
        LOGO.Tex = GetTexture(&GameState->Assets, GAI_Miz);
        v2 Dim = v2(550, 274);
        v2 GridCoords = v2(0, Y++);
        GUIAddLogo(MainMenu, GridCoords, Dim, LOGO);
    }{
        Button BTN = {};
        BTN.Text = "Singleplayer";
        BTN.FontType = BtnFont;
        BTN.ID = GameStart;
        BTN.RegularColor = RegularColor;
        BTN.HoverColor = HoverColor;
        BTN.RegularTextColor = RegularTextColor;
        BTN.HoverTextColor = HoverTextColor;
        v2 GridCoords = v2(0, Y++);
        v2 Dim = BtnDim;
        AddButton(MainMenu, GridCoords, Dim, BTN);
    }{
        Button BTN = {};
        BTN.Text = "Multiplayer";
        BTN.FontType = BtnFont;
        BTN.ID = Multiplayer;
        BTN.RegularColor = RegularColor;
        BTN.HoverColor = HoverColor;
        BTN.RegularTextColor = RegularTextColor;
        BTN.HoverTextColor = HoverTextColor;
        v2 GridCoords = v2(0, Y++);
        v2 Dim = BtnDim;
        AddButton(MainMenu, GridCoords, Dim, BTN);
    }{
        Button BTN = {};
        BTN.Text = "Quit";
        BTN.FontType = BtnFont;
        BTN.ID = Quit;
        BTN.RegularColor = RegularColor;
        BTN.HoverColor = HoverColor;
        BTN.RegularTextColor = RegularTextColor;
        BTN.HoverTextColor = HoverTextColor;
        v2 GridCoords = v2(0, Y++);
        v2 Dim = BtnDim;
        AddButton(MainMenu, GridCoords, Dim, BTN);
    }
    
    //MainMenu->BackgroundColor = 0x96000000;
    
    FontStringInit(&GameState->Test, GameState->Assets.Fontss[GAFI_Rubik], "Look Bro,", 100,  0xFF89CFF0);
}

{
    GUIEvents *Events = HandleGUIEvents(MainMenu, &p->Input);
    
    if (Events->BtnPressID == GameStart) {
        SetCursorMode(&p->Input, Arrow);
        GameState->ResetGame = true;
        GameState->Menu = menu_mode::not_in_menu;
        GameState->Mode = game_mode::singleplayer;
    }
    else if (Events->BtnPressID == Multiplayer) {
        SetCursorMode(&p->Input, Arrow);
        GameState->Menu = menu_mode::multiplayer_menu;
    }
    else if (Events->BtnPressID == Btn4) {
        //createClient(&client, GetTextBoxText(&MainMenu, IP), GetTextBoxText(&MainMenu, PORT), TCP);
    }
    else if (Events->BtnPressID == Quit)
        p->Input.Quit = 1;
    
    UpdateGUI(MainMenu, v2(p->Dimension.Width, p->Dimension.Height));
    RenderGUI(MainMenu);
    
    DrawBackground(GetTexture(&GameState->Assets, GAI_MainMenuBack), GetTopLeftCornerCoords(p), GetDim(p), 0.0f);
    
    v2 PlatformDim = GetDim(p);
    if (GameState->OldPlatformDim != PlatformDim) {
        font *Temp = GameState->Assets.Fontss[GAFI_Rubik];
        FontStringResize(&GameState->Test, ResizeEquivalentAmount(GameState->Test.PixelHeight, GameState->OldPlatformDim.y, PlatformDim.y));
        
        GameState->OldPlatformDim = PlatformDim;
    }
    
    FontStringPrint(&GameState->Test, GetTopLeftCornerCoords(p) + 100);
    
    BeginMode2D(GameState->C);
    RenderPieceGroup(RenderGroup);
    EndMode2D();
}
*/

internal void
DrawMainMenu(platform *p, game_state *GameState)
{
    enum menu_component_id
    {
        MCI,
        MCI_Singleplayer,
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
            menu_logo Logo = {};
            Logo.Tex = GetTexture(&GameState->Assets, GAI_Miz);
            v2 Dim = v2(550, 274);
            v2 GridCoords = v2(0, Y++);
            MenuAddLogo(Menu, GridCoords, Dim, &Logo);
        }
        {
            menu_button Button = {};
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
            menu_button Button = {};
            Button.DefaultColor = DefaultColor;
            Button.HoverColor = HoverColor;
            Button.DefaultTextColor = DefaultTextColor;
            Button.HoverTextColor = HoverTextColor;
            
            FontStringSetText(&Button.FontString, "Multiplayer");
            Button.FontString.Font = ButtonFont;
            Button.FontString.PixelHeight = 50;
            
            v2 GridCoords = v2(0, Y++);
            v2 Dim = BtnDim;
            MenuAddButton(Menu, MCI_Multiplayer, GridCoords, BtnDim, &Button);
        }
        {
            menu_button Button = {};
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
        PlatformSetCursorMode(p->Input, platform_cursor_mode::Arrow);
        MenuReset(Menu);
    }
    
    HandleMenuEvents(Menu, p->Input);
    if (Menu->Events.ButtonClicked == MCI_Singleplayer) {
        PlatformSetCursorMode(p->Input, platform_cursor_mode::Arrow);
        SetMenu(GameState, menu_mode::not_in_menu);
        GameState->ResetGame = true;
        GameState->Game = game_mode::singleplayer;
    }
    else if (Menu->Events.ButtonClicked == MCI_Multiplayer) {
        SetMenu(GameState, menu_mode::multiplayer_menu);
    }
    else if (Menu->Events.ButtonClicked == MCI_Quit)
        p->Input->Quit = 1;
    
    
    UpdateMenu(Menu, GetDim(p));
    DrawMenu(Menu, 100.0f);
    DrawBackground(GetTexture(&GameState->Assets, GAI_MainMenuBack), GetTopLeftCornerCoords(p), GetDim(p), 0.0f);
}