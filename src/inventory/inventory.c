#include "ivy/inventory/inventory.h"
#include "ivy/game.h"

#include <stdlib.h>


Inventory *CreateInventory(void)
{
    Inventory *inv = calloc(1, sizeof(Inventory));
    IVY_ASSERT(inv, "[Inventory] Failed to allocate memory for Inventory instance.");
    return inv;
}

void DestroyInventory(Inventory *inv)
{
    free(inv);
}

void InventoryAdd(Inventory *inv, const Item *item)
{
    IVY_ASSERT(inv, "[Inventory] Cannot add item to a NULL inventory.");
    IVY_ASSERT(item, "[Inventory] Attempted to add a NULL item pointer.");

    if (inv->count >= INVENTORY_CAPACITY) {
        TraceLog(LOG_WARNING, "Inventory full!");
        return;
    }
    inv->items[inv->count++] = item;
}

void InventoryRemoveAt(Inventory *inv, const u32 index)
{
    IVY_ASSERT(inv, "[Inventory] Index out of bounds or inventory is NULL.");

    for (u32 i = index; i < inv->count - 1; i++)
        inv->items[i] = inv->items[i + 1];
    inv->items[--inv->count] = NULL;

    if (inv->count > 0 && inv->selectedIndex >= inv->count)
        inv->selectedIndex = inv->count - 1;
}

void EquipItem(PlayerEquipment *equip, Inventory *inv, const u32 inventoryIndex)
{
    IVY_ASSERT(equip && inv, "[Equipment] Equip or Inventory instance is NULL.");

    if (inventoryIndex >= inv->count) return;

    const Item *item = inv->items[inventoryIndex];
    if (!item || item->type != ITEM_EQUIPMENT) return;

    const EquipmentSlot slot = item->data.equipment.slot;

    if (equip->slotMask & (1u << slot)) {
        const Item *old = equip->slots[slot];
        equip->slots[slot]     = NULL;
        equip->slotMask       &= ~(1u << slot);
        InventoryAdd(inv, old);
    }

    InventoryRemoveAt(inv, inventoryIndex);
    equip->slots[slot]  = item;
    equip->slotMask    |= (1u << slot);
}

void UnequipSlot(PlayerEquipment *equip, Inventory *inv, const EquipmentSlot slot)
{
    IVY_ASSERT(equip && inv, "[Equipment] Equip or Inventory instance is NULL.");

    if (!(equip->slotMask & (1u << slot))) return;

    const Item *item    = equip->slots[slot];
    equip->slots[slot]  = NULL;
    equip->slotMask    &= ~(1u << slot);

    InventoryAdd(inv, item);
}