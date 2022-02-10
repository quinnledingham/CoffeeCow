internal void 
RenderRect(Rect *S, int fill, uint32 color)
{
    win32_offscreen_buffer *Buffer = &GlobalBackbuffer;
    
    uint8 *EndOfBuffer = (uint8 *)Buffer->Memory + Buffer->Pitch*Buffer->Height;
    uint32 Color = color;
    
    for(int X = S->x;
        X < (S->x + S->width);
        ++X)
    {
        uint8 *Pixel = ((uint8 *)Buffer->Memory +
                        X*Buffer->BytesPerPixel +
                        S->y*Buffer->Pitch);
        
        for(int Y = S->y;
            Y < (S->y + S->height);
            ++Y)
        {
            // Check if the pixel exists
            if((Pixel >= Buffer->Memory) &&
               ((Pixel + 4) <= EndOfBuffer))
            {
                if (fill == FILL)
                {
                    *(uint32 *)Pixel = Color;
                }
                else if (fill == NOFILL)
                {
                    // Only draw border
                    if ((X == S->x) ||
                        (Y == S->y) ||
                        (X == (S->x + S->width) - 1) ||
                        (Y == (S->y + S->height) - 1))
                    {
                        *(uint32 *)Pixel = Color;
                    }
                }
            }
            
            Pixel += Buffer->Pitch;
        }
    }
}

internal void
RenderCircle(Circle *C, int Fill, uint32 Color)
{
    win32_offscreen_buffer *Buffer = &GlobalBackbuffer;
    
    uint8 *EndOfBuffer = (uint8 *)Buffer->Memory + Buffer->Pitch*Buffer->Height;
    //uint32 Color = Color;
    
    for(int X = C->X;
        X < (C->X + (C->Radius * 2));
        ++X)
    {
        uint8 *Pixel = ((uint8 *)Buffer->Memory +
                        X*Buffer->BytesPerPixel +
                        C->Y*Buffer->Pitch);
        
        for(int Y = C->Y;
            Y < (C->Y + (C->Radius * 2));
            ++Y)
        {
            // Check if the pixel exists
            if((Pixel >= Buffer->Memory) &&
               ((Pixel + 4) <= EndOfBuffer))
            {
                int32 CRX = X - (C->X + C->Radius);
                int32 CRY = Y - (C->Y + C->Radius);
                int32 CurrentRadius = (CRX * CRX) + (CRY * CRY);
                double DCurrentRadius = sqrt((double)CurrentRadius);
                if (Fill == FILL)
                {
                    if (DCurrentRadius <= C->Radius)
                    {
                        *(uint32 *)Pixel = Color;
                    }
                }
                else if (Fill == NOFILL)
                {
                    // Only draw border
                    if (DCurrentRadius == C->Radius)
                    {
                        *(uint32 *)Pixel = Color;
                    }
                }
            }
            
            Pixel += Buffer->Pitch;
        }
    }
}

internal void
RenderBitmap(loaded_bitmap *Bitmap, real32 RealX, real32 RealY)
{
    win32_offscreen_buffer *Buffer = &GlobalBackbuffer;
    
    int32 MinX = RoundReal32ToInt32(RealX);
    int32 MinY = RoundReal32ToInt32(RealY);
    int32 MaxX = MinX + Bitmap->Width;
    int32 MaxY = MinY + Bitmap->Height;
    
    if(MinX < 0)
    {
        MinX = 0;
    }
    
    if(MinY < 0)
    {
        MinY = 0;
    }
    
    if(MaxX > Buffer->Width)
    {
        MaxX = Buffer->Width;
    }
    
    if(MaxY > Buffer->Height)
    {
        MaxY = Buffer->Height;
    }
    
    uint32 *SourceRow = (uint32*)Bitmap->Memory + Bitmap->Width * (Bitmap->Height - 1);
    uint8 *DestRow = ((uint8*)Buffer->Memory +
                      MinX*Buffer->BytesPerPixel +
                      MinY*Buffer->Pitch);
    
    for(int Y = MinY; Y < MaxY; ++Y)
    {
        uint32 *Dest = (uint32*)DestRow;
        uint32 *Source = SourceRow;
        
        for(int X = MinX; X < MaxX; ++X)
        {
            real32 A = (real32)((*Source >> 24) & 0xFF) / 255.0f;
            real32 SR = (real32)((*Source >> 16) & 0xFF);
            real32 SG = (real32)((*Source >> 8) & 0xFF);
            real32 SB = (real32)((*Source >> 0) & 0xFF);
            
            real32 DR = (real32)((*Dest >> 16) & 0xFF);
            real32 DG = (real32)((*Dest >> 8) & 0xFF);
            real32 DB = (real32)((*Dest >> 0) & 0xFF);
            
            real32 R = (1.0f-A)*DR + A*SR;
            real32 G = (1.0f-A)*DG + A*SG;
            real32 B = (1.0f-A)*DB + A*SB;
            
            *Dest = (((uint32)(R + 0.5f) << 16) |
                     ((uint32)(G + 0.5f) << 8) |
                     ((uint32)(B + 0.5f) << 0));
            
            ++Dest;
            ++Source;
        }
        
        DestRow += Buffer->Pitch;
        SourceRow -= Bitmap->Width;
    }
}

internal void 
RenderRectImage(Rect *S, Image *image)
{
    win32_offscreen_buffer *Buffer = &GlobalBackbuffer;
    
    Image re = {};
    re.data = image->data;
    re.x = S->width;
    re.y = S->height;
    re.n = image->n;
    //RenderImage(Buffer, &re);
    
    
    uint8 *EndOfBuffer = (uint8 *)Buffer->Memory + Buffer->Pitch*Buffer->Height;
    
    for(int X = S->x; X < (S->x + S->width); ++X)
    {
        uint8 *Pixel = ((uint8 *)Buffer->Memory + X * Buffer->BytesPerPixel +S->y*Buffer->Pitch);
        uint8 *Color = ((uint8 *)re.data + (X - S->x) * re.n);
        
        for(int Y = S->y; Y < (S->y + S->height); ++Y)
        {
            // Check if the pixel exists
            if((Pixel >= Buffer->Memory) && ((Pixel + 4) <= EndOfBuffer))
            {
                uint32 c = *Color;
                
                int r = *Color++;
                int g = *Color++;
                int b = *Color;
                Color--;
                Color--;
                
                c = createRGB(r, g, b);
                *(uint32 *)Pixel =c;
                Color += (re.n * re.x);
            }
            Pixel += Buffer->Pitch;
        }
    }
    
}

internal void
RenderImage(Image *image)
{
    win32_offscreen_buffer *Buffer = &GlobalBackbuffer;
    
    int xt = 50;
    for(int X = 0; X < image->x; ++X)
    {
        uint8 *Pixel = ((uint8 *)Buffer->Memory + X * Buffer->BytesPerPixel);
        uint8 *Color = ((uint8 *)image->data + X * image->n);
        
        for(int Y = 0; Y < image->y; ++Y)
        {
            uint32 c = *Color;
            
            int r = *Color++;
            int g = *Color++;
            int b = *Color;
            Color--;
            Color--;
            
            c = createRGB(r, g, b);
            *(uint32 *)Pixel =c;
            Pixel += Buffer->Pitch;
            Color += (image->n * image->x);
        }
    }
}

internal void
RenderBackgroundGrid(int GridX, int GridY, int GridWidth, int GridHeight, int GridSize, Image *image)
{
    win32_offscreen_buffer *Buffer = &GlobalBackbuffer;
    
    for (int i = 0;
         i < GridWidth;
         i++)
    {
        for (int j = 0;
             j < GridHeight;
             j++)
        {
            Rect newRect = {GridX + (i * GridSize), GridY + (j * GridSize), GridSize, GridSize};
            RenderRect(&newRect, NOFILL, 0xFF000000);
            RenderRectImage(&newRect, image);
        }
    }
}

// Paints the screen white
internal void
ClearScreen()
{
    win32_offscreen_buffer *Buffer = &GlobalBackbuffer;
    memset(Buffer->Memory, 0xFF, (Buffer->Width * Buffer->Height) * Buffer->BytesPerPixel);
}

internal PrintOnScreenReturn
PrintOnScreen(Font* SrcFont, char* SrcText, int InputX, int InputY, uint32 Color)
{
    win32_offscreen_buffer *Buffer = &GlobalBackbuffer;
    
    int X = InputX;
    int StrLength = StringLength(SrcText);
    int BiggestY = 0;
    
    for (int i = 0; i < StrLength; i++)
    {
        int SrcChar = SrcText[i];
        SrcFont->Memory[SrcChar].Advance = 0;
        
        int Y = -1 *  SrcFont->Memory[SrcChar].C_Y1;
        if(BiggestY < Y)
        {
            BiggestY = Y;
        }
        
        // advance x 
        SrcFont->Memory[SrcChar].Advance += (int)roundf(SrcFont->Memory[SrcChar].AX * SrcFont->Scale);
        
        // add kerning
        int kern;
        kern = stbtt_GetCodepointKernAdvance(&SrcFont->Info, SrcText[i], SrcText[i + 1]);
        SrcFont->Memory[SrcChar].Advance += (int)roundf(kern * SrcFont->Scale);
        
        X += SrcFont->Memory[SrcChar].Advance;
    }
    
    int StringWidth = (X - InputX);
    X = InputX;
    
    PrintOnScreenReturn R = {};
    R.Height = BiggestY;
    R.Width = StringWidth;;
    
    for (int i = 0; i < StrLength; i++)
    {
        int SrcChar = SrcText[i];
        
        int Y = InputY + SrcFont->Memory[SrcChar].C_Y1 + BiggestY;
        
        loaded_bitmap SrcBitmap = {};
        SrcBitmap.Width = SrcFont->Memory[SrcChar].Width;
        SrcBitmap.Height = SrcFont->Memory[SrcChar].Height;
        SrcBitmap.Pitch = SrcFont->Memory[SrcChar].Pitch;
        SrcBitmap.Memory = SrcFont->Memory[SrcChar].Memory;
        
        ChangeBitmapColor(SrcBitmap, Color);
        RenderBitmap(&SrcBitmap, (real32)X, (real32)Y);
        
        X += SrcFont->Memory[SrcChar].Advance;
        
    }
    
    return R;
}