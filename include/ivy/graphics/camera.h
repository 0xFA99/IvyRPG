#ifndef IVY_GRAPHICS_CAMERA_H
#define IVY_GRAPHICS_CAMERA_H

#include "ivy/utils/forward.h"

#include "raylib/raylib.h"

#ifdef __cplusplus
extern "C" {
#endif

struct IvyCamera {
    Camera2D view;
    float targetZoom;
    float minZoom;
    float maxZoom;
};

IvyCamera   Ivy_Camera_Init(void);
void        Ivy_Camera_Update(IvyCamera *camera, Vector2 playerTarget, float mapWidth, float mapHeight);

#ifdef __cplusplus
}
#endif

#endif