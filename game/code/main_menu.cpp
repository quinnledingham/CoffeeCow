internal v2
GetGridCoords(char *Input, int *X, int *Y)
{
    v2_string Coords = Getv2StringFromChar(Input);
    
    if (Equal(Coords.y.Value, "Y++"))
        return v2(atoi(Coords.x.Value), (*Y)++);
    else if (Equal(Coords.y.Value, "Y"))
        return v2(atoi(Coords.x.Value), (*Y));
    else if (Equal(Coords.y.Value, "Y+1"))
        return v2(atoi(Coords.x.Value), (*Y)+1);
    else if (Equal(Coords.y.Value, "Y+2"))
        return v2(atoi(Coords.x.Value), (*Y)+2);
    else if (Equal(Coords.y.Value, "Y+3"))
        return v2(atoi(Coords.x.Value), (*Y)+3);
    else
        return v2(atoi(Coords.x.Value), atoi(Coords.y.Value));
}

internal void
ReadMenuFromFile(menu *Menu, const char* FileName, game_state *GameState, pair_int_string *IDs, int NumOfIDs)
{
    entire_file EntireFile = ReadEntireFile(FileName);
    const char* Cursor = (const char*)EntireFile.Contents;
    
    if (Cursor == 0)
        return;
    
    bool FillCollection = false;
    
    int NumOfReplaceTokens = 0;
    menu_token ReplaceTokens[10];
    memset(ReplaceTokens, 0, ArrayCount(ReplaceTokens) * sizeof(menu_token));
    
    int NumOfCollections = 0;
    menu_token_collection Collections[10];
    memset(Collections, 0, ArrayCount(Collections) * sizeof(menu_token_collection));
    
    menu_token_collection *Collection = 0;
    
    int i = 0;
    while(1)
    {
        // Read Line
        char Line[100];
        memset(Line, 0, 100);
        int j = 0;
        while (Cursor[i] != 0x0A && i < (int)EntireFile.ContentsSize)
        {
            Line[j] = Cursor[i];
            j++;
            i++;
        }
        
        if (j != 0)
        {
            if (Line[0] == '#')
            {
                //fprintf(stderr, "%s\n", Line);
                
                if (Equal(Line + 1, "end"))
                    break;
                else
                {
                    Collection = &Collections[NumOfCollections++];
                    CopyBuffer(Collection->Type, Line+1, GetLength(Line+1));
                    FillCollection = true;
                }
            }
            else if (FillCollection)
            {
                //fprintf(stderr, "%s\n", Line);
                
                int CursorIndex = 0;
                int TokenIndex = 0;
                menu_token *Token = &Collection->Tokens[Collection->NumOfTokens++];
                while (Line[CursorIndex] != ':')
                {
                    if (Line[CursorIndex] != 0x20) {
                        Token->ID[TokenIndex] = Line[CursorIndex];
                        TokenIndex++;
                    }
                    CursorIndex++;
                }
                
                CursorIndex++;
                TokenIndex = 0;
                while (CursorIndex != j)
                {
                    if (Line[CursorIndex] != 0x20 || Equal(Token->ID, "Text")) {
                        Token->Value[TokenIndex] = Line[CursorIndex];
                        TokenIndex++;
                    }
                    CursorIndex++;
                }
            }
        }
        
        if (i >= (int)EntireFile.ContentsSize)
            break;
        
        i++;
    }
    DestroyEntireFile(EntireFile);
    
    // Replace
    for (int i = 1; i < NumOfCollections; i++)
    {
        menu_token_collection *Collection = &Collections[i];
        for (int j = 0; j < Collection->NumOfTokens; j++)
        {
            menu_token *Token = &Collection->Tokens[j];
            for (int k = 0; k < Collections[0].NumOfTokens; k++)
            {
                menu_token *ReplaceToken = &Collections[0].Tokens[k];
                
                if (Equal(Token->Value, ReplaceToken->ID))
                {
                    memset(Token->Value, 0, 100);
                    CopyBuffer(Token->Value, ReplaceToken->Value, GetLength(ReplaceToken->Value));
                }
            }
        }
    }
    
    int Y = 0;
    int X = 0;
    for (int i = 1; i < NumOfCollections; i++)
    {
        menu_token_collection *Collection = &Collections[i];
        if (Equal(Collection->Type, "Menu"))
        {
            for (int j = 0; j < Collection->NumOfTokens; j++)
            {
                menu_token *Token = &Collection->Tokens[j];
                if (Equal(Token->ID,  "BackgroundColor"))
                    Menu->BackgroundColor = StringHex2Int(Token->Value);
                else if (Equal(Token->ID,  "BackgroundTexture"))
                    Menu->BackgroundTexture = GetTexture(&GameState->Assets, Token->Value);
                else if (Equal(Token->ID,  "DefaultDim"))
                    Menu->DefaultDim = GetCoordsFromChar(Token->Value);
                else if (Equal(Token->ID,  "DefaultPadding"))
                    Menu->DefaultPadding = atoi(Token->Value);
            }
        }
        else if (Equal(Collection->Type, "Logo"))
        {
            menu_component_logo Logo = {};
            v2 Dim;
            v2 GridCoords;
            for (int j = 0; j < Collection->NumOfTokens; j++)
            {
                menu_token *Token = &Collection->Tokens[j];
                if (Equal(Token->ID,  "Texture"))
                    Logo.Tex = GetTexture(&GameState->Assets, Token->Value);
                else if (Equal(Token->ID,  "Dim")) 
                    Dim = GetCoordsFromChar(Token->Value);
                else if (Equal(Token->ID,  "GridCoords")) 
                    GridCoords = GetGridCoords(Token->Value, &X, &Y);
            }
            MenuAddLogo(Menu, GridCoords, Dim, &Logo);
        }
        else if (Equal(Collection->Type, "Button"))
        {
            menu_component_button Button = {};
            v2 GridCoords;
            v2 Dim;
            int ID = 0;
            
            for (int j = 0; j < Collection->NumOfTokens; j++)
            {
                menu_token *Token = &Collection->Tokens[j];
                if (Equal(Token->ID,  "ID")) 
                    ID = GetInt(IDs, NumOfIDs, Token->Value);
                else if (Equal(Token->ID,  "DefaultColor"))
                    Button.DefaultColor = StringHex2Int(Token->Value);
                else if (Equal(Token->ID,  "HoverColor"))
                    Button.HoverColor = StringHex2Int(Token->Value);
                else if (Equal(Token->ID,  "DefaultTextColor"))
                    Button.DefaultTextColor = StringHex2Int(Token->Value);
                else if (Equal(Token->ID,  "HoverTextColor"))
                    Button.HoverTextColor = StringHex2Int(Token->Value);
                else if (Equal(Token->ID,  "Text"))
                    FontStringSetText(&Button.FontString, Token->Value);
                else if (Equal(Token->ID,  "Font"))
                    Button.FontString.Font = GetFont(&GameState->Assets, Token->Value);
                else if (Equal(Token->ID,  "PixelHeight"))
                    Button.FontString.PixelHeight = (real32)atoi(Token->Value);
                else if (Equal(Token->ID,  "Dim")) 
                    Dim = GetCoordsFromChar(Token->Value);
                else if (Equal(Token->ID,  "GridCoords")) 
                    GridCoords = GetGridCoords(Token->Value, &X, &Y);
            }
            
            MenuAddButton(Menu, ID, GridCoords, Dim, &Button);
        }
        else if (Equal(Collection->Type, "Text"))
        {
            menu_component_text Text = {};
            v2 GridCoords;
            v2 Dim;
            int ID = 0;
            menu_component *Align = 0;
            
            for (int j = 0; j < Collection->NumOfTokens; j++)
            {
                menu_token *Token = &Collection->Tokens[j];
                if (Equal(Token->ID,  "ID")) 
                    ID = GetInt(IDs, NumOfIDs, Token->Value);
                else if (Equal(Token->ID,  "DefaultTextColor"))
                    Text.DefaultTextColor = StringHex2Int(Token->Value);
                else if (Equal(Token->ID,  "Text"))
                    FontStringSetText(&Text.FontString, Token->Value);
                else if (Equal(Token->ID,  "Font"))
                    Text.FontString.Font = GetFont(&GameState->Assets, Token->Value);
                else if (Equal(Token->ID,  "PixelHeight"))
                    Text.FontString.PixelHeight = (real32)atoi(Token->Value);
                else if (Equal(Token->ID,  "Dim")) 
                    Dim = GetCoordsFromChar(Token->Value);
                else if (Equal(Token->ID,  "Align")) 
                    Align = MenuGetComponent(Menu, IDs, NumOfIDs, Token->Value);
                else if (Equal(Token->ID,  "GridCoords")) 
                    GridCoords = GetGridCoords(Token->Value, &X, &Y);
            }
            
            MenuAddText(Menu, ID, GridCoords, &Text, Align);
        }
        else if (Equal(Collection->Type, "TextBox"))
        {
            menu_component_textbox TextBox = {};
            v2 GridCoords;
            v2 Dim;
            int ID = 0;
            
            for (int j = 0; j < Collection->NumOfTokens; j++)
            {
                menu_token *Token = &Collection->Tokens[j];
                if (Equal(Token->ID,  "ID")) 
                    ID = GetInt(IDs, NumOfIDs, Token->Value);
                else if (Equal(Token->ID,  "DefaultTextColor"))
                    TextBox.CurrentTextColor = StringHex2Int(Token->Value);
                else if (Equal(Token->ID,  "DefaultColor"))
                    TextBox.CurrentColor = StringHex2Int(Token->Value);
                else if (Equal(Token->ID,  "Text"))
                    FontStringSetText(&TextBox.FontString, Token->Value);
                else if (Equal(Token->ID,  "Font"))
                    TextBox.FontString.Font = GetFont(&GameState->Assets, Token->Value);
                else if (Equal(Token->ID,  "PixelHeight"))
                    TextBox.FontString.PixelHeight = (real32)atoi(Token->Value);
                else if (Equal(Token->ID,  "Dim")) 
                    Dim = GetCoordsFromChar(Token->Value);
                else if (Equal(Token->ID,  "GridCoords")) 
                    GridCoords = GetGridCoords(Token->Value, &X, &Y);
            }
            
            MenuAddTextBox(Menu, ID, GridCoords, Dim, &TextBox);
        }
        else if (Equal(Collection->Type, "CheckBox"))
        {
            menu_component_checkbox CheckBox = {};
            v2 GridCoords;
            v2 Dim;
            int ID = 0;
            
            for (int j = 0; j < Collection->NumOfTokens; j++)
            {
                menu_token *Token = &Collection->Tokens[j];
                if (Equal(Token->ID,  "ID")) 
                    ID = GetInt(IDs, NumOfIDs, Token->Value);
                else if (Equal(Token->ID,  "DefaultTexture"))
                    CheckBox.DefaultTexture = GetTexture(&GameState->Assets, Token->Value);
                else if (Equal(Token->ID,  "ActiveTexture"))
                    CheckBox.ActiveTexture = GetTexture(&GameState->Assets, Token->Value);
                else if (Equal(Token->ID,  "ClickedTexture"))
                    CheckBox.ClickedTexture = GetTexture(&GameState->Assets, Token->Value);
                else if (Equal(Token->ID,  "ActiveClickedTexture"))
                    CheckBox.ActiveClickedTexture = GetTexture(&GameState->Assets, Token->Value);
                else if (Equal(Token->ID,  "Dim")) 
                    Dim = GetCoordsFromChar(Token->Value);
                else if (Equal(Token->ID,  "GridCoords")) 
                    GridCoords = GetGridCoords(Token->Value, &X, &Y);
            }
            
            MenuAddCheckBox(Menu, ID, GridCoords, Dim, &CheckBox);
        }
        else if (Equal(Collection->Type, "GridCoords"))
        {
            for (int j = 0; j < Collection->NumOfTokens; j++)
            {
                menu_token *Token = &Collection->Tokens[j];
                if (Equal(Token->ID,  "Y")) 
                    Y = atoi(Token->Value);
            }
        }
    }
}

// Returns true if events should be processed
internal bool
DoMenu(menu *Menu, const char *FileName, platform *p,  game_state *GameState, pair_int_string *IDs, int NumOfIDs)
{
    platform_keyboard_input *Keyboard = &p->Input.Keyboard;
    if (OnKeyDown(&Keyboard->F6))
        Toggle(&GameState->EditMenu);
    
    if (!Menu->Initialized)
    {
        
        MenuInit(Menu);
        ReadMenuFromFile(Menu, FileName, GameState, IDs, NumOfIDs);
        MenuSortActiveComponents(Menu);
    }
    else if (Menu->Reset) {
        PlatformSetCursorMode(&p->Input.Mouse, platform_cursor_mode::Arrow);
        MenuReset(Menu);
        //MenuSortActiveComponents(Menu);
    }
    
    if (GameState->EditMenu.Value)
    {
        font_string EditMenu = {};
        const char* EM = "Edit Menu";
        FontStringInit(&EditMenu, GetFont(&GameState->Assets, GAFI_Rubik), EM, 50, 0xFFFFFFFF);
        v2 SDim = FontStringGetDim(&EditMenu);
        FontStringPrint(&EditMenu, v2((p->Dimension.Width/2)-(int)SDim.x-10, -p->Dimension.Height/2 + 10));
        
        //*Menu = {};
        MenuInit(Menu);
        ReadMenuFromFile(Menu, FileName, GameState, IDs, NumOfIDs);
        
        PaddingResizeMenu(Menu);
        UpdateMenu(Menu);
        return false;
    }
    else
    {
        HandleMenuEvents(Menu, &p->Input);
        
        if (Menu->ScreenDim != GetDim(p)) 
            UpdateMenu(Menu, GetDim(p));
        
        return true;
    }
}