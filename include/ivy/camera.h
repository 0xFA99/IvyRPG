#ifndef IVY_CAMERA_H
#define IVY_CAMERA_H

#include "ivy/tilemap/tilemap.h"

typedef struct Player Player;

typedef struct {
    Camera2D    camera2D;
    float       followSpeed;
    float       zoom;
    float       zoomTarget;
    bool        smoothFollow;
} GameCamera;

GameCamera  InitGameCamera(u32 virtualWidth, u32 virtualHeight);
void        UpdateGameCamera(GameCamera *camera, const Player *player, const Tilemap *tilemap, float frameTime);

#endif