#ifndef IVY_SYSTEMS_TEXTURE_MANAGER_H
#define IVY_SYSTEMS_TEXTURE_MANAGER_H

#include "ivy/core/types.h"
#include "ivy/utils/forward.h"

#include "raylib/raylib.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
   IVY_TEX_NONE = 0,

   IVY_TEX_MENU_BACKGROUND,
   IVY_TEX_CURSOR_1,
   IVY_TEX_CURSOR_2,
   IVY_TEX_ICONS_ATLAS,

   IVY_TEX_PLAYER_BASE_BODY,
   IVY_TEX_PLAYER_BASE_HEAD,
   IVY_TEX_PLAYER_BASE_HAIR,

   IVY_TEX_COUNT
} IvyTextureID;

struct IvyTextureManager {
   Texture2D textures[IVY_TEX_COUNT];
};

IvyTextureManager *Ivy_TextureManager_Init(IvyArenaLinear *restrict arena, IvyAssetManager *restrict assetManager);

Texture2D   Ivy_TextureManager_Get(const IvyTextureManager *texManager, IvyTextureID id);
void        Ivy_TextureManager_LoadDynamic(IvyTextureManager *restrict texManager, IvyAssetManager *restrict assetManager, IvyTextureID slotID, u32 texID);
void        Ivy_TextureManager_Destroy(IvyTextureManager *texManager);

#ifdef __cplusplus
}
#endif

#endif