#include "ivy/tilemap/tilemap.h"
#include "ivy/utils.h"
#include <string.h>

#include <stdlib.h>
#include <assert.h>


void TM_LoadHeader(FILE *file, Tilemap *tilemap)
{
    ReadExact(file, &tilemap->header, sizeof(TilemapHeader));
}

void TM_LoadTilesets(FILE *file, Tilemap *tilemap)
{
    const u32 count = tilemap->header.tilesetCount;

    tilemap->tilesets = malloc(sizeof(Tileset) * count);
    assert(tilemap->tilesets && "[ERROR] Failed to allocate tilesets!");

    char pathBuffer[MAX_PATH_LEN];

    for (u32 i = 0; i < count; i++)
    {
        Tileset *ts = &tilemap->tilesets[i];

        ReadExact(file, &ts->firstGid, sizeof(u32));
        ts->texturePath = ReadString(file);

        char nameBuffer[MAX_PATH_LEN];
        snprintf(nameBuffer, MAX_PATH_LEN, "%s", (const char *)ts->texturePath);
        char *dot = strrchr(nameBuffer, '.');
        if (dot) strcpy(dot, ".bin");

        snprintf(pathBuffer, MAX_PATH_LEN, "%s/%s", TILESET_ASSET_PATH, nameBuffer);
        ts->texture = LoadTextureFromImageBin(pathBuffer);
    }
}

void TM_LoadLayers(FILE *file, Tilemap *tilemap)
{
    const u32 count = tilemap->header.layerCount;

    tilemap->layers = malloc(sizeof(Layer) * count);
    assert(tilemap->layers && "[ERROR] Failed to allocate layers!");

    for (u32 i = 0; i < count; i++)
    {
        Layer *layer = &tilemap->layers[i];

        ReadExact(file, &layer->width,  sizeof(u32));
        ReadExact(file, &layer->height, sizeof(u32));

        const u32 cellCount = layer->width * layer->height;
        layer->data = malloc(sizeof(u32) * cellCount);
        assert(layer->data && "[ERROR] Failed to allocate layer data!");
        ReadExact(file, layer->data, sizeof(u32) * cellCount);
    }
}


void TM_LoadLookupTables(FILE *file, Tilemap *tilemap)
{
    u32 maxGid = 0;
    for (u32 l = 0; l < tilemap->header.layerCount; l++)
    {
        const Layer *layer = &tilemap->layers[l];
        const u32    total = layer->width * layer->height;
        for (u32 i = 0; i < total; i++) {
            if (layer->data[i] > maxGid)
                maxGid = layer->data[i];
        }
    }
    tilemap->maxGid = maxGid;

    const u32 tableSize = maxGid + 1;

    tilemap->tileTypeTable     = malloc(tableSize * sizeof(u8));
    tilemap->tilesetIndexTable = malloc(tableSize * sizeof(u8));
    assert(tilemap->tileTypeTable && tilemap->tilesetIndexTable &&
           "[ERROR] Failed to allocate lookup tables!");

    ReadExact(file, tilemap->tileTypeTable,     tableSize * sizeof(u8));
    ReadExact(file, tilemap->tilesetIndexTable, tableSize * sizeof(u8));
}

void TM_BuildDrawInfoTable(Tilemap *tilemap)
{
    const u32 tableSize = tilemap->maxGid + 1;

    tilemap->tileDrawInfoTable = calloc(tableSize, sizeof(TileDrawInfo));
    assert(tilemap->tileDrawInfoTable && "[ERROR] Failed to allocate draw info table!");

    for (u32 tsIdx = 0; tsIdx < tilemap->header.tilesetCount; tsIdx++)
    {
        const Tileset *ts          = &tilemap->tilesets[tsIdx];
        const u32      tilesPerRow = (u32)ts->texture.width  / tilemap->header.tileWidth;
        const u32      tilesPerCol = (u32)ts->texture.height / tilemap->header.tileHeight;
        const u32      tileCount   = tilesPerRow * tilesPerCol;

        for (u32 localId = 0; localId < tileCount; localId++)
        {
            const u32 gid = ts->firstGid + localId;
            if (gid > tilemap->maxGid) continue;

            tilemap->tileDrawInfoTable[gid] = (TileDrawInfo){
                .src = (Rectangle){
                    .x      = (float)(localId % tilesPerRow) * (float)tilemap->header.tileWidth,
                    .y      = (float)(localId / tilesPerRow) * (float)tilemap->header.tileHeight,
                    .width  = (float)tilemap->header.tileWidth,
                    .height = (float)tilemap->header.tileHeight,
                },
                .pos     = (Vector2){ 0 },
                .type    = (TileType)tilemap->tileTypeTable[gid],
                .tileset = ts,
            };
        }
    }
}

int TM_FindTilesetIndexByGid(const Tilemap *tilemap, const u32 gid)
{
    if (gid == 0 || gid > tilemap->maxGid) return -1;
    return tilemap->tilesetIndexTable[gid];
}

TileType TM_GetTileType(const Tilemap *tilemap,
                        const u32 layerIndex,
                        const u32 x, const u32 y)
{
    if (layerIndex >= tilemap->header.layerCount) return TILE_NONE;
    if (x          >= tilemap->header.width)      return TILE_NONE;
    if (y          >= tilemap->header.height)     return TILE_NONE;

    const u32 gid = tilemap->layers[layerIndex].data[y * tilemap->header.width + x];
    if (gid == 0 || gid > tilemap->maxGid)        return TILE_NONE;

    return (TileType)tilemap->tileTypeTable[gid];
}

TileDrawInfo GetTileDrawInfo(const Tilemap *tilemap,
                             const Layer   *layer,
                             const u32 x, const u32 y)
{
    const u32 gid = layer->data[y * layer->width + x];
    if (gid == 0 || gid > tilemap->maxGid) return (TileDrawInfo){0};

    TileDrawInfo info = tilemap->tileDrawInfoTable[gid];

    if (!info.tileset) return (TileDrawInfo){0};

    info.pos = (Vector2){
        .x = (float)x * (float)tilemap->header.tileWidth,
        .y = (float)y * (float)tilemap->header.tileHeight,
    };
    return info;
}

void DrawTileById(const Tilemap      *tilemap,
                  const TileDrawInfo *info,
                  const u32 x, const u32 y)
{
    switch (info->type)
    {
        case TILE_WALL:   TM_DrawTileWall  (tilemap, info->tileset, info->src, info->pos, x, y); break;
        case TILE_CARPET: TM_DrawTileCarpet(tilemap, info->tileset, info->src, info->pos, x, y); break;
        case TILE_TABLE:  TM_DrawTileTable (tilemap, info->tileset, info->src, info->pos, x, y); break;
        case TILE_BORDER: TM_DrawTileBorder(tilemap, info->tileset, info->src, info->pos, x, y); break;
        default:          DrawTextureRec(info->tileset->texture, info->src, info->pos, WHITE);   break;
    }
}

void DrawNonBorderTiles(const Tilemap *tilemap)
{
    for (u32 l = 0; l < tilemap->header.layerCount; l++)
    {
        const Layer *layer = &tilemap->layers[l];
        for (u32 y = 0; y < layer->height; y++) {
            for (u32 x = 0; x < layer->width; x++) {
                TileDrawInfo info = GetTileDrawInfo(tilemap, layer, x, y);
                if (info.type == TILE_NONE || info.type == TILE_BORDER) continue;
                DrawTileById(tilemap, &info, x, y);
            }
        }
    }
}

void DrawBorderTiles(const Tilemap *tilemap)
{
    for (u32 l = 0; l < tilemap->header.layerCount; l++)
    {
        const Layer *layer = &tilemap->layers[l];
        for (u32 y = 0; y < layer->height; y++) {
            for (u32 x = 0; x < layer->width; x++) {
                TileDrawInfo info = GetTileDrawInfo(tilemap, layer, x, y);
                if (info.type != TILE_BORDER) continue;
                DrawTileById(tilemap, &info, x, y);
            }
        }
    }
}

void TM_DrawOnCanva(const Tilemap *tilemap)
{
    DrawNonBorderTiles(tilemap);
    DrawBorderTiles(tilemap);
}

void TM_ReloadCanva(Tilemap *tilemap)
{
    const TilemapHeader *h = &tilemap->header;
    tilemap->canva = LoadRenderTexture(
        (int)(h->width  * h->tileWidth),
        (int)(h->height * h->tileHeight)
    );
    BeginTextureMode(tilemap->canva);
        ClearBackground(BLANK);
        TM_DrawOnCanva(tilemap);
    EndTextureMode();
}