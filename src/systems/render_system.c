#include "ivy/systems/render_system.h"
#include "ivy/entities/player.h"
#include "ivy/entities/door.h"

void Ivy_RenderSystem_SortAndDraw(const IvyObjectManager *objectManager)
{
    IVY_ASSERT(objectManager != NULL, "[IvyRenderSystem] Object Manager is NULL!");

    IvyRenderSystem renderSystem = {0};

    if (objectManager->hasPlayer && objectManager->player.data != NULL) {
        IvyPlayer *player = (IvyPlayer *)objectManager->player.data;
        renderSystem.nodes[renderSystem.nodeCount++] = (IvyRenderNode){
            .ySortValue = player->movement.position.y,
            .type = IVY_OBJECT_TYPE_PLAYER,
            .instanceData = player
        };
    }

    for (u8 i = 0; i < objectManager->objectCount; ++i) {
        const IvyObject *obj = &objectManager->objects[i];
        if (obj == NULL || obj->data == NULL) continue;

        float yValue = 0.0f;
        bool hasVisual = false;

        switch (obj->type) {
            case IVY_OBJECT_TYPE_DOOR: {
                const IvyDoor *door = (IvyDoor *)obj->data;
                yValue = door->position.y;
                hasVisual = true;
            } break;

            default:
                break;
        }

        if (hasVisual) {
            renderSystem.nodes[renderSystem.nodeCount++] = (IvyRenderNode){
                .ySortValue = yValue,
                .type = obj->type,
                .instanceData = obj->data
            };
        }
    }

    for (i32 i = 1; i < renderSystem.nodeCount; ++i) {
        const IvyRenderNode key = renderSystem.nodes[i];
        i32 j = i - 1;

        while (j >= 0 && renderSystem.nodes[j].ySortValue > key.ySortValue) {
            renderSystem.nodes[j + 1] = renderSystem.nodes[j];
            j = j - 1;
        }
        renderSystem.nodes[j + 1] = key;
    }

    for (u8 i = 0; i < renderSystem.nodeCount; ++i) {
        const IvyRenderNode *node = &renderSystem.nodes[i];

        switch (node->type) {
            case IVY_OBJECT_TYPE_PLAYER:
                Ivy_Player_Render((IvyPlayer *)node->instanceData);
                break;

            case IVY_OBJECT_TYPE_DOOR:
                Ivy_Door_Draw((IvyDoor *)node->instanceData);
                break;

            default:
                break;
        }
    }
}