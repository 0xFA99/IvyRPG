#ifndef IVY_GRAPHICS_CAMERA_H
#define IVY_GRAPHICS_CAMERA_H

#include "ivy/graphics/camera.h"
#include "ivy/utils/forward.h"

#include "raylib/raylib.h"

#ifdef __cplusplus
extern "C" {
#endif

struct IvyCamera {
    Camera2D view;
};

IvyCamera   Ivy_Camera_Init(void);
void        Ivy_Camera_Update(IvyCamera *camera, Vector2 playerTarget);

#ifdef __cplusplus
}
#endif

#endif