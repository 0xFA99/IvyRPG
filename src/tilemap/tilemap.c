#include "ivy/tilemap/tilemap.h"
#include "ivy/game.h"

#include <stdio.h>
#include <stdlib.h>


Tilemap *LoadTilemapById(const u32 id)
{
    char path[MAX_PATH_LEN] = {0};
    snprintf(path, MAX_PATH_LEN, "%s/map_%u.bin", TILEMAP_ASSET_PATH, id);

    FILE *file = fopen(path, "rb");
    IVY_ASSERT(file, "Failed to open tilemap file");

    Tilemap *tilemap = malloc(sizeof(Tilemap));
    IVY_ASSERT(tilemap, "Failed to allocate tilemap");

    TM_LoadHeader       (file, tilemap);
    TM_LoadTilesets     (file, tilemap);
    TM_LoadLayers       (file, tilemap);
    TM_LoadLookupTables (file, tilemap);

    fclose(file);

    TM_BuildDrawInfoTable(tilemap);
    TM_ReloadCanva(tilemap);

    return tilemap;
}

void DrawTilemapFromCanva(const Tilemap *tilemap)
{
    IVY_ASSERT(tilemap, "Tilemap is NULL");

    const float w = (float)tilemap->canva.texture.width;
    const float h = (float)tilemap->canva.texture.height;

    const Rectangle src = { 0.0f, 0.0f,  w, -h };
    const Rectangle dst = { 0.0f, 0.0f,  w,  h };

    DrawTexturePro(tilemap->canva.texture, src, dst, (Vector2){ 0 }, 0.0f, WHITE);
}

void UnloadTilemap(Tilemap *tilemap)
{
    if (!tilemap) return;

    UnloadRenderTexture(tilemap->canva);

    if (tilemap->tilesets) {
        for (u32 i = 0; i < tilemap->header.tilesetCount; i++) {
            UnloadTexture(tilemap->tilesets[i].texture);
            free(tilemap->tilesets[i].texturePath);
        }
        free(tilemap->tilesets);
    }

    if (tilemap->layers) {
        for (u32 i = 0; i < tilemap->header.layerCount; i++) {
            free(tilemap->layers[i].data);
        }
        free(tilemap->layers);
    }

    free(tilemap->tileDrawInfoTable);
    free(tilemap->tilesetIndexTable);
    free(tilemap->tileTypeTable);

    free(tilemap);
}