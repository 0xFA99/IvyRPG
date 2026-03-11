#include "ivy/tilemap/tilemap.h"

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>


Tilemap *LoadTilemapById(const u32 id)
{
    char path[MAX_PATH_LEN] = {0};
    snprintf(path, MAX_PATH_LEN, "%s/map_%d.bin", TILEMAP_ASSET_PATH, id);

    FILE *file = fopen(path, "rb");
    assert(file && "[ERROR] Failed to open file!");

    Tilemap *tilemap = malloc(sizeof(Tilemap));
    assert(tilemap && "[ERROR] Failed to allocate memory tilemap!");

    TM_LoadHeader(file, tilemap);
    TM_LoadTilesets(file, tilemap);
    TM_LoadLayers(file, tilemap);
    TM_FindMaxGid(tilemap);
    TM_ReloadCanva(tilemap);

    fclose(file);
    return tilemap;
}

void DrawTilemapFromCanva(const Tilemap *tilemap)
{
    assert(tilemap && "[ERROR] Tilemap not found!");

    const float canvaW = (float)tilemap->canva.texture.width;
    const float canvaH = (float)tilemap->canva.texture.height;

    const Rectangle src = { 0.0f, 0.0f, canvaW, -canvaH };
    const Rectangle dst = { 0.0f, 0.0f, canvaW,  canvaH  };

    DrawTexturePro(tilemap->canva.texture, src, dst, (Vector2){0}, 0.0f, WHITE);
}

void UnloadTilemap(Tilemap *tilemap)
{
    if (!tilemap) return;

    UnloadRenderTexture(tilemap->canva);

    if (tilemap->tilesets) {
        for (u32 i = 0; i < tilemap->header.tilesetCount; i++) {
            UnloadTexture(tilemap->tilesets[i].texture);
            free(tilemap->tilesets[i].texturePath);
            free(tilemap->tilesets[i].properties);
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