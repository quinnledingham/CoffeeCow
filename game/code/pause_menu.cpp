GUI *PauseMenu = &GameState->GUIs[2];

if (PauseMenu->Initialized == 0)
{
    PauseMenu->Padding = 10;
    PauseMenu->DefaultPadding = 10;
    PauseMenu->Components.Init(100, sizeof(GUIComponent));
    PauseMenu->DefaultDim = v2(1000, 1000);
    PauseMenu->Initialized = 1;
    
    int Y = 0;
    
    Text TXT = {};
    Button btn = {};
    TextBox tb = {};
    
    TXT.Text = "Paused";
    TXT.ID = Btn1;
    TXT.FontType = GetFont(&GameState->Assets, FI_Faune100);
    TXT.TextColor = 0xFF000000;
    AddText(PauseMenu, v2(0, Y++), TXT);
    
    btn = 
    {
        "Reset",    // Text
        GetFont(&GameState->Assets, FI_Faune100),   // Font
        Reset,       // ID
        0,          // Color (CurrentColor)
        0xFF32a89b, // RegularColor
        0xFFeba434, // HoverColor
        0xFFFFFFFF, // TextColor
    };
    AddButton(PauseMenu, v2(0, Y++), v2(600, 150), btn);
    
    btn = 
    {
        "Menu",    // Text
        GetFont(&GameState->Assets, FI_Faune100),   // Font
        Menu,       // ID
        0,          // Color (CurrentColor)
        0xFF32a89b, // RegularColor
        0xFFeba434, // HoverColor
        0xFFFFFFFF, // TextColor
    };
    AddButton(PauseMenu, v2(0, Y++), v2(600, 150), btn);
}

{
    game_packet Send = {};
    
    GUIEvents Events = HandleGUIEvents(PauseMenu, &p->Input);
    if (Events.BtnPressID == Menu) {
        if (GameState->PreviousMode == game_mode::multiplayer)
            Send.Disconnect = true;
        
        SetCursorMode(&p->Input, Arrow);
        GameState->Mode = game_mode::not_in_game;
        GameState->Menu = menu::main_menu;
    }
    else if (Events.BtnPressID == Reset) {
        GameState->ResetGame = true;
        GameState->Mode = game_mode::singleplayer;
        GameState->Menu = menu::not_in_menu;
    }
    
    platform_keyboard_input *Controller = &p->Input.Keyboard;
    if (Controller->Escape.NewEndedDown)
        GameState->Mode = game_mode::singleplayer;
    
    if (GameState->PreviousMode == game_mode::multiplayer) {
        memset(GameState->Buffer, 0, BUF_SIZE);
        memcpy(GameState->Buffer, &Send, sizeof(game_packet)); 
        GameState->client.sendq(GameState->Buffer, SEND_BUFFER_SIZE);
        
        if (!Send.Disconnect) {
            memset(GameState->Buffer, 0, BUF_SIZE);
            GameState->client.recvq(GameState->Buffer, BUF_SIZE);
        }
        else {
            GameState->client.disconnect();
        }
    }
    
    UpdateGUI(PauseMenu, v2(p->Dimension.Width, p->Dimension.Height));
    RenderGUI(PauseMenu);
    
    BeginMode2D(GameState->C);
    RenderPieceGroup(RenderGroup);
    EndMode2D();
}