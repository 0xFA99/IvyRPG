#include "ivy/item.h"
#include "ivy/utils.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

ItemManager *CreateItemManager(void)
{
    ItemManager *m = calloc(1, sizeof(ItemManager));
    assert(m && "[ERROR] Failed to alloc ItemManager");
    return m;
}

void DestroyItemManager(ItemManager *manager)
{
    if (!manager) return;

    for (u32 i = 0; i < manager->count; i++) {
        const Item *item = &manager->items[i];
        if (item->type == ITEM_EQUIPMENT) {
            UnloadTexture(item->data.equipment.iconTexture);
            UnloadTexture(item->data.equipment.charTexture);
            UnloadTexture(item->data.equipment.portraitTex);
        }
    }

    free(manager);
}

const Item *ItemManagerFind(const ItemManager *manager, const u32 id)
{
    assert(manager);
    for (u32 i = 0; i < manager->count; i++) {
        if (manager->items[i].id == id) return &manager->items[i];
    }
    return NULL;
}

void LoadItemsFromFile(ItemManager *manager, const char *filename)
{
    if (manager->count >= ITEM_MANAGER_CAPACITY) return;

    FILE *f = fopen(filename, "rb");
    if (!f) {
        TraceLog(LOG_WARNING, "LoadItemsFromFile: cannot open '%s'", filename);
        return;
    }

    unsigned int fileCount;
    fread(&fileCount, sizeof(unsigned int), 1, f);
    if (fileCount == 0) {
        fclose(f);
        return;
    }

    Item *it = &manager->items[manager->count];
    fread(&it->id,   sizeof(unsigned int), 1, f);
    fread(&it->type, sizeof(int),          1, f);
    fread(it->name,  1, 32, f);
    fread(it->desc,  1, 64, f);
    it->name[31] = '\0';
    it->desc[63] = '\0';

    if (it->type == ITEM_EQUIPMENT) {
        EquipmentData *eq = &it->data.equipment;
        char path[64];

        fread(path, 1, 64, f);  path[63] = '\0';
        eq->iconTexture = LoadTextureFromImageBin(path);

        fread(path, 1, 64, f);  path[63] = '\0';
        eq->charTexture = LoadTextureFromImageBin(path);

        fread(path, 1, 64, f);  path[63] = '\0';
        eq->portraitTex = LoadTextureFromImageBin(path);

        fread(&eq->position.x, sizeof(float), 1, f);
        fread(&eq->position.y, sizeof(float), 1, f);
        fread(&eq->slot,       sizeof(int),   1, f);
    }

    manager->count++;
    fclose(f);
}