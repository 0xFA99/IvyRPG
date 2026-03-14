#ifndef IVY_PLAYER_H
#define IVY_PLAYER_H

#include "ivy/virtual.h"
#include "ivy/collision.h"
#include "../inventory/inventory.h"
#include "ivy/player/portrait.h"
#include "ivy/player/player_internal.h"

struct Player {
    PlayerGraphics      graphics;
    PlayerMovement      movement;
    PlayerAnimation     animation;
    PlayerEquipment     equipment;
    Inventory          *inventory;
    Portrait            portrait;
};

Player  *InitPlayer(u32 spawnX, u32 spawnY, u32 tileSize);
void     UpdatePlayer(Player *player, float frameTime, const Collision *collision, u32 tileSize);
void     DrawPlayer(const Player *player, const VirtualResolution *vr);
void     DrawPlayerDebug(const Player *player);
void     UpdatePlayerCollision(Player *player);

void     PlayerEquip(Player *player, u32 inventoryIndex);
void     PlayerUnequip(Player *player, EquipmentSlot slot);

void     DestroyPlayer(Player *player);

#endif