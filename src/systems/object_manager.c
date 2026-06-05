#include "ivy/core/game.h"
#include "ivy/systems/object_manager.h"
#include "ivy/arena/linear.h"
#include "ivy/entities/player.h"

#define IVY_OBJECT_PLAYER_MAX 1

IvyObjectManager *Ivy_ObjectManager_Init(IvyArenaLinear *arena)
{
    IVY_ASSERT(arena != NULL, "[IvyArenaLinear] Instance is NULL during ObjectManager Init!");

    IvyObjectManager *objectManager = Ivy_Arena_LinearAllocZero(arena, sizeof(IvyObjectManager));
    return objectManager;
}

void Ivy_ObjectManager_CreatePlayer(IvyObjectManager *restrict objectManager, IvyArenaLinear *restrict arena,
                                    IvyAssetManager *restrict assetManager, const Vector2 position)
{
    IVY_ASSERT(objectManager != NULL, "[IvyObjectManager] Instance is NULL!");
    IVY_ASSERT(arena != NULL, "[IvyArenaLinear] Instance is NULL!");
    IVY_ASSERT(assetManager != NULL, "[IvyAssetManager] Instance is NULL!");

    // if (objectManager->objectCount >= IVY_OBJECT_MAX) return;

    // const u32 index = IVY_OBJECT_OFFSET_PLAYER;
    // if (objectManager->object[index].type == IVY_OBJECT_TYPE_PLAYER && objectManager->object[index].data != NULL) {
    //     return;
    // }

    IvyObject *object = &objectManager->object[IVY_OBJECT_OFFSET_PLAYER];
    if (object == NULL) return;


    object->id = IVY_OBJECT_OFFSET_PLAYER;
    object->type = IVY_OBJECT_TYPE_PLAYER;
    object->data = Ivy_Player_Init(arena, assetManager, position);

    objectManager->objectCount += 1;
}

void Ivy_ObjectManager_CreateDoor(IvyObjectManager *restrict objectManager, IvyArenaLinear *restrict arena, IvyAssetManager *restrict assetManager, const IvyDoorSide side, const Vector2 position)
{
    IVY_ASSERT(objectManager != NULL, "[IvyObjectManager] Instance is NULL!");
    IVY_ASSERT(arena != NULL, "[IvyArenaLinear] Instance is NULL!");
    IVY_ASSERT(assetManager != NULL, "[IvyAssetManager] Instance is NULL!");

    IvyObject *object = &objectManager->object[IVY_OBJECT_OFFSET_DOOR];
    object->id = IVY_OBJECT_OFFSET_DOOR;
    object->type = IVY_OBJECT_TYPE_DOOR;
    object->data = Ivy_Door_Init(arena, assetManager, side, position);

    objectManager->objectCount += 1;
}

IvyObject *Ivy_ObjectManager_GetPlayer(IvyObjectManager *objectManager)
{
    IVY_ASSERT(objectManager != NULL, "[IvyObjectManager] Instance is NULL!");
    return &objectManager->object[IVY_OBJECT_OFFSET_PLAYER];
}

IvyObject *Ivy_ObjectManager_GetDoor(IvyObjectManager *objectManager)
{
    return &objectManager->object[IVY_OBJECT_OFFSET_DOOR];
}