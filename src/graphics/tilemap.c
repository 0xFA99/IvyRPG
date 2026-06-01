#include "ivy/core/types.h"
#include "ivy/arena/linear.h"
#include "ivy/systems/asset_manager.h"
#include "ivy/graphics/tilemap.h"
#include "ivy/graphics/gfx.h"

#include "raylib/rlgl.h"
#include "raylib/raymath.h"

#include <stdio.h>
#include <stdalign.h>
#include <xmmintrin.h>

typedef struct {
    u32 file_id;
    u16 tileWidth;
    u16 tileHeight;
    u16 mapWidth;
    u16 mapHeight;
    u16 totalCommands;
} IvyTilemapHeader;

typedef struct {
    u16 destX, destY;
    u16 srcX, srcY;
    u16 srcW, srcH;
    u16 padding[2];
} IvyDrawCmd;

typedef struct {
    float x, y, z;
    float u, v;
    u32 color;
} IvyVertex;

static void Tilemap_SetupDefaultShader(IvyTilemap *tilemap)
{
    IVY_ASSERT(tilemap != NULL, "Tilemap must not be NULL");

    tilemap->shader.id   = rlGetShaderIdDefault();
    tilemap->shader.locs = rlGetShaderLocsDefault();

    const Matrix modelView = rlGetMatrixModelview();
    const Matrix projection = rlGetMatrixProjection();
    const Matrix mvp = MatrixMultiply(modelView, projection);

    SetShaderValueMatrix(tilemap->shader, tilemap->shader.locs[SHADER_LOC_MATRIX_MVP], mvp);

    tilemap->colDiffuseLoc = tilemap->shader.locs[SHADER_LOC_COLOR_DIFFUSE];
}

static const IvyDrawCmd*
Tilemap_LoadCommands(IvyAssetManager *manager, const u32 vertex, usize *out_count)
{
    IVY_ASSERT(manager != NULL, "Asset manager must not be NULL");

    usize size;
    const void *raw = Ivy_Asset_Get(manager, vertex, &size);
    IVY_CHECK(raw != NULL, "[Tilemap] Failed to load vertex binary asset (id=%d)", vertex);

    IVY_ASSERT(size % sizeof(IvyDrawCmd) == 0, "[Tilemap] Invalid command data size: %zu bytes", size);

    *out_count = size / sizeof(IvyDrawCmd);

    return (const IvyDrawCmd *)raw;
}

IVY_INLINE __m128 Tilemap_ComputeInvTexSize(const int texWidth, const int texHeight)
{
    IVY_ASSERT(texWidth > 0 && texHeight > 0, "Texture dimensions must be positive (w=%d, h=%d)", texWidth, texHeight);

    const __m128 size    = _mm_set_ps(0.0f, (float)texHeight, (float)texWidth, 0.0f);
    const __m128 invSize = _mm_rcp_ps(size);

    return invSize;
}

IVY_INLINE void
Tilemap_BuildQuadVertices(IvyVertex *restrict vertices, const int quadIndex, const IvyDrawCmd *restrict cmd, const __m128 invTexSize)
{
    IVY_ASSERT(vertices != NULL, "Vertex buffer must not be NULL");
    IVY_ASSERT(cmd      != NULL, "Draw command must not be NULL");
    IVY_ASSERT(quadIndex >= 0,   "Quad index must not be negative");

    const float dx = cmd->destX;
    const float dy = cmd->destY;
    const float sx = cmd->srcX;
    const float sy = cmd->srcY;
    const float sw = cmd->srcW;
    const float sh = cmd->srcH;

    const __m128 src_start = _mm_set_ps(0.0f, sy,      sx,      0.0f);
    const __m128 src_end   = _mm_set_ps(0.0f, sy + sh, sx + sw, 0.0f);

    alignas(16) float uv0[4], uv1[4];
    _mm_store_ps(uv0, _mm_mul_ps(src_start, invTexSize));
    _mm_store_ps(uv1, _mm_mul_ps(src_end,   invTexSize));

    const float u0 = uv0[1], v0 = uv0[2]; // top-left  UV
    const float u1 = uv1[1], v1 = uv1[2]; // bot-right UV

    const int base = quadIndex * 6;

    // Triangle 1
    vertices[base + 0] = (IvyVertex){ dx,      dy,      0.0f, u0, v0, 0xFFFFFFFF }; // top-left
    vertices[base + 1] = (IvyVertex){ dx + sw, dy,      0.0f, u1, v0, 0xFFFFFFFF }; // top-right
    vertices[base + 2] = (IvyVertex){ dx,      dy + sh, 0.0f, u0, v1, 0xFFFFFFFF }; // bot-left

    // Triangle 2
    vertices[base + 3] = (IvyVertex){ dx + sw, dy,      0.0f, u1, v0, 0xFFFFFFFF }; // top-right
    vertices[base + 4] = (IvyVertex){ dx,      dy + sh, 0.0f, u0, v1, 0xFFFFFFFF }; // bot-left
    vertices[base + 5] = (IvyVertex){ dx + sw, dy + sh, 0.0f, u1, v1, 0xFFFFFFFF }; // bot-right
}

