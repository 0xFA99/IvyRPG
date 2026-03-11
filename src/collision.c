#include "ivy/collision.h"
#include "ivy/tilemap/tilemap_internal.h"

#include <assert.h>
#include <stdlib.h>


static bool
IsSolidTile(const Tilemap *tilemap, const int layerIndex, const int x, const int y)
{
    const Layer *layer = &tilemap->layers[layerIndex];
    if (x < 0 || x >= (int)layer->width || y < 0 || y >= (int)layer->height) return false;
    if (layer->data[y * layer->width + x] == 0) return false;

    const TileType type = TM_GetTileType(tilemap, (u32)layerIndex, (u32)x, (u32)y);

    return type == TILE_BORDER    ||
           type == TILE_WALL      ||
           type == TILE_COLLISION ||
           type == TILE_TABLE;
}

static void
ComputeCollisionRects(const Tilemap *tilemap, const int layerIndex,
                      RectInfo **outRects, int *outCount, int *outCapacity)
{
    const Layer *layer  = &tilemap->layers[layerIndex];
    bool *visited       = calloc(layer->width * layer->height, sizeof(bool));
    assert(visited);

    int rectCount    = 0;
    int rectCapacity = 0;
    RectInfo *rects  = NULL;

    for (int y = 0; y < (int)layer->height; y++)
    {
        for (int x = 0; x < (int)layer->width; x++)
        {
            if (!IsSolidTile(tilemap, layerIndex, x, y) || visited[y * layer->width + x])
                continue;

            // Expand horizontal
            int w = 1;
            while (x + w < (int)layer->width &&
                   IsSolidTile(tilemap, layerIndex, x + w, y) &&
                   !visited[y * layer->width + (x + w)])
                w++;

            // Expand vertical
            int h = 1;
            while (y + h < (int)layer->height)
            {
                bool canExpand = true;
                for (int i = 0; i < w; i++) {
                    if (!IsSolidTile(tilemap, layerIndex, x + i, y + h) ||
                        visited[(y + h) * layer->width + (x + i)]) {
                        canExpand = false;
                        break;
                    }
                }
                if (!canExpand) break;
                h++;
            }

            // Mark visited
            for (int row = 0; row < h; row++)
                for (int col = 0; col < w; col++)
                    visited[(y + row) * layer->width + (x + col)] = true;

            if (rectCount >= rectCapacity) {
                rectCapacity = rectCapacity == 0 ? 16 : rectCapacity * 2;
                RectInfo *tmp = realloc(rects, rectCapacity * sizeof(RectInfo));
                assert(tmp && "[ERROR] Failed to realloc collision rects");
                rects = tmp;
            }

            rects[rectCount++] = (RectInfo){ x, y, w, h };
        }
    }

    free(visited);
    *outRects    = rects;
    *outCount    = rectCount;
    *outCapacity = rectCapacity;
}

Collision *InitCollisionAllLayers(const Tilemap *tilemap)
{
    assert(tilemap && "[ERROR] Tilemap is NULL");

    Collision *collision  = malloc(sizeof(Collision));
    assert(collision);
    collision->rect      = NULL;
    collision->rectCount = 0;

    RectInfo *allRects  = NULL;
    int allCount        = 0;
    int allCapacity     = 0;

    for (int l = 0; l < (int)tilemap->header.layerCount; l++)
    {
        RectInfo *layerRects = NULL;
        int layerCount       = 0;
        int layerCapacity    = 0;

        ComputeCollisionRects(tilemap, l, &layerRects, &layerCount, &layerCapacity);

        for (int i = 0; i < layerCount; i++)
        {
            if (allCount >= allCapacity) {
                allCapacity = allCapacity == 0 ? 16 : allCapacity * 2;
                RectInfo *tmp = realloc(allRects, allCapacity * sizeof(RectInfo));
                assert(tmp && "[ERROR] Failed to realloc allRects");
                allRects = tmp;
            }
            allRects[allCount++] = layerRects[i];
        }

        free(layerRects);
    }

    if (allCount == 0) {
        free(allRects);
        return collision;
    }

    const float tw = (float)tilemap->header.tileWidth;
    const float th = (float)tilemap->header.tileHeight;

    collision->rect = malloc(allCount * sizeof(Rectangle));
    assert(collision->rect && "[ERROR] Failed to alloc collision rects");

    for (int i = 0; i < allCount; i++) {
        collision->rect[i] = (Rectangle){
            .x      = (float)allRects[i].x * tw,
            .y      = (float)allRects[i].y * th,
            .width  = (float)allRects[i].w * tw,
            .height = (float)allRects[i].h * th
        };
    }

    collision->rectCount = (u32)allCount;

    free(allRects);
    return collision;
}

void DestroyCollision(Collision *collision)
{
    if (!collision) return;
    free(collision->rect);
    free(collision);
}