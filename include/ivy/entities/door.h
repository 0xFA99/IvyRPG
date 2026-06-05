#ifndef IVY_ENTITIES_DOOR_H
#define IVY_ENTITIES_DOOR_H

#include "ivy/audio/wav.h"
#include "ivy/utils/forward.h"

#include "raylib/raylib.h"

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    IVY_DOOR_LEFT = 0,
    IVY_DOOR_RIGHT,
    IVY_DOOR_BOTH,
    IVY_DOOR_MAX
} IvyDoorSide;

typedef enum {
    IVY_DOOR_STATE_CLOSED = 0,
    IVY_DOOR_STATE_OPENED,
    IVY_DOOR_STATE_MAX
} IvyDoorState;

struct IvyDoor {
    Texture2D       texture;
    Rectangle       rect;
    Vector2         position;
    IvyDoorSide     side;
    bool            isOpened;
    IvySound        sound[IVY_DOOR_STATE_MAX];
};

IvyDoor *Ivy_Door_Init(IvyArenaLinear *restrict arena, IvyAssetManager *restrict assetManager, IvyDoorSide side,
                              Vector2 position);
void Ivy_Door_Update(IvyDoor *door);
void Ivy_Door_Draw(const IvyDoor *door);

void Ivy_Door_Open(IvyDoor *door);
void Ivy_Door_Close(IvyDoor *door);
void Ivy_Door_Toggle(IvyDoor *door);
void Ivy_Door_SetPosition(IvyDoor *door, Vector2 newPos);
Rectangle Ivy_Door_GetCollisionRect(const IvyDoor *door);
void Ivy_Door_Unload(IvyDoor *door);

bool Ivy_Door_IsSolid(const IvyDoor *door);
void Ivy_Door_GetTileRect(const IvyDoor *door, int *outTileX, int *outTileY, int *outTileW, int *outTileH);
bool Ivy_Door_CanInteract(const IvyDoor *door, const IvyPlayer *player);
void Ivy_Door_Interact(IvyDoor *door);

#ifdef __cplusplus
}
#endif

#endif