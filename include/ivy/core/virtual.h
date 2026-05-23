#ifndef IVY_CORE_VIRTUAL_RESOLUTION_H
#define IVY_CORE_VIRTUAL_RESOLUTION_H

#include "ivy/core/types.h"
#include "ivy/utils/forward.h"

#include "raylib/raylib.h"

#ifdef __cplusplus
extern "C" {
#endif

// #define VIRTUAL_WIDTH   320.0f
// #define VIRTUAL_HEIGHT  180.0f

#define VIRTUAL_WIDTH   640.0f
#define VIRTUAL_HEIGHT  360.0f

struct IvyVirtualScreen {
    RenderTexture2D target;         // 44
    Rectangle       source;         // 16
    Rectangle       destination;    // 16
    float           scale;          // 4
};                                  // 80
IVY_ASSERT_STATIC(sizeof(IvyVirtualScreen) == 80, "[Gfx](IvyVirtualScreen) Size must be 80 bytes!");

IvyVirtualScreen   *Ivy_VirtualScreen_Init(IvyArenaLinear *arena, Vector2 size);
void                Ivy_VirtualScreen_Update(IvyVirtualScreen *vs, Vector2 size);
void                Ivy_VirtualScreen_Draw(const IvyVirtualScreen *vs);
void                Ivy_VirtualScreen_Unload(const IvyVirtualScreen *vs);

#ifdef __cplusplus
}
#endif

#endif