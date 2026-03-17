#include "ivy/collision.h"
#include "ivy/tilemap/tilemap_internal.h"

#include <stdlib.h>

#include "ivy/game.h"


typedef struct {
    int x, y, w, h;
} RectInfo;

#define MAX_COLLISION_RECTS 512

static bool IsSolidTile(const Tilemap *tilemap, const int layerIndex,
                         const int x, const int y)
{
    const Layer *layer = &tilemap->layers[layerIndex];
    if (x < 0 || x >= (int)layer->width)  return false;
    if (y < 0 || y >= (int)layer->height) return false;
    if (layer->data[y * layer->width + x] == 0) return false;

    const TileType type = TM_GetTileType(tilemap, (u32)layerIndex, (u32)x, (u32)y);
    return type == TILE_BORDER   ||
           type == TILE_WALL     ||
           type == TILE_COLLISION ||
           type == TILE_TABLE;
}

static int ComputeCollisionRects(const Tilemap *tilemap, const int layerIndex,
                                  RectInfo *outBuf, const int maxRects)
{
    const Layer *layer = &tilemap->layers[layerIndex];
    const int    w     = (int)layer->width;
    const int    h     = (int)layer->height;

    bool *visited = calloc((size_t)(w * h), sizeof(bool));
    IVY_ASSERT(visited, "[Collusion] Failed to allocate visited buffer!");

    int rectCount = 0;

    for (int y = 0; y < h; y++)
    {
        for (int x = 0; x < w; x++)
        {
            if (visited[y * w + x] || !IsSolidTile(tilemap, layerIndex, x, y))
                continue;

            int rw = 1;
            while (x + rw < w &&
                   !visited[y * w + (x + rw)] &&
                   IsSolidTile(tilemap, layerIndex, x + rw, y))
            {
                rw++;
            }

            int rh = 1;
            while (y + rh < h)
            {
                bool canExpand = true;
                for (int col = 0; col < rw; col++) {
                    if (visited[(y + rh) * w + (x + col)] ||
                        !IsSolidTile(tilemap, layerIndex, x + col, y + rh))
                    {
                        canExpand = false;
                        break;
                    }
                }
                if (!canExpand) break;
                rh++;
            }

            for (int row = 0; row < rh; row++)
                for (int col = 0; col < rw; col++)
                    visited[(y + row) * w + (x + col)] = true;

            if (rectCount < maxRects)
                outBuf[rectCount++] = (RectInfo){ x, y, rw, rh };
        }
    }

    free(visited);
    return rectCount;
}

Collision *InitCollisionAllLayers(const Tilemap *tilemap)
{
    IVY_ASSERT(tilemap, "[Collision] Cannot initialize from a NULL tilemap.");

    RectInfo *tmpRects = malloc(MAX_COLLISION_RECTS * sizeof(RectInfo));
    IVY_ASSERT(tmpRects, "[RectInfo] Failed to allocate temporary buffer.");

    int totalCount = 0;

    for (int l = 0; l < (int)tilemap->header.layerCount; l++)
    {
        const int remaining = MAX_COLLISION_RECTS - totalCount;
        if (remaining <= 0) break;

        const int layerCount = ComputeCollisionRects(
            tilemap, l,
            tmpRects + totalCount,
            remaining
        );
        totalCount += layerCount;
    }

    Collision *collision = malloc(sizeof(Collision));
    IVY_ASSERT(collision, "[Collision] Failed to allocate memory for the Collision header.");

    collision->rect      = NULL;
    collision->rectCount = 0;

    if (totalCount == 0) {
        free(tmpRects);
        return collision;
    }

    const float tw = (float)tilemap->header.tileWidth;
    const float th = (float)tilemap->header.tileHeight;

    collision->rect = malloc((size_t)totalCount * sizeof(Rectangle));
    IVY_ASSERT(collision->rect, "[Collision] Failed to allocate final rectangle array.");

    for (int i = 0; i < totalCount; i++) {
        collision->rect[i] = (Rectangle){
            .x      = (float)tmpRects[i].x * tw,
            .y      = (float)tmpRects[i].y * th,
            .width  = (float)tmpRects[i].w * tw,
            .height = (float)tmpRects[i].h * th,
        };
    }

    collision->rectCount = (u32)totalCount;

    free(tmpRects);
    return collision;
}

void DestroyCollision(Collision *collision)
{
    if (!collision) return;
    free(collision->rect);
    free(collision);
}