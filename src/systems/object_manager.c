#include "ivy/systems/object_manager.h"
#include "ivy/arena/linear.h"
#include "ivy/entities/player.h"

#define IVY_OBJECT_PLAYER_MAX 1

typedef struct {
    union {
        struct {
            u8 tileX;   // 1
            u8 tileY;   // 1
        };
        u16 tilePacked;     // (2)
    };
    u8 objectID;        // 1
    u8 padding;         // 1
} IvyObjectSlot;            // (4)

static IvyObjectSlot IVY_OBJECT_SLOTS[IVY_OBJECT_MAX] = {0};
static u8 slotCount = 0;

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
    IVY_ASSERT(!objectManager->hasPlayer, "[IvyObjectManager] Player already initialized!");

    objectManager->player.id = 0;
    objectManager->player.type = IVY_OBJECT_TYPE_PLAYER;
    objectManager->player.data = Ivy_Player_Init(arena, assetManager, position);
    objectManager->hasPlayer = true;
}

void Ivy_ObjectManager_CreateDoor(IvyObjectManager *restrict objectManager, IvyArenaLinear *restrict arena, IvyAssetManager *restrict assetManager, const IvyDoorSide side, const Vector2 position)
{
    IVY_ASSERT(objectManager != NULL, "[IvyObjectManager] Instance is NULL!");
    IVY_ASSERT(arena != NULL, "[IvyArenaLinear] Instance is NULL!");
    IVY_ASSERT(assetManager != NULL, "[IvyAssetManager] Instance is NULL!");
    IVY_ASSERT(objectManager->objectCount < IVY_OBJECT_MAX, "[IvyObjectManager] Max object limit reached!");

    const u8 index = objectManager->objectCount;
    IvyObject *object = &objectManager->objects[index];

    object->id = index;
    object->type = IVY_OBJECT_TYPE_DOOR;
    object->data = Ivy_Door_Init(arena, assetManager, side, position);

    IVY_OBJECT_SLOTS[slotCount] = (IvyObjectSlot) {
        .objectID = index,
        .tileX = (u16)position.x,
        .tileY = (u16)position.y + 1
    };

    slotCount += 1;
    objectManager->objectCount += 1;
}

IvyObject *Ivy_ObjectManager_GetPlayer(IvyObjectManager *objectManager)
{
    IVY_ASSERT(objectManager != NULL, "[IvyObjectManager] Instance is NULL!");
    if (!objectManager->hasPlayer) return NULL;

    return &objectManager->player;
}

IvyObject *Ivy_ObjectManager_GetDoor(IvyObjectManager *objectManager, const u8 doorIndex)
{
    IVY_ASSERT(objectManager != NULL, "[IvyObjectManager] Instance is NULL!");

    u8 currentDoor = 0;
    for (u8 i = 0; i < objectManager->objectCount; ++i) {
        if (objectManager->objects[i].type == IVY_OBJECT_TYPE_DOOR) {
            if (currentDoor == doorIndex) {
                return &objectManager->objects[i];
            }
            currentDoor++;
        }
    }
    return NULL;
}

IvyObject *Ivy_ObjectManager_GetObject(IvyObjectManager *objectManager, const u8 tileX, const u8 tileY)
{
    const u16 targetPacked = (u16)tileX | ((u16)tileY << 8);

    for (usize i = 0; i < IVY_OBJECT_MAX; i++) {
        const IvyObjectSlot *objectSlot = &IVY_OBJECT_SLOTS[i];

        if (objectSlot->tilePacked == targetPacked) {
            const usize index = objectSlot->objectID;

            return &objectManager->objects[index];
        }
    }

    return NULL;
}