static IvyVertex*
Tilemap_BuildVertexBuffer(IvyArenaLinear *restrict arena, const void *restrict commandsRow, const int count, const __m128 invTexSize)
{
    IVY_ASSERT(arena        != NULL, "Arena must not be NULL");
    IVY_ASSERT(commandsRow != NULL, "Commands must not be NULL");
    IVY_ASSERT(count > 0,            "Command count must be positive, got %d", count);

    IvyVertex *vertices = Ivy_Arena_LinearAlloc(arena, (usize)count * 6 * sizeof(IvyVertex));
    IVY_CHECK(vertices != NULL, "[Tilemap] Arena allocation failed for %d vertices", count * 6);

    const char *raw = (const char *)commandsRow;
    for (int i = 0; i < count; i++) {
        IvyDrawCmd cmd;
        memcpy(&cmd, raw + (i * sizeof(IvyDrawCmd)), sizeof(IvyDrawCmd));
        Tilemap_BuildQuadVertices(vertices, i, &cmd, invTexSize);
    }

    return vertices;
}

static void
Tilemap_UploadToGPU(IvyTilemap *restrict tilemap, const IvyVertex *restrict vertices)
{
    IVY_ASSERT(tilemap  != NULL, "Tilemap must not be NULL");
    IVY_ASSERT(vertices != NULL, "Vertex data must not be NULL");

    tilemap->vaoId = rlLoadVertexArray();
    IVY_ENSURE(tilemap->vaoId != 0);

    rlEnableVertexArray(tilemap->vaoId);

    tilemap->vboId = rlLoadVertexBuffer(vertices, tilemap->vertexCount * (int)sizeof(IvyVertex), false);
    IVY_ENSURE(tilemap->vboId != 0);

    // loc 0 — vertexPosition
    rlSetVertexAttribute(0, 3, RL_FLOAT, false, sizeof(IvyVertex), offsetof(IvyVertex, x));
    rlEnableVertexAttribute(0);

    // loc 1 — vertexTexCoord
    rlSetVertexAttribute(1, 2, RL_FLOAT, false, sizeof(IvyVertex), offsetof(IvyVertex, u));
    rlEnableVertexAttribute(1);

    // loc 3 — vertexColor
    rlSetVertexAttribute(3, 4, RL_UNSIGNED_BYTE, true, sizeof(IvyVertex), offsetof(IvyVertex, color));
    rlEnableVertexAttribute(3);

    rlDisableVertexArray();
}

IvyTilemap *Ivy_Tilemap_LoadMap(IvyAssetManager *restrict manager, IvyArenaLinear *restrict arena, const u32 metadata, const u32 vertex)
{
    IVY_ASSERT(manager != NULL, "Asset manager must not be NULL");
    IVY_ASSERT(arena   != NULL, "Arena must not be NULL");

    IvyTilemap *tilemap = Ivy_Arena_LinearAllocZero(arena, sizeof(IvyTilemap));

    // Setup default shader
    Tilemap_SetupDefaultShader(tilemap);

    usize header_size;
    const void *header_raw = Ivy_Asset_Get(manager, metadata, &header_size);
    IVY_CHECK(header_raw != NULL, "[Tilemap] Failed to load metadata asset (id=%d)", metadata);

    IvyTilemapHeader header;
    memcpy(&header, header_raw, sizeof(IvyTilemapHeader));

    tilemap->tilesetTex = Ivy_Gfx_LoadTextureDDS(manager, header.file_id);
    SetTextureFilter(tilemap->tilesetTex, TEXTURE_FILTER_POINT);

    const IvyArenaLinearSnapshot snap = Ivy_Arena_LinearGetSnapshot(arena);

    // Load commands
    usize cmd_count = 0;
    const IvyDrawCmd *commands = Tilemap_LoadCommands(manager, vertex, &cmd_count);
    const int count = (int)cmd_count;

    // Build vertices
    tilemap->vertexCount        = count * 6;
    const __m128  invTexSize    = Tilemap_ComputeInvTexSize(tilemap->tilesetTex.width, tilemap->tilesetTex.height);
    const IvyVertex *vertices   = Tilemap_BuildVertexBuffer(arena, (const void *)commands, count, invTexSize);

    // Upload GPU
    Tilemap_UploadToGPU(tilemap, vertices);
    Ivy_Arena_LinearRestore(arena, snap);

    return tilemap;
}

void Ivy_Tilemap_Render(const IvyTilemap *tilemap)
{
    IVY_ASSERT(tilemap != NULL,          "Tilemap must not be NULL");
    IVY_ASSERT(tilemap->vaoId != 0,      "VAO not initialized");
    IVY_ASSERT(tilemap->vertexCount > 0, "No vertices to draw");

    // Sinc Batching
    rlDrawRenderBatchActive();
    rlDisableBackfaceCulling();
    rlDisableDepthTest();

    // Update MVP
    const Matrix mvp = MatrixMultiply(rlGetMatrixModelview(), rlGetMatrixProjection());
    SetShaderValueMatrix(tilemap->shader, tilemap->shader.locs[SHADER_LOC_MATRIX_MVP], mvp);

    // Set Colors
    const Vector4 white = { 1.0f, 1.0f, 1.0f, 1.0f };
    SetShaderValue(tilemap->shader, tilemap->colDiffuseLoc, &white, SHADER_UNIFORM_VEC4);

    // Bind Resources & Draw
    rlActiveTextureSlot(0);
    rlEnableTexture(tilemap->tilesetTex.id);

    rlEnableVertexArray(tilemap->vaoId);
    rlDrawVertexArray(0, tilemap->vertexCount);
    rlDisableVertexArray();

    rlDisableTexture();
}

void Ivy_Tilemap_Unload(const IvyTilemap *tilemap)
{
    IVY_ASSERT(tilemap != NULL, "Tilemap must not be NULL");

    rlUnloadVertexBuffer(tilemap->vboId);
    rlUnloadVertexArray(tilemap->vaoId);
    rlUnloadTexture(tilemap->tilesetTex.id);
}
