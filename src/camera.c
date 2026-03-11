#include "ivy/camera.h"
#include "ivy/player/player.h"
#include "ivy/virtual.h"

#include "raylib/raymath.h"
#include <math.h>

GameCamera InitGameCamera(const u32 virtualWidth, const u32 virtualHeight)
{
    const float hw = floorf((float)virtualWidth  * 0.5f);
    const float hh = floorf((float)virtualHeight * 0.5f);

    const GameCamera gc = {
        .camera2D = (Camera2D){
            .offset   = { hw, hh },
            .target   = { 0 },
            .rotation = 0.0f,
            .zoom     = 1.0f
        },
        .followSpeed = 8.0f,
        .zoom        = 1.0f,
        .zoomTarget  = 1.0f,
        .smoothFollow = true
    };

    return gc;
}

void UpdateGameCamera(GameCamera *camera, const Player *player,
                      const Tilemap *tilemap, const float frameTime)
{
    if (!camera || !player || !tilemap) return;

    const float wheel = GetMouseWheelMove();
    if (wheel != 0.0f) {
        camera->zoomTarget += wheel * 0.25f;
        camera->zoomTarget  = Clamp(camera->zoomTarget, 0.5f, 4.0f);
    }

    const float zoomLerp    = 1.0f - expf(-8.0f * frameTime);
    camera->zoom            = Lerp(camera->zoom, camera->zoomTarget, zoomLerp);
    camera->camera2D.zoom   = camera->zoom;

    const Vector2 target = {
        player->movement.position.x,
        player->movement.position.y
    };

    if (camera->smoothFollow) {
        const float posLerp = 1.0f - expf(-camera->followSpeed * frameTime);
        camera->camera2D.target = Vector2Lerp(camera->camera2D.target, target, posLerp);
    } else {
        camera->camera2D.target = target;
    }

    const float mapW = (float)(tilemap->header.width  * tilemap->header.tileWidth);
    const float mapH = (float)(tilemap->header.height * tilemap->header.tileHeight);

    const float halfViewW = camera->camera2D.offset.x / camera->camera2D.zoom;
    const float halfViewH = camera->camera2D.offset.y / camera->camera2D.zoom;

    if (halfViewW * 2.0f >= mapW)
        camera->camera2D.target.x = mapW * 0.5f;
    else
        camera->camera2D.target.x = Clamp(camera->camera2D.target.x, halfViewW, mapW - halfViewW);

    if (halfViewH * 2.0f >= mapH)
        camera->camera2D.target.y = mapH * 0.5f;
    else
        camera->camera2D.target.y = Clamp(camera->camera2D.target.y, halfViewH, mapH - halfViewH);
}