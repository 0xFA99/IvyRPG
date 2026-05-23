#ifndef IVY_CORE_ITEM_MANAGER_H
#define IVY_CORE_ITEM_MANAGER_H

#include "ivy/core/types.h"
#include "ivy/utils/forward.h"

#include "raylib/raylib.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    Rectangle icon;         // 16

    u32 spriteSheet;        // 20
    u32 portrait;           // 20

    u16 id;                 // 2
    u8 category;            // 1
    u8 type;                // 1
    u8 slot;                // 1

    char name[32];          // 32
    char description[64];   // 64
    char padding[3];        // 3
} IvyItem;                  // 128

struct IvyItemManager {
    u16 totalItem;
    IvyItem *items;
};

IvyItemManager Ivy_ItemManager_Init(IvyArenaLinear *restrict arena, IvyAssetManager *restrict assetManager);

#ifdef __cplusplus
}
#endif

#endif