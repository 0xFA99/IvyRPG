#ifndef IVY_INVENTORY_H
#define IVY_INVENTORY_H

#include "ivy/core/types.h"
#include "ivy/utils/forward.h"

#include <stdbool.h>

#define IVY_INVENTORY_MAX 64

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    u16 itemID;     // 2
    u16 quantity;   // 2
} IvyInventorySlot; // 4

typedef struct {
    u8 index[IVY_SLOT_MAX]; // 11
    u8 padding;             //  1
} IvyEquipmentRack;         // 12

typedef struct {
    IvyInventorySlot slot[IVY_INVENTORY_MAX];           // 4×64 = 256
    u8               categoryIndices[IVY_INVENTORY_MAX];//      =  64
    u8               categoryOffset[IVY_ITEM_TYPE_MAX]; //      =   3
    u8               categoryCount[IVY_ITEM_TYPE_MAX];  //      =   3
    u8               count;                             //          1
    u8               capacity;                          //          1
    u8               padding[2];                        //          2
} IvyInventoryBag;                                      //        330

struct IvyInventory {
    IvyInventoryBag  bag;           // 330
    IvyEquipmentRack equipped;      //  12
    u8               padding[2];    //   2
};                                  // 344

IvyInventory Ivy_Inventory_Init(void);

i32          Ivy_Inventory_AddItem(IvyInventoryBag *bag, u16 itemID, u8 itemType, u16 quantity);
bool         Ivy_Inventory_RemoveItem(IvyInventoryBag *restrict bag, const IvyItemManager *restrict itemManager, u16 itemID, u16 quantity);

bool         Ivy_Inventory_Equip(IvyInventory *inventory, u8 bagIndex, u8 equipSlot);
bool         Ivy_Inventory_Unequip(IvyInventory *inventory, u8 equipSlot);

u16          Ivy_Inventory_GetEquippedItemID(const IvyInventory *inventory, u8 equipSlot);
void         Ivy_Inventory_RebuildCategoryIndex(IvyInventoryBag *restrict bag, const IvyItemManager *restrict itemManager);

#ifdef __cplusplus
}
#endif

#endif
