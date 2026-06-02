#include "ivy/arena/linear.h"
#include "ivy/core/types.h"
#include "ivy/entities/player.h"
#include "ivy/graphics/collusion.h"
#include "ivy/scenes/title.h"
#include "ivy/systems/asset_manager.h"
#include "ivy/utils/file_ids.h"
#include "ivy/utils/forward.h"

#ifdef IVY_DEBUG
#include "raylib/external/glad.h"
#endif

#include "raylib/rlgl.h"

#include <stddef.h>
#include <math.h>

typedef struct {
    float x, y, z;
    float u, v;
} IvyCollisionVertex;

#define COLLISION_FLOATS_PER_VERTEX 5

static IvyCollisionVertex *CollisionMap_LoadVertices(IvyAssetManager *restrict manager, IvyArenaLinear *restrict arena, int *outVertexCount)
{
    usize dataSize;
    const u8 *rawData = (const u8 *)Ivy_Asset_Get(manager, ASSET_MAPS_MAP_1_COLLISION_BIN, &dataSize);

    IVY_CHECK(rawData != NULL, "[CollisionMap] Failed to load collusion binary");

    *outVertexCount = (int)(dataSize / sizeof(IvyCollisionVertex));

    IvyCollisionVertex *vertices = Ivy_Arena_LinearAlloc(arena, dataSize);
    IVY_CHECK(vertices != NULL, "[CollisionMap] Arena allocation failed");

    memcpy(vertices, rawData, dataSize);

    return vertices;
}

static void CollisionMap_UploadToGPU(IvyCollusionMap *restrict map, const IvyCollisionVertex *restrict vertices)
{
    map->vaoId = rlLoadVertexArray();
    IVY_ENSURE(map->vaoId != 0);

    rlEnableVertexArray(map->vaoId);

    // upload vertex buffer
    map->vboId = rlLoadVertexBuffer(vertices, map->vertexCount * (int)sizeof(IvyCollisionVertex), false);
    IVY_ENSURE(map->vboId != 0);

    // position attribute (location 0)
    rlSetVertexAttribute(0, 3, RL_FLOAT, false, sizeof(IvyCollisionVertex), offsetof(IvyCollisionVertex, x));
    rlEnableVertexAttribute(0);

    // texcoord attribute (location 1)
    rlSetVertexAttribute(1, 2, RL_FLOAT, false, sizeof(IvyCollisionVertex), offsetof(IvyCollisionVertex, u));
    rlEnableVertexAttribute(1);

    rlDisableVertexArray();
}

static void RasteriseTriangle(const IvyCollusionGrid *grid, float ax, float ay, float bx, float by, float cx, float cy)
{
    if (!isfinite(ax) || !isfinite(ay) ||
        !isfinite(bx) || !isfinite(by) ||
        !isfinite(cx) || !isfinite(cy))
        return;

    // bounding box in pixels
    const float fMinX = ax < bx ? (ax < cx ? ax : cx) : (bx < cx ? bx : cx);
    const float fMinY = ay < by ? (ay < cy ? ay : cy) : (by < cy ? by : cy);
    const float fMaxX = ax > bx ? (ax > cx ? ax : cx) : (bx > cx ? bx : cx);
    const float fMaxY = ay > by ? (ay > cy ? ay : cy) : (by > cy ? by : cy);

    // convert to tile range
    int tMinX = (int)floorf((fMinX - grid->originX) / IVY_TILE_SIZE);
    int tMinY = (int)floorf((fMinY - grid->originY) / IVY_TILE_SIZE);
    int tMaxX = (int)floorf((fMaxX - grid->originX) / IVY_TILE_SIZE);
    int tMaxY = (int)floorf((fMaxY - grid->originY) / IVY_TILE_SIZE);

    // clamp to grid bounds
    if (tMinX < 0) tMinX = 0;
    if (tMinY < 0) tMinY = 0;
    if (tMaxX >= grid->gridW) tMaxX = grid->gridW - 1;
    if (tMaxY >= grid->gridH) tMaxY = grid->gridH - 1;

    // edge vectors for point-in-triangle (barycentric)
    const float v0x = cx - ax, v0y = cy - ay;
    const float v1x = bx - ax, v1y = by - ay;
    const float d00 = v0x * v0x + v0y * v0y;
    const float d01 = v0x * v1x + v0y * v1y;
    const float d11 = v1x * v1x + v1y * v1y;
    float invDenom  = d00 * d11 - d01 * d01;

    if (fabsf(invDenom) < 1e-6f) return;
    invDenom = 1.0f / invDenom;

    for (int ty = tMinY; ty <= tMaxY; ty++) {
        for (int tx = tMinX; tx <= tMaxX; tx++)
        {
            const float px = grid->originX + ((float)tx + 0.5f) * IVY_TILE_SIZE;
            const float py = grid->originY + ((float)ty + 0.5f) * IVY_TILE_SIZE;

            const float v2x = px - ax, v2y = py - ay;
            const float d20 = v2x * v0x + v2y * v0y;
            const float d21 = v2x * v1x + v2y * v1y;

            const float v = (d11 * d20 - d01 * d21) * invDenom;
            const float w = (d00 * d21 - d01 * d20) * invDenom;
            const float u = 1.0f - v - w;

            if (u >= 0.0f && v >= 0.0f && w >= 0.0f) {
                grid->tiles[ty * grid->gridW + tx] = true;
            }
        }
    }
}

static IvyCollusionGrid *BuildCollisionGrid(IvyArenaLinear *restrict arena,
                                            const IvyCollisionVertex *restrict vertices,
                                            const int vertexCount)
{
    const int triCount = vertexCount / 3;
    if (triCount == 0) return NULL;

    // find bounding box of ALL collusion geometry
    float minX =  1e30f, minY =  1e30f;
    float maxX = -1e30f, maxY = -1e30f;

    for (int i = 0; i < vertexCount; i++)
    {
        float x = vertices[i].x;
        float y = vertices[i].y;

        if (!isfinite(x) || !isfinite(y)) continue;

        if (x < minX) minX = x;
        if (y < minY) minY = y;
        if (x > maxX) maxX = x;
        if (y > maxY) maxY = y;
    }

    // compute grid dimensions (add +1 tile margin)
    int gridW = (int)ceilf((maxX - minX) / IVY_TILE_SIZE) + 2;
    int gridH = (int)ceilf((maxY - minY) / IVY_TILE_SIZE) + 2;

    // safety cap
    if (gridW > 512) gridW = 512;
    if (gridH > 512) gridH = 512;

    // allocate grid
    IvyCollusionGrid *grid = Ivy_Arena_LinearAllocZero(arena, sizeof(IvyCollusionGrid));
    IVY_CHECK(grid != NULL, "[CollisionGrid] Arena alloc failed");

    grid->tiles = Ivy_Arena_LinearAllocZero(arena, (usize)gridW * (usize)gridH * sizeof(bool));
    IVY_CHECK(grid->tiles != NULL, "[CollisionGrid] Arena alloc for tiles failed");

    grid->gridW  = gridW;
    grid->gridH  = gridH;
    grid->originX = minX - IVY_TILE_SIZE;
    grid->originY = minY - IVY_TILE_SIZE;

    for (int t = 0; t < triCount; t++)
    {
        const IvyCollisionVertex *a = &vertices[t * 3 + 0];
        const IvyCollisionVertex *b = &vertices[t * 3 + 1];
        const IvyCollisionVertex *c = &vertices[t * 3 + 2];

        RasteriseTriangle(grid, a->x, a->y, b->x, b->y, c->x, c->y);
    }

    return grid;
}

IvyCollusionMap *Ivy_Collusion_Load(IvyArenaLinear *restrict arena, IvyAssetManager *restrict manager)
{
    IVY_ASSERT(manager != NULL, "Asset manager must not be NULL");
    IVY_ASSERT(arena   != NULL, "Arena must not be NULL");

    IvyCollusionMap *map = Ivy_Arena_LinearAllocZero(arena, sizeof(IvyCollusionMap));
    IVY_CHECK(map != NULL, "[CollisionMap] Arena allocation failed");

    int vertexCount = 0;
    IvyCollisionVertex *vertices = CollisionMap_LoadVertices(manager, arena, &vertexCount);

    map->vertexCount = vertexCount;
    map->vertices    = vertices;
    map->floatStride = COLLISION_FLOATS_PER_VERTEX;

    // build precomputed collusion grid
    map->grid = BuildCollisionGrid(arena, vertices, vertexCount);

    CollisionMap_UploadToGPU(map, vertices);

#ifdef IVY_DEBUG
    if (map->grid) {
        TraceLog(LOG_INFO, "[CollisionMap] Grid built: %dx%d tiles, origin(%.0f, %.0f)",
                 map->grid->gridW, map->grid->gridH, map->grid->originX, map->grid->originY);
    } else {
        TraceLog(LOG_WARNING, "[CollisionMap] No collusion grid built (no triangles?)");
    }
#endif

    return map;
}

void Ivy_Collusion_Draw(const IvyCollusionMap *map)
{
    IVY_ASSERT(map != NULL,          "CollisionMap must not be NULL");
    IVY_ASSERT(map->vaoId != 0,      "VAO not initialized");
    IVY_ASSERT(map->vertexCount > 0, "No vertices to draw");

    rlDrawRenderBatchActive();
    rlEnableShader(rlGetShaderIdDefault());

#ifdef IVY_DEBUG
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
#endif

    rlEnableVertexArray(map->vaoId);
    rlDrawVertexArray(0, map->vertexCount);
    rlDisableVertexArray();

#ifdef IVY_DEBUG
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
#endif
}

void Ivy_Collusion_Unload(const IvyCollusionMap *map)
{
    IVY_ASSERT(map != NULL, "CollisionMap must not be NULL");

    if (map->vboId != 0) rlUnloadVertexBuffer(map->vboId);
    if (map->vaoId != 0) rlUnloadVertexArray(map->vaoId);
}

bool Ivy_Collusion_IsTileSolid(const IvyCollusionMap *map, const int tileX, const int tileY)
{
    if (map == NULL || map->grid == NULL) return false;

    const IvyCollusionGrid *g = map->grid;

    const int gx = tileX - (int)floorf(g->originX / IVY_TILE_SIZE);
    const int gy = tileY - (int)floorf(g->originY / IVY_TILE_SIZE);

    if (gx < 0 || gx >= g->gridW || gy < 0 || gy >= g->gridH) return false;

    return g->tiles[gy * g->gridW + gx];
}
