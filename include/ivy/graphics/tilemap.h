#ifndef IVY_GRAPHICS_TILEMAP_H
#define IVY_GRAPHICS_TILEMAP_H

#include "ivy/core/types.h"
#include "ivy/utils/forward.h"

#include "raylib/raylib.h"

#ifdef __cplusplus
extern "C" {
#endif

struct IvyTilemap {
    Texture2D tilesetTex;   // 20
    Shader shader;          // 16

    u32 vaoId;              // 4
    u32 vboId;              // 4
    int vertexCount;        // 4
    int colDiffuseLoc;      // 4
    int padding;            // 4
};                          // 64

IvyTilemap *Ivy_Tilemap_LoadMap(IvyAssetManager *restrict manager, IvyArenaLinear *restrict arena, u32 metadata, u32 vertex);
void        Ivy_Tilemap_Render(const IvyTilemap *tilemap);
void        Ivy_Tilemap_Unload(const IvyTilemap *tilemap);

#ifdef __cplusplus
}
#endif

#endif