#ifndef IVY_VIRTUAL_RESOLUTION_H
#define IVY_VIRTUAL_RESOLUTION_H

#include "ivy/types.h"
#include "raylib/raylib.h"

// #define VIRTUAL_WIDTH   320.0f
// #define VIRTUAL_HEIGHT  180.0f

#define VIRTUAL_WIDTH   640.0f
#define VIRTUAL_HEIGHT  360.0f

typedef struct {
    RenderTexture2D target;
    Rectangle       source;
    Rectangle       destination;
    float           scale;
} VirtualResolution;

VirtualResolution   InitVirtualScreen(u32 sw, u32 sh);
void                UpdateVirtualResolution(VirtualResolution *vr, u32 sw, u32 sh);
void                DrawVirtualResolution(const VirtualResolution *vr);

#endif