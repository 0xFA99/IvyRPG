#ifndef IVY_OBJECT_MANAGER_H
#define IVY_OBJECT_MANAGER_H

#include "ivy/core/types.h"
#include "ivy/entities/door.h"
#include "ivy/utils/forward.h"

#ifdef __cplusplus
extern "C" {
#endif

#define IVY_OBJECT_MAX 8
#define IVY_OBJECT_OFFSET_PLAYER 0
#define IVY_OBJECT_OFFSET_DOOR 1

typedef enum {
    IVY_OBJECT_TYPE_PLAYER = 0,
    // IVY_OBJECT_TYPE_NPC,
    IVY_OBJECT_TYPE_DOOR,
    IVY_OBJECT_TYPE_MAX
 } IvyObjectType;

typedef struct {
    u32 id;
    IvyObjectType type;
    void *data;
} IvyObject;

struct IvyObjectManager {
    IvyObject player;
    bool hasPlayer;

    IvyObject objects[IVY_OBJECT_MAX];
    u8 objectCount;
};

IvyObjectManager   *Ivy_ObjectManager_Init(IvyArenaLinear *arena);

void                Ivy_ObjectManager_CreatePlayer(IvyObjectManager *restrict objectManager,
                                                   IvyArenaLinear *restrict arena,
                                                   IvyAssetManager *restrict assetManager,
                                                   Vector2 position);

void                Ivy_ObjectManager_CreateDoor(IvyObjectManager *restrict objectManager,
                                                 IvyArenaLinear *restrict arena, IvyAssetManager *restrict assetManager,
                                                 IvyDoorSide side, Vector2 position);

IvyObject          *Ivy_ObjectManager_GetPlayer(IvyObjectManager *objectManager);
IvyObject          *Ivy_ObjectManager_GetDoor(IvyObjectManager *objectManager, u8 doorIndex);
IvyObject          *Ivy_ObjectManager_GetObject(IvyObjectManager *objectManager, u8 tileX, u8 tileY);

#ifdef __cplusplus
}
#endif

#endif