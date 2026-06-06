#ifndef IVY_RENDER_SYSTEM_H
#define IVY_RENDER_SYSTEM_H

#include "ivy/core/types.h"
#include "ivy/systems/object_manager.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    float ySortValue;
    IvyObjectType type;
    void *instanceData;
} IvyRenderNode;

struct IvyRenderSystem {
    IvyRenderNode nodes[IVY_OBJECT_MAX + 1];
    u8 nodeCount;
};

void Ivy_RenderSystem_SortAndDraw(const IvyObjectManager *objectManager);

#ifdef __cplusplus
}
#endif

#endif