#ifndef IVY_INVENTORY_H
#define IVY_INVENTORY_H

#include "ivy/item.h"

#define INVENTORY_CAPACITY 64

typedef struct {
    const Item  *items[INVENTORY_CAPACITY];
    u32          count;
    u32          selectedIndex;
} Inventory;

typedef struct {
    const Item  *slots[SLOT_MAX_SIZE];
    u32          slotMask;
} PlayerEquipment;

Inventory       *CreateInventory(void);
void             DestroyInventory(Inventory *inv);

void             InventoryAdd(Inventory *inv, const Item *item);
void             InventoryRemoveAt(Inventory *inv, u32 index);

void             EquipItem(PlayerEquipment *equip, Inventory *inv, u32 inventoryIndex);

void             UnequipSlot(PlayerEquipment *equip, Inventory *inv, EquipmentSlot slot);

#endif