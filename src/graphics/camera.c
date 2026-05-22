#include "ivy/graphics/camera.h"

#include "ivy/core/virtual.h"
#include "ivy/entities/player.h"

IvyCamera Ivy_Camera_Init(void)
{
    IvyCamera camera = {0};

    const float hW = VIRTUAL_WIDTH*0.5f;
    const float hH = VIRTUAL_HEIGHT*0.5f;

    camera.view.offset = (Vector2){ hW, hH };
    camera.view.target = (Vector2){ hW, hH };
    camera.view.rotation = 0.0f;
    camera.view.zoom = 1.0f;

    return camera;
}

void Ivy_Camera_Update(IvyCamera *camera, const Vector2 playerTarget)
{
    const float smoothSpeed = 5.0f;
    const float dt = GetFrameTime();

    camera->view.target.x += (playerTarget.x - camera->view.target.x) * smoothSpeed * dt;
    camera->view.target.y += (playerTarget.y - camera->view.target.y) * smoothSpeed * dt;
}