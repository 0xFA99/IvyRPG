#ifndef IVY_SYSTEMS_INVENTORY_H
#define IVY_SYSTEMS_INVENTORY_H

#include "ivy/core/types.h"
#include "ivy/utils/forward.h"

#include "raylib/raylib.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    Rectangle   icon;           // 16
    u32         spriteSheetID;  // 4
    u32         portraitID;     // 4
} IvyItemVisual;                // 24

typedef struct {
    u16 id;         // 2
    u8  category;   // 1
    u8  type;       // 1
    u8  slot;       // 1
    u8  padding;    // 1
} IvyItemAttribute; //  6

typedef struct {
    u16 nameOffset; // 2
    u16 descOffset; // 2
} IvyItemText;      // 4

struct IvyItemManager {
    IvyItemAttribute   *attributes;     // 8
    IvyItemVisual      *visuals;        // 8
    IvyItemText        *texts;          // 8
    const char         *stringPool;     // 8
    u16                 totalItem;      // 2
    char                padding[6];     // 6
};                                      // 40

IvyItemManager          Ivy_ItemManager_Init(IvyArenaLinear *restrict arena, IvyAssetManager *restrict assetManager);
const IvyItemAttribute *Ivy_ItemManager_GetAttribute(const IvyItemManager *mgr, u16 id);
const IvyItemVisual    *Ivy_ItemManager_GetVisual(const IvyItemManager *mgr, u16 id);

const char             *Ivy_ItemManager_GetName(const IvyItemManager *mgr, u16 id);
const char             *Ivy_ItemManager_GetDesc(const IvyItemManager *mgr, u16 id);

#ifdef __cplusplus
}
#endif

#endif