#include "ivy/graphics/camera.h"
#include "ivy/core/virtual.h"

IvyCamera Ivy_Camera_Init(void)
{
    IvyCamera camera = {0};

    const float hW = VIRTUAL_WIDTH * 0.5f;
    const float hH = VIRTUAL_HEIGHT * 0.5f;

    camera.view.offset = (Vector2){ hW, hH };
    camera.view.target = (Vector2){ hW, hH };
    camera.view.rotation = 0.0f;
    camera.view.zoom = 1.0f;

    camera.targetZoom = 1.0f;
    camera.minZoom = 0.75f;
    camera.maxZoom = 2.5f;

    return camera;
}

void Ivy_Camera_Update(IvyCamera *camera, const Vector2 playerTarget, const float mapWidth, const float mapHeight)
{
    const float dt = GetFrameTime();
    const float smoothSpeed = 5.0f;
    const float zoomSpeed = 4.0f;

    const float wheel = GetMouseWheelMove();
    if (wheel != 0.0f) {
        camera->targetZoom += wheel * 0.15f;

        if (camera->targetZoom < camera->minZoom) camera->targetZoom = camera->minZoom;
        if (camera->targetZoom > camera->maxZoom) camera->targetZoom = camera->maxZoom;
    }

    camera->view.zoom += (camera->targetZoom - camera->view.zoom) * zoomSpeed * dt;

    camera->view.target.x += (playerTarget.x - camera->view.target.x) * smoothSpeed * dt;
    camera->view.target.y += (playerTarget.y - camera->view.target.y) * smoothSpeed * dt;

    const float minX = (VIRTUAL_WIDTH * 0.5f) / camera->view.zoom;
    const float minY = (VIRTUAL_HEIGHT * 0.5f) / camera->view.zoom;
    const float maxX = mapWidth - minX;
    const float maxY = mapHeight - minY;

    if (mapWidth < VIRTUAL_WIDTH / camera->view.zoom) {
        camera->view.target.x = mapWidth * 0.5f;
    }
    else {
        if (camera->view.target.x < minX) camera->view.target.x = minX;
        if (camera->view.target.x > maxX) camera->view.target.x = maxX;
    }

    if (mapHeight < VIRTUAL_HEIGHT / camera->view.zoom) {
        camera->view.target.y = mapHeight * 0.5f;
    }
    else {
        if (camera->view.target.y < minY) camera->view.target.y = minY;
        if (camera->view.target.y > maxY) camera->view.target.y = maxY;
    }
}