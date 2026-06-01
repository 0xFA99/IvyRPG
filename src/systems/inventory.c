#include "ivy/systems/inventory.h"
#include "ivy/systems/item_manager.h"
#include "ivy/systems/scene_manager.h"

#include <string.h>

enum {
    INVENTORY_WIDTH  = 320,
    INVENTORY_HEIGHT = 280,
    ITEM_ICON_SIZE   = 18
};

static void InsertCategoryIndex(IvyInventoryBag *bag, const u8 bagIndex, const u8 itemType)
{
    const u8 insertPos = bag->categoryOffset[itemType] + bag->categoryCount[itemType];

    const u8 total = bag->count;
    for (u8 i = total - 1; i > insertPos; i--) {
        bag->categoryIndices[i] = bag->categoryIndices[i - 1];
    }

    bag->categoryIndices[insertPos] = bagIndex;
    bag->categoryCount[itemType]++;

    for (usize t = itemType + 1; t < IVY_ITEM_TYPE_MAX; t++) {
        bag->categoryOffset[t]++;
    }
}

IvyInventory Ivy_Inventory_Init(void)
{
    IvyInventory inv = {0};

    inv.bag.capacity = IVY_INVENTORY_MAX;
    memset(inv.equipped.index, IVY_SLOT_EMPTY, sizeof(inv.equipped.index));

    return inv;
}

i32 Ivy_Inventory_AddItem(IvyInventoryBag *bag, const u16 itemID, const u8 itemType, const u16 quantity)
{
    IVY_ASSERT(bag, "[IvyInventoryBag] Instance is NULL!");

    if (IVY_UNLIKELY(bag->count >= bag->capacity)) return -1;

    for (u8 i = 0; i < bag->count; i++) {
        if (bag->slot[i].itemID == itemID) {
            bag->slot[i].quantity += quantity;
            return i;
        }
    }

    const u8 newIndex = bag->count;
    bag->slot[newIndex].itemID   = itemID;
    bag->slot[newIndex].quantity = quantity;
    bag->count++;

    InsertCategoryIndex(bag, newIndex, itemType);

    return newIndex;
}

static void CompactSlot(IvyInventory *inv, const u8 removedIndex)
{
    IvyInventoryBag  *bag      = &inv->bag;
    IvyEquipmentRack *equipped = &inv->equipped;

    for (u8 i = removedIndex; i < bag->count - 1; i++) {
        bag->slot[i] = bag->slot[i + 1];
    }

    bag->slot[bag->count - 1].itemID   = 0;
    bag->slot[bag->count - 1].quantity = 0;
    bag->count--;

    for (usize s = 0; s < IVY_SLOT_MAX; s++)
    {
        if (equipped->index[s] == removedIndex) {
            equipped->index[s] = IVY_SLOT_EMPTY;
        }
        else if (equipped->index[s] > removedIndex && equipped->index[s] != IVY_SLOT_EMPTY) {
            equipped->index[s]--;
        }
    }

    for (u8 i = 0; i < bag->count + 1; i++)
    {
        if (bag->categoryIndices[i] == removedIndex) {
            for (u8 j = i; j < bag->count; j++) {
                bag->categoryIndices[j] = bag->categoryIndices[j + 1];
            }

            bag->categoryIndices[bag->count] = 0;
            break;
        }
    }

    for (u8 i = 0; i < bag->count; i++) {
        if (bag->categoryIndices[i] > removedIndex) {
            bag->categoryIndices[i]--;
        }
    }
}

bool Ivy_Inventory_RemoveItem(IvyInventoryBag *restrict bag, const IvyItemManager *restrict itemManager, const u16 itemID, const u16 quantity)
{
    for (u8 i = 0; i < bag->count; i++) {
        if (bag->slot[i].itemID != itemID) continue;

        if (bag->slot[i].quantity > quantity) {
            bag->slot[i].quantity -= quantity;
            return true;
        }

        const IvyItemAttribute *attr = Ivy_ItemManager_GetAttribute(itemManager, itemID);
        if (IVY_LIKELY(attr)) {
            bag->categoryCount[attr->type]--;
            for (usize t = attr->type + 1; t < IVY_ITEM_TYPE_MAX; t++) {
                bag->categoryOffset[t]--;
            }
        }

        IvyInventory *inv = (IvyInventory *)bag;
        CompactSlot(inv, i);
        return true;
    }

    return false;
}

bool Ivy_Inventory_Equip(IvyInventory *inventory, const u8 bagIndex, const u8 equipSlot)
{
    IVY_ASSERT(inventory, "[IvyInventory] Instance is NULL!");

    if (IVY_UNLIKELY(bagIndex >= inventory->bag.count)) return false;
    if (IVY_UNLIKELY(equipSlot >= IVY_SLOT_MAX))        return false;

    inventory->equipped.index[equipSlot] = bagIndex;
    return true;
}

bool Ivy_Inventory_Unequip(IvyInventory *inventory, const u8 equipSlot)
{
    IVY_ASSERT(inventory, "[IvyInventory] Instance is NULL!");

    if (IVY_UNLIKELY(equipSlot >= IVY_SLOT_MAX)) return false;
    if (IVY_UNLIKELY(inventory->equipped.index[equipSlot] == IVY_SLOT_EMPTY)) return false;

    inventory->equipped.index[equipSlot] = IVY_SLOT_EMPTY;
    return true;
}

u16 Ivy_Inventory_GetEquippedItemID(const IvyInventory *inventory, const u8 equipSlot)
{
    IVY_ASSERT(inventory, "[IvyInventory] Instance is NULL!");

    if (IVY_UNLIKELY(equipSlot >= IVY_SLOT_MAX)) return 0;

    const u8 bagIndex = inventory->equipped.index[equipSlot];
    if (IVY_UNLIKELY(bagIndex == IVY_SLOT_EMPTY)) return 0;
    if (IVY_UNLIKELY(bagIndex >= inventory->bag.count)) return 0;

    return inventory->bag.slot[bagIndex].itemID;
}

void Ivy_Inventory_RebuildCategoryIndex(IvyInventoryBag *restrict bag, const IvyItemManager *restrict itemManager)
{
    memset(bag->categoryOffset, 0, sizeof(bag->categoryOffset));
    memset(bag->categoryCount,  0, sizeof(bag->categoryCount));

    for (u8 i = 0; i < bag->count; i++) {
        const IvyItemAttribute *attr = Ivy_ItemManager_GetAttribute(itemManager, bag->slot[i].itemID);
        if (!attr) continue;
        bag->categoryCount[attr->type]++;
    }

    bag->categoryOffset[0] = 0;
    for (usize t = 1; t < IVY_ITEM_TYPE_MAX; t++) {
        bag->categoryOffset[t] = bag->categoryOffset[t - 1] + bag->categoryCount[t - 1];
    }

    u8 cursor[IVY_ITEM_TYPE_MAX];
    memcpy(cursor, bag->categoryOffset, sizeof(cursor));

    for (u8 i = 0; i < bag->count; i++) {
        const IvyItemAttribute *attr = Ivy_ItemManager_GetAttribute(itemManager, bag->slot[i].itemID);
        if (IVY_UNLIKELY(!attr)) continue;

        bag->categoryIndices[cursor[attr->type]++] = i;
    }
}
