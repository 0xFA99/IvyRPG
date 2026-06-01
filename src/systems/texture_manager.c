#include "ivy/core/types.h"
#include "ivy/systems/texture_manager.h"
#include "ivy/graphics/gfx.h"
#include "ivy/utils/file_ids.h"
#include "ivy/arena/linear.h"

#include "raylib/rlgl.h"

static IvyTextureID GetIndexFromHash(const u32 hash)
{
    switch (hash)
    {
        case ASSET_TEXTURES_BACKGROUND_DDS:     return IVY_TEX_MENU_BACKGROUND;
        case ASSET_TEXTURES_CURSOR_WHITE_DDS:   return IVY_TEX_CURSOR_1;
        case ASSET_TEXTURES_CURSOR_YELLOW_DDS:  return IVY_TEX_CURSOR_2;
        case ASSET_TEXTURES_ICONS_DDS:          return IVY_TEX_ICONS_ATLAS;
        default:                                return IVY_TEX_NONE;
    }
}

IvyTextureManager *Ivy_TextureManager_Init(IvyArenaLinear *restrict arena, IvyAssetManager *restrict assetManager)
{
    IVY_ASSERT(arena != NULL, "[IvyTextureManager] IvyArenaLinear is NULL!");
    IVY_ASSERT(assetManager != NULL, "[IvyTextureManager] IvyAssetManager is NULL!");

    IvyTextureManager *textureManager = Ivy_Arena_LinearAllocZero(arena, sizeof(IvyTextureManager));

    textureManager->textures[IVY_TEX_MENU_BACKGROUND] = Ivy_Gfx_LoadTextureDDS(assetManager, ASSET_TEXTURES_BACKGROUND_DDS);
    textureManager->textures[IVY_TEX_CURSOR_1]        = Ivy_Gfx_LoadTextureDDS(assetManager, ASSET_TEXTURES_CURSOR_WHITE_DDS);
    textureManager->textures[IVY_TEX_CURSOR_2]        = Ivy_Gfx_LoadTextureDDS(assetManager, ASSET_TEXTURES_CURSOR_YELLOW_DDS);
    textureManager->textures[IVY_TEX_ICONS_ATLAS]     = Ivy_Gfx_LoadTextureDDS(assetManager, ASSET_TEXTURES_ICONS_DDS);

    return textureManager;
}

Texture2D Ivy_TextureManager_Get(const IvyTextureManager *texManager, const IvyTextureID id)
{
    const IvyTextureID texIndex = GetIndexFromHash(id);

    return texManager->textures[texIndex];
}

void Ivy_TextureManager_LoadDynamic(IvyTextureManager *restrict texManager, IvyAssetManager *restrict assetManager, const IvyTextureID slotID, const u32 texID)
{
    IVY_ASSERT(texManager != NULL, "[IvyTextureManager] Manager is NULL!");

    if (texManager->textures[slotID].id >0) {
        rlUnloadTexture(texManager->textures[slotID].id);
        texManager->textures[slotID].id = 0;
    }

    texManager->textures[slotID] = Ivy_Gfx_LoadTextureDDS(assetManager, texID);
}

void Ivy_TextureManager_Destroy(IvyTextureManager *texManager)
{
    if (texManager == NULL) return;

    for (usize i = 0; i < IVY_TEX_COUNT; i++)
    {
        if (texManager->textures[i].id > 0) {
            rlUnloadTexture(texManager->textures[i].id);
            texManager->textures[i].id = 0;
        }
    }
}
