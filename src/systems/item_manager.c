#include "ivy/core/types.h"
#include "ivy/arena/linear.h"
#include "ivy/systems/item_manager.h"
#include "ivy/systems/asset_manager.h"
#include "ivy/graphics/gfx.h"
#include "ivy/utils/file_ids.h"

IvyItemManager Ivy_ItemManager_Init(IvyArenaLinear *restrict arena, IvyAssetManager *restrict assetManager)
{
    usize items_size;
    const u8 *header = Ivy_Asset_Get(assetManager, ASSET_ITEMS_BIN, &items_size);

    IvyItemManager itemManager = {0};

    // read total count once
    u16 totalItem;
    memcpy(&totalItem, header, sizeof(u16));
    header += sizeof(u16);

    itemManager.totalItem = totalItem;
    itemManager.items     = Ivy_Arena_LinearAlloc(arena, sizeof(IvyItem) * totalItem);

    // cache icon dimensions to avoid repeated calculations
    const float iconSize = (float)IVY_ICON_SIZE;

    u16 id, iconIndex;

    IvyItem *items = itemManager.items;

    for (u16 i = 0; i < totalItem; i++)
    {
        IvyItem *item = &items[i];

        memcpy(&id, header, sizeof(u16));
        header += sizeof(u16);

        const u8 category = *header++;
        const u8 type     = *header++;
        const u8 slot     = *header++;

        item->id        = id;
        item->category  = category;
        item->type      = type;
        item->slot      = slot;

        // copy strings
        memcpy(item->name, header, 32);
        header += 32;

        memcpy(item->description, header, 64);
        header += 64;

        // icon rectangle
        memcpy(&iconIndex, header, sizeof(u16));
        header += sizeof(u16);

        item->icon = (Rectangle) {
            .x = (float)iconIndex * iconSize,
            .y = 0,
            .width = iconSize,
            .height = iconSize
        };

        memcpy(&item->spriteSheet, header, sizeof(u32));
        header += sizeof(u32);

        memcpy(&item->portrait, header, sizeof(u32));
        header += sizeof(u32);
    }

    return itemManager;
}
