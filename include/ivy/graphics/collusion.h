#ifndef IVY_GRAPHICS_COLLUSION_H
#define IVY_GRAPHICS_COLLUSION_H

#include "ivy/core/types.h"
#include "ivy/utils/forward.h"

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    bool  *tiles;       // 8
    float  originX;     // 4
    float  originY;     // 4
    int    gridW;       // 4
    int    gridH;       // 4
} IvyCollusionGrid;     // 24

struct IvyCollisionMap {
    IvyCollusionGrid   *grid;           // 8
    void               *vertices;       // 8
    u32                 vaoId;          // 4
    u32                 vboId;          // 4
    int                 vertexCount;    // 4
    int                 floatStride;    // 4
};                                      // 32

IvyCollusionMap    *Ivy_Collusion_Load(IvyArenaLinear *restrict arena, IvyAssetManager *restrict manager);
void                Ivy_Collusion_Draw(const IvyCollusionMap *map);
void                Ivy_Collusion_Unload(const IvyCollusionMap *map);

bool                Ivy_Collusion_IsTileSolid(const IvyCollusionMap *map, int tileX, int tileY);

#ifdef __cplusplus
}
#endif

#endif
