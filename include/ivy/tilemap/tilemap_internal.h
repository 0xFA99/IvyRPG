#ifndef GAME_TILEMAP_INTERNAL_H
#define GAME_TILEMAP_INTERNAL_H

#include "raylib/raylib.h"
#include "ivy/types.h"

#include <stdio.h>

#define TILEMAP_ASSET_PATH  "assets/tilemaps"
#define TILESET_ASSET_PATH  "assets/tilesets"
#define MAX_PATH_LEN        256

#define IS_TILE_VALID(x, y, w, h) \
    ((u32)(x) >= 0 && (x) < (w) && (u32)(y) >= 0 && (y) < (h))

#define HAS_TILE(layer, x, y, width, height) \
    (IS_TILE_VALID((x), (y), (width), (height)) && \
     (layer)->data[(y) * (width) + (x)] != 0)

typedef struct Tilemap Tilemap;

typedef enum {
    TILE_NONE      = 0,
    TILE_GROUND    = 1,
    TILE_WALL      = 2,
    TILE_BORDER    = 3,
    TILE_COLLISION = 4,
    TILE_CARPET    = 5,
    TILE_TABLE     = 6,
} TileType;

typedef struct {
    Texture2D  texture;
    u8        *texturePath;
    u32        firstGid;
} Tileset;

typedef struct {
    u32 *data;
    u32  width;
    u32  height;
} Layer;

typedef struct {
    u32 width;
    u32 height;
    u32 tileWidth;
    u32 tileHeight;
    u32 tilesetCount;
    u32 layerCount;
    u32 mapId;
    u32 spawnPointX;
    u32 spawnPointY;
    u32 eventGotoMapId;
    u32 eventGotoTileX;
    u32 eventGotoTileY;
} TilemapHeader;

typedef struct {
    Rectangle      src;
    Vector2        pos;
    TileType       type;
    const Tileset *tileset;
} TileDrawInfo;


void TM_LoadHeader        (FILE *file, Tilemap *tilemap);
void TM_LoadTilesets      (FILE *file, Tilemap *tilemap);
void TM_LoadLayers        (FILE *file, Tilemap *tilemap);
void TM_LoadLookupTables  (FILE *file, Tilemap *tilemap);

void TM_BuildDrawInfoTable (Tilemap *tilemap);

int      TM_FindTilesetIndexByGid (const Tilemap *tilemap, u32 gid);
TileType TM_GetTileType           (const Tilemap *tilemap, u32 layerIndex, u32 x, u32 y);

TileDrawInfo GetTileDrawInfo    (const Tilemap *tilemap, const Layer *layer, u32 x, u32 y);
void         DrawTileById       (const Tilemap *tilemap, const TileDrawInfo *info, u32 x, u32 y);
void         DrawNonBorderTiles (const Tilemap *tilemap);
void         DrawBorderTiles    (const Tilemap *tilemap);
void         TM_DrawOnCanva     (const Tilemap *tilemap);
void         TM_ReloadCanva     (Tilemap *tilemap);

void TM_DrawTileWall   (const Tilemap *tilemap, const Tileset *tileset, Rectangle src, Vector2 pos, u32 x, u32 y);
void TM_DrawTileTable  (const Tilemap *tilemap, const Tileset *tileset, Rectangle src, Vector2 pos, u32 x, u32 y);
void TM_DrawTileBorder (const Tilemap *tilemap, const Tileset *tileset, Rectangle src, Vector2 pos, u32 x, u32 y);
void TM_DrawTileCarpet (const Tilemap *tilemap, const Tileset *tileset, Rectangle src, Vector2 pos, u32 x, u32 y);

#endif