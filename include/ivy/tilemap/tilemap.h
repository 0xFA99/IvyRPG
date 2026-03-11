#ifndef IVY_TILEMAP_H
#define IVY_TILEMAP_H

#include "ivy/types.h"
#include "ivy/tilemap/tilemap_internal.h"


struct Tilemap {
    TilemapHeader   header;
    Layer           *layers;
    Tileset         *tilesets;

    RenderTexture2D canva;

    u8              *tileTypeTable;
    u8              *tilesetIndexTable;
    TileDrawInfo    *tileDrawInfoTable;
    u32             maxGid;
};


Tilemap    *LoadTilemapById(u32 id);
void        DrawTilemapFromCanva(const Tilemap *tilemap);
void        UnloadTilemap(Tilemap *tilemap);


#endif