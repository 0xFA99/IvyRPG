#ifndef IVY_NPC_INTERNAL_H
#define IVY_NPC_INTERNAL_H

#include "raylib/raylib.h"
#include "ivy/item.h"

typedef struct NPC NPC;

typedef struct {
    Texture2D hairTexture;
    Texture2D headTexture;
    Texture2D bodyTexture;
} NPCGraphics;

typedef struct {
    Vector2     position;
    Vector2     tilePosition;
    Rectangle   collisionBox;
} NPCMovement;

typedef struct {
    const Item *slots[SLOT_MAX_SIZE];
    u32         slotMask;
} NPCEquipment;

#endif