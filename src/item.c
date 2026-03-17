#include "ivy/item.h"
#include "ivy/utils.h"
#include "ivy/game.h"

#include <stdlib.h>


ItemManager *CreateItemManager(void)
{
    ItemManager *m = calloc(1, sizeof(ItemManager));
    IVY_ASSERT(m, "[ItemManager] Memory allocation failed.");

    const int result = ArenaPoolInit(&m->pool, sizeof(Item), ITEM_MANAGER_CAPACITY);
    IVY_ASSERT(result == 0, "[ItemManager] Failed initialization Arena Pool.");

    return m;
}

void DestroyItemManager(ItemManager *manager)
{
    if (!manager) return;

    for (u32 i = 0; i < manager->count; i++) {
        const Item *item = manager->items[i];
        if (item->type == ITEM_EQUIPMENT) {
            UnloadTexture(item->data.equipment.iconTexture);
            UnloadTexture(item->data.equipment.charTexture);
            UnloadTexture(item->data.equipment.portraitTex);
        }
    }

    if (manager->table) {
        free(manager->table);
        manager->table = NULL;
    }

    ArenaPoolDestroy(&manager->pool);
    free(manager);
}

void BuildItemTable(ItemManager *manager)
{
    IVY_ASSERT(manager, "[ItemManager] Instance pointer is NULL.");

    if (manager->table) {
        free(manager->table);
    }

    manager->table = calloc(1, sizeof(ItemTable));
    IVY_ASSERT(manager->table, "[ItemManager] failed to alloc ItemTable.");

    for (u32 i = 0; i < manager->count; i++) {
        const Item *item = manager->items[i];
        if (item->type != ITEM_EQUIPMENT) continue;

        const EquipmentSlot slot = item->data.equipment.slot;
        SlotBucket *b = &manager->table->buckets[slot];

        if (b->count >= ITEM_MANAGER_CAPACITY) {
            TraceLog(LOG_WARNING, "[ItemManager] Item Table slot %d FULL!", slot);
            continue;
        }

        b->entries[b->count++] = item;
    }

    for (u32 s = 0; s < SLOT_MAX_SIZE; s++) {
        TraceLog(LOG_INFO, "[Table Item] %s: %u item(s)",
            EquipmentSlotName((EquipmentSlot)s),
            manager->table->buckets[s].count);
    }
}

const Item *ItemManagerFind(const ItemManager *manager, const u32 id)
{
    IVY_ASSERT(manager, "[ItemManager] Instance pointer is NULL");

    for (u32 i = 0; i < manager->count; i++) {
        if (manager->items[i]->id == id) return manager->items[i];
    }
    return NULL;
}

void LoadItemsFromFile(ItemManager *manager, const char *filename)
{
    if (manager->count >= ITEM_MANAGER_CAPACITY) return;

    FILE *f = fopen(filename, "rb");
    if (!f) {
        TraceLog(LOG_WARNING, "[ItemManager] Cannot open '%s'", filename);
        return;
    }

    unsigned int fileCount;
    fread(&fileCount, sizeof(unsigned int), 1, f);

    if (fileCount == 0) {
        fclose(f);
        return;
    }

    Item *it = ArenaPoolAlloc(&manager->pool);
    IVY_ASSERT(it, "[ItemManager] Fatal: Out of memory in ArenaPool. Max entities reached.");

    fread(&it->id,   sizeof(unsigned int), 1, f);
    fread(&it->type, sizeof(int),          1, f);
    fread(it->name,  1, 32, f);
    fread(it->desc,  1, 64, f);
    it->name[31] = '\0';
    it->desc[63] = '\0';

    if (it->type == ITEM_EQUIPMENT)
    {
        EquipmentData *eq = &it->data.equipment;
        char path[64];

        fread(path, 1, 64, f);  path[63] = '\0';
        eq->iconTexture = LoadTextureFromImageBin(path);

        fread(path, 1, 64, f);  path[63] = '\0';
        eq->charTexture = LoadTextureFromImageBin(path);

        fread(path, 1, 64, f);  path[63] = '\0';
        if (path[0] != '0') eq->portraitTex = LoadTextureFromImageBin(path);

        fread(&eq->position.x, sizeof(float), 1, f);
        fread(&eq->position.y, sizeof(float), 1, f);
        fread(&eq->slot,       sizeof(int),   1, f);
    }

    manager->items[manager->count++] = it;
    fclose(f);
}