#ifndef DEBUG_ASSET_H
#define DEBUG_ASSET_H

struct asset_vector
{
    real32 E[Tag_Count];
};

struct linked_list_node
{
    void *Data;
    linked_list_node *Next;
};

struct linked_list
{
    u32 NodeCount;
    linked_list_node *I;
    linked_list_node *Head;
};

internal void
LinkedListAddNode(linked_list *List, void *Data)
{
    linked_list_node *Node = (linked_list_node*)qalloc(sizeof(linked_list_node));
    Node->Data = Data;
    if (List->Head != 0) {
        linked_list_node *Last = List->Head;
        for (u32 i = 1; i < List->NodeCount; i++)
            Last = Last->Next;
        Last->Next = Node;
    }
    else {
        List->Head = Node;
    }
    
    List->NodeCount++;
}

internal void*
LinkedListIGetNext(linked_list *List)
{
    linked_list_node *Ret = List->I;
    if (List->I->Next != 0)
        List->I = List->I->Next;
    else
        List->I = List->Head;
    
    return Ret->Data;
    
}
inline void LinkedListIReset(linked_list *List) { List->I = List->Head; }

struct debug_asset_tag
{
    u32 ID;
    u32 Value;
};

enum struct asset_memory_type
{
    bitmap,
    font,
    sound,
    
    count
};

struct debug_asset
{
    asset_memory_type MemoryType;
    union
    {
        loaded_bitmap Bitmap;
        loaded_font Font;
        loaded_sound Sound;
    };
};

struct debug_asset_type
{
    uint32 FirstAssetIndex;
    uint32 OnePastLastAssetIndex;
};

struct debug_assets
{
    u32 TagCount;
    debug_asset_tag *Tags;
    
    u32 AssetCount;
    debug_asset *Assets;
    
    debug_asset_type AssetTypes[Asset_Count];
};

internal bitmap_id
GetBestMatchBitmap(debug_assets *Assets, asset_type_id TypeID, asset_vector *MatchVector, asset_vector *WeightVector)
{
    u32 Start = Assets->AssetTypes[TypeID].FirstAssetIndex;
    u32 BestIndex = 0;
    for (u32 i = 0; i < Assets->TagCount; i++)
    {
        if (Assets->AssetTypes[TypeID].FirstAssetIndex <= Assets->Tags[i].Value &&
            Assets->Tags[i].Value < Assets->AssetTypes[TypeID].OnePastLastAssetIndex) {
            //if (MatchVector->E[i] > 0.0f)
            BestIndex = Assets->Tags[i].Value;
        }
        
    }
    return bitmap_id(BestIndex);
}

internal bitmap_id
GetIndexBitmap(debug_assets *Assets, asset_type_id TypeID, u32 Index)
{
    return bitmap_id(Assets->AssetTypes[TypeID].FirstAssetIndex + Index);
}

inline bitmap_id
GetFirstBitmap(debug_assets *Assets, asset_type_id TypeID)
{
    //debug_asset *Asset = Assets->Assets + Assets->AssetTypes[TypeID].FirstAssetIndex;
    return bitmap_id(Assets->AssetTypes[TypeID].FirstAssetIndex);
}

inline loaded_bitmap* GetBitmap(debug_assets *Assets, bitmap_id ID)
{
    debug_asset *Asset = Assets->Assets + ID.id;
    return (loaded_bitmap*)&Asset->Bitmap;
}

inline loaded_sound* GetSound(debug_assets *Assets, sound_id ID)
{
    debug_asset *Asset = Assets->Assets + Assets->AssetTypes[ID.id].FirstAssetIndex;
    int i = 0;
    return (loaded_sound*)&Asset->Sound;
}

enum debug_builder_asset_type
{
    DBAT_Bitmap,
    DBAT_Font,
    DBAT_Sound,
    
    DBAT_Count
};
struct debug_builder_asset_tag
{
    u32 ID;
};
struct debug_builder_asset
{
    debug_asset Asset;
    asset_type_id Type;
    debug_builder_asset_type AssetType;
    linked_list Tags;
};
struct debug_builder_assets
{
    linked_list Assets;
    u32 TagCount;
};

internal void
BuilderAssetTag(linked_list *Tags, asset_tag_id Tag1)
{
    debug_builder_asset_tag *Tag = (debug_builder_asset_tag*)qalloc(sizeof(debug_builder_asset_tag));
    Tag->ID = Tag1;
    LinkedListAddNode(Tags, (void*)Tag);
}

internal debug_builder_asset*
BuilderAddBitmap(debug_builder_assets *Assets, const char *FileName, asset_type_id Type)
{
    debug_builder_asset *BuilderAsset = Qalloc(debug_builder_asset);
    BuilderAsset->Type = Type;
    BuilderAsset->Asset.Bitmap = LoadBitmap2(FileName);
    BuilderAsset->AssetType = DBAT_Bitmap;
    BuilderAsset->Asset.MemoryType = asset_memory_type::bitmap;
    LinkedListAddNode(&Assets->Assets, (void*)BuilderAsset);
    return BuilderAsset;
}

inline void BuilderAddBitmapTag(debug_builder_assets *Assets, 
                                const char *FileName, 
                                asset_type_id Type,
                                asset_tag_id Tag1)
{
    debug_builder_asset *BuilderAsset = BuilderAddBitmap(Assets, FileName, Type);
    BuilderAssetTag(&BuilderAsset->Tags, Tag1);
    Assets->TagCount++;
}

internal debug_builder_asset*
BuilderAddSound(debug_builder_assets *Assets, const char *FileName, asset_type_id Type)
{
    debug_builder_asset *BuilderAsset = Qalloc(debug_builder_asset);
    BuilderAsset->Type = Type;
    BuilderAsset->Asset.Sound = LoadWAV(FileName);
    BuilderAsset->AssetType = DBAT_Sound;
    BuilderAsset->Asset.MemoryType = asset_memory_type::sound;
    LinkedListAddNode(&Assets->Assets, (void*)BuilderAsset);
    return BuilderAsset;
}

internal void
BuilderMakeFile(debug_builder_assets *Assets)
{
    debug_assets FinalAssets = {};
    linked_list PrintList = {};
    
    // Order them by asset types and determine ranges of types
    for (int i = 0; i < Asset_Count; i++) {
        LinkedListIReset(&Assets->Assets);
        for (u32 j = 0; j < Assets->Assets.NodeCount; j++) {
            debug_builder_asset *BuilderAsset = (debug_builder_asset*)LinkedListIGetNext(&Assets->Assets);
            
            if ((int)BuilderAsset->Type == i) {
                if (FinalAssets.AssetTypes[i].FirstAssetIndex == 0) {
                    FinalAssets.AssetTypes[i].FirstAssetIndex = PrintList.NodeCount;
                    FinalAssets.AssetTypes[i].OnePastLastAssetIndex = PrintList.NodeCount + 1;
                }
                else
                    FinalAssets.AssetTypes[i].OnePastLastAssetIndex++;
                
                LinkedListAddNode(&PrintList, (void*)BuilderAsset);
            }
        }
    }
    
    FinalAssets.Assets = (debug_asset*)qalloc(sizeof(debug_asset) * Assets->Assets.NodeCount);
    FinalAssets.Tags = (debug_asset_tag*)qalloc(sizeof(debug_asset_tag) * Assets->TagCount);
    
    // Put all of the assets and tags into chunks of memory
    LinkedListIReset(&PrintList);
    for (u32 i = 0; i < PrintList.NodeCount; i++) {
        debug_builder_asset *BuilderAsset = (debug_builder_asset*)LinkedListIGetNext(&PrintList);
        memcpy(&FinalAssets.Assets[i], &BuilderAsset->Asset, sizeof(debug_asset));
        FinalAssets.AssetCount++;
        
        LinkedListIReset(&BuilderAsset->Tags);
        for (u32 j = 0; j < BuilderAsset->Tags.NodeCount; j++) {
            debug_builder_asset_tag *BuilderTag = (debug_builder_asset_tag*)LinkedListIGetNext(&BuilderAsset->Tags);
            debug_asset_tag *NextTag = &FinalAssets.Tags[FinalAssets.TagCount];
            NextTag->ID = BuilderTag->ID;
            NextTag->Value = i;
            FinalAssets.TagCount++;
        }
    }
    
    FILE *AssetFile = fopen("assets.ethan", "wb");
    
    fwrite(&FinalAssets.TagCount, sizeof(u32), 1, AssetFile);
    fwrite(FinalAssets.Tags, sizeof(debug_asset_tag) * FinalAssets.TagCount, 1, AssetFile);
    fwrite(&FinalAssets.AssetCount, sizeof(u32), 1, AssetFile);
    fwrite(FinalAssets.Assets, sizeof(debug_asset) * FinalAssets.AssetCount, 1, AssetFile);
    fwrite(FinalAssets.AssetTypes, sizeof(debug_asset_type) * Asset_Count, 1, AssetFile);
    
    // Large chunk of memory for each asset
    for (u32 i = 0; i < FinalAssets.AssetCount; i++) {
        if (FinalAssets.Assets[i].MemoryType == asset_memory_type::bitmap) {
            loaded_bitmap *Bitmap = (loaded_bitmap*)&FinalAssets.Assets[i].Bitmap;
            fwrite(Bitmap->Memory, Bitmap->Width * Bitmap->Height * Bitmap->Channels, 1, AssetFile);
        }
        else if (FinalAssets.Assets[i].MemoryType == asset_memory_type::sound) {
            loaded_sound *Sound = (loaded_sound*)&FinalAssets.Assets[i].Sound;
            fwrite(Sound->Samples[0], Sound->SampleCount, 1, AssetFile);
        }
    }
    fclose(AssetFile);
}

internal void
TextureInit(loaded_bitmap *Bitmap)
{
    Assert(Bitmap->Width != 0 && Bitmap->Height != 0);
    
    glGenTextures(1, &U32FromPointer(Bitmap->TextureHandle));
    glBindTexture(GL_TEXTURE_2D, (GLuint)U32FromPointer(Bitmap->TextureHandle));
    
    if (Bitmap->Channels == 3) {
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, Bitmap->Width, Bitmap->Height, 0, GL_RGB, GL_UNSIGNED_BYTE, Bitmap->Memory);
    }
    else if (Bitmap->Channels == 4)
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, Bitmap->Width, Bitmap->Height, 0, GL_RGBA, GL_UNSIGNED_BYTE, Bitmap->Memory);
    
    glGenerateMipmap(GL_TEXTURE_2D);
    
    // Tile
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

internal void
BuilderLoadFile(debug_assets *Assets)
{
    FILE *AssetFile = fopen("assets.ethan", "rb");
    
    fread(&Assets->TagCount, sizeof(u32), 1, AssetFile);
    Assets->Tags = (debug_asset_tag*)qalloc(sizeof(debug_asset_tag) * Assets->TagCount);
    fread(Assets->Tags, sizeof(debug_asset_tag) * Assets->TagCount, 1, AssetFile);
    
    fread(&Assets->AssetCount, sizeof(u32), 1, AssetFile);
    Assets->Assets = (debug_asset*)qalloc(sizeof(debug_asset) * Assets->AssetCount);
    fread(Assets->Assets, sizeof(debug_asset) * Assets->AssetCount, 1, AssetFile);
    
    fread(Assets->AssetTypes, sizeof(debug_asset_type) * Asset_Count, 1, AssetFile);
    
    for (u32 i = 0; i < Assets->AssetCount; i++) {
        if (Assets->Assets[i].MemoryType == asset_memory_type::bitmap) {
            loaded_bitmap *Bitmap = (loaded_bitmap*)&Assets->Assets[i].Bitmap;
            Bitmap->Memory = qalloc(Bitmap->Width * Bitmap->Height * Bitmap->Channels);
            fread(Bitmap->Memory, Bitmap->Width * Bitmap->Height * Bitmap->Channels, 1, AssetFile);
            TextureInit(Bitmap);
        }
        else if (Assets->Assets[i].MemoryType == asset_memory_type::sound) {
            loaded_sound *Sound = (loaded_sound*)&Assets->Assets[i].Sound;
            Sound->Samples[0] = (int16*)qalloc(Sound->SampleCount * 2);
            Sound->Samples[1] = Sound->Samples[0] + Sound->SampleCount;
            fread(Sound->Samples[0], Sound->SampleCount, 1, AssetFile);
        }
    }
    
    fclose(AssetFile);
}

#endif //DEBUG_ASSET_H
