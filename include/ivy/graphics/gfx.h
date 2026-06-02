#ifndef IVY_GRAPHICS_GFX_H
#define IVY_GRAPHICS_GFX_H

#include "ivy/core/types.h"
#include "ivy/core/virtual.h"
#include "ivy/scenes/types.h"
#include "ivy/utils/forward.h"

#include "raylib/raylib.h"

#ifdef __cplusplus
extern "C" {
#endif

Texture2D   Ivy_Gfx_LoadTextureDDS(IvyAssetManager *mgr, u32 id);
void        Ivy_Gfx_UnloadTexture(Texture2D *texture);

Font        Ivy_Gfx_LoadFont(IvyArenaLinear *restrict arena, IvyAssetManager *restrict mgr, u32 metaId, u32 atlasId, int fontSize);
void        Ivy_Gfx_UnloadFont(Font *font);

void        Ivy_Gfx_DrawLocaleText(Font font, const char *text, u32 len, Vector2 position, float fontSize, float spacing, Color tint);
Vector2     Ivy_Gfx_GetScreenPos(const IvyVirtualScreen *vr, Vector2 vp);

IVY_INLINE void Ivy_Gfx_VirtualDraw(const IvyVirtualScreen *vr)
{
    IVY_ASSERT(vr, "[Gfx](IvyVirtualScreen) Instance is NULL");
    DrawTexturePro( vr->target.texture, vr->source, vr->destination, (Vector2){ 0, 0 }, 0.0f, WHITE);
}

#ifdef __cplusplus
}
#endif

#endif