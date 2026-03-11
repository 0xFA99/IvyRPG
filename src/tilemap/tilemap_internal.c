#include "ivy/tilemap/tilemap.h"
#include "ivy/utils.h"

#include <stdlib.h>
#include <assert.h>

#define EXTRA_GID 256

u32 TM_BuildTileTable(const Tilemap *tilemap)
{
    u32 max = 0;

    for (u32 l = 0; l < tilemap->header.layerCount; l++)
    {
        const u32 totalCells = tilemap->layers[l].width * tilemap->layers[l].height;
        for (u32 i = 0; i < totalCells; i++) {
            if (tilemap->layers[l].data[i] > max)
                max = tilemap->layers[l].data[i];
        }
    }

    return max;
}

void TM_FindMaxGid(Tilemap *tilemap)
{
    const u32 maxGid    = TM_BuildTileTable(tilemap);
    tilemap->maxGid     = maxGid + EXTRA_GID;

    tilemap->tileTypeTable     = calloc(tilemap->maxGid + 1, sizeof(u8));
    tilemap->tilesetIndexTable = calloc(tilemap->maxGid + 1, sizeof(u8));
    tilemap->tileDrawInfoTable = calloc(tilemap->maxGid + 1, sizeof(TileDrawInfo));

    assert(tilemap->tileTypeTable && tilemap->tilesetIndexTable && tilemap->tileDrawInfoTable);

    for (u32 tsIdx = 0; tsIdx < tilemap->header.tilesetCount; tsIdx++)
    {
        const Tileset *ts       = &tilemap->tilesets[tsIdx];
        const u32 tilesPerRow   = ts->texture.width  / tilemap->header.tileWidth;
        const u32 tilesPerCol   = ts->texture.height / tilemap->header.tileHeight;
        const u32 tileCount     = tilesPerRow * tilesPerCol;

        for (u32 localId = 0; localId < tileCount; localId++)
        {
            const u32 gid = ts->firstGid + localId;
            if (gid > tilemap->maxGid) continue;

            tilemap->tilesetIndexTable[gid] = (u8)tsIdx;

            TileType type = TILE_GROUND;
            for (u32 p = 0; p < ts->propertyCount; p++) {
                if (ts->properties[p].id == localId) {
                    type = ts->properties[p].type;
                    break;
                }
            }

            tilemap->tileTypeTable[gid] = (u8)type;

            tilemap->tileDrawInfoTable[gid] = (TileDrawInfo) {
                .src = (Rectangle) {
                    .x      = (float)(localId % tilesPerRow) * (float)tilemap->header.tileWidth,
                    .y      = (float)(localId / tilesPerRow) * (float)tilemap->header.tileHeight,
                    .width  = (float)tilemap->header.tileWidth,
                    .height = (float)tilemap->header.tileHeight
                },
                .pos    = (Vector2){ 0 },
                .type   = type,
                .tileset = ts
            };
        }
    }
}

int TM_FindTilesetIndexByGid(const Tilemap *tilemap, const u32 gid)
{
    if (gid == 0 || gid > tilemap->maxGid) return -1;
    return tilemap->tilesetIndexTable[gid];
}

TileType TM_GetTileType(const Tilemap *tilemap, const u32 layerIndex, const u32 x, const u32 y)
{
    if (layerIndex >= tilemap->header.layerCount)   return TILE_NONE;
    if (x >= tilemap->header.width)                 return TILE_NONE;
    if (y >= tilemap->header.height)                return TILE_NONE;

    const u32 gid = tilemap->layers[layerIndex].data[y * tilemap->header.width + x];
    if (gid == 0 || gid > tilemap->maxGid)          return TILE_NONE;

    return (TileType)tilemap->tileTypeTable[gid];
}

void TM_LoadHeader(FILE *file, Tilemap *tilemap)
{
    ReadExact(file, &tilemap->header, sizeof(TilemapHeader));
}

void TM_LoadTilesets(FILE *file, Tilemap *tilemap)
{
    tilemap->tilesets = malloc(sizeof(Tileset) * tilemap->header.tilesetCount);
    assert(tilemap->tilesets && "[ERROR] Failed to allocate memory tilesets!");

    char pathBuffer[MAX_PATH_LEN] = {0};

    for (u32 i = 0; i < tilemap->header.tilesetCount; i++)
    {
        Tileset *ts = &tilemap->tilesets[i];

        ReadExact(file, &ts->firstGid,      sizeof(u32));
        ReadExact(file, &ts->propertyCount, sizeof(u32));

        ts->texturePath = ReadString(file);

        ts->properties = malloc(ts->propertyCount * sizeof(TileProp));
        assert(ts->properties && "[ERROR] Failed to allocate memory tile properties!");

        ReadExact(file, ts->properties, sizeof(TileProp) * ts->propertyCount);

        snprintf(pathBuffer, MAX_PATH_LEN, "%s/%s", TILESET_ASSET_PATH, (const char *)ts->texturePath);
        ts->texture = LoadTextureFromBin(pathBuffer);
    }
}

void TM_LoadLayers(FILE *file, Tilemap *tilemap)
{
    tilemap->layers = malloc(tilemap->header.layerCount * sizeof(Layer));
    assert(tilemap->layers && "[ERROR] Failed to allocate memory for layers!");

    for (u32 i = 0; i < tilemap->header.layerCount; i++)
    {
        Layer *layer = &tilemap->layers[i];

        ReadExact(file, &layer->width,  sizeof(u32));
        ReadExact(file, &layer->height, sizeof(u32));

        const u32 cellCount = layer->width * layer->height;
        layer->data = malloc(sizeof(u32) * cellCount);
        assert(layer->data && "[ERROR] Failed to allocate memory for layer data!");

        ReadExact(file, layer->data, cellCount * sizeof(u32));
    }
}

void DrawTileById(const Tilemap *tilemap, const TileDrawInfo *info, const u32 x, const u32 y)
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

TileDrawInfo GetTileDrawInfo(const Tilemap *tilemap, const Layer *layer, const u32 x, const u32 y)
{
    const u32 gid = layer->data[y * layer->width + x];
    if (gid == 0 || gid > tilemap->maxGid) return (TileDrawInfo){0};

    TileDrawInfo info = tilemap->tileDrawInfoTable[gid];
    if (!info.tileset) return (TileDrawInfo){0};

    info.pos = (Vector2) {
        .x = (float)x * (float)tilemap->header.tileWidth,
        .y = (float)y * (float)tilemap->header.tileHeight
    };

    return info;
}

void TM_ReloadCanva(Tilemap *tilemap)
{
    const TilemapHeader *h = &tilemap->header;

    tilemap->canva = LoadRenderTexture(h->width * h->tileWidth, h->height * h->tileHeight);

    BeginTextureMode(tilemap->canva);
        ClearBackground(BLANK);
        TM_DrawOnCanva(tilemap);
    EndTextureMode();
}

void TM_DrawOnCanva(const Tilemap *tilemap)
{
    DrawNonBorderTiles(tilemap);
    DrawBorderTiles(tilemap);
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