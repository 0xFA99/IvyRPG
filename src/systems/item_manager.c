#include "ivy/arena/linear.h"
#include "ivy/audio/buffer.h"
#include "ivy/core/types.h"
#include "ivy/systems/asset_manager.h"
#include "ivy/systems/item_manager.h"
#include "ivy/utils/file_ids.h"
#include "ivy/utils/forward.h"

IvyItemManager Ivy_ItemManager_Init(IvyArenaLinear *restrict arena, IvyAssetManager *restrict assetManager)
{
    usize dataSize;
    const u8 *cur = Ivy_Asset_Get(assetManager, ASSET_ITEMS_BIN, &dataSize);

    IvyItemManager mgr = {0};

    u16 total;
    memcpy(&total, cur, sizeof(u16));
    cur += sizeof(u16);

    mgr.totalItem  = total;
    mgr.attributes = Ivy_Arena_LinearAlloc(arena, sizeof(IvyItemAttribute) * total);
    mgr.visuals    = Ivy_Arena_LinearAlloc(arena, sizeof(IvyItemVisual)    * total);
    mgr.texts      = Ivy_Arena_LinearAlloc(arena, sizeof(IvyItemText)      * total);

    const float iconSize = (float)IVY_ICON_SIZE;

    for (u16 i = 0; i < total; i++) {
        IvyItemAttribute *attribute = &mgr.attributes[i];
        IvyItemVisual    *visual    = &mgr.visuals[i];
        IvyItemText      *text      = &mgr.texts[i];

        memcpy(&attribute->id, cur, sizeof(u16));   cur += sizeof(u16);

        attribute->category = *cur++;
        attribute->type     = *cur++;
        attribute->slot     = *cur++;
        attribute->padding  = *cur++;

        u16 iconIndex;
        memcpy(&iconIndex, cur, sizeof(u16));  cur += sizeof(u16);
        visual->icon = (Rectangle) {
            .x      = (float)iconIndex * iconSize,
            .y      = 0.0f,
            .width  = iconSize,
            .height = iconSize
        };

        memcpy(&visual->spriteSheetID, cur, sizeof(u32));  cur += sizeof(u32);
        memcpy(&visual->portraitID,    cur, sizeof(u32));  cur += sizeof(u32);

        memcpy(&text->nameOffset, cur, sizeof(u16));  cur += sizeof(u16);
        memcpy(&text->descOffset, cur, sizeof(u16));  cur += sizeof(u16);
    }

    u16 poolSize;
    memcpy(&poolSize, cur, sizeof(u16));
    cur += sizeof(u16);

    mgr.stringPool = (const char *)cur;

    return mgr;
}

const IvyItemAttribute *Ivy_ItemManager_GetAttribute(const IvyItemManager *mgr, const u16 id)
{
    const u16 idx = id - 1;
    if (IVY_UNLIKELY(idx > mgr->totalItem)) return NULL;
    return &mgr->attributes[idx];
}

const IvyItemVisual *Ivy_ItemManager_GetVisual(const IvyItemManager *mgr, const u16 id)
{
    const u16 idx = id - 1;
    if (IVY_UNLIKELY(idx >= mgr->totalItem)) return NULL;
    return &mgr->visuals[idx];
}

const char *Ivy_ItemManager_GetName(const IvyItemManager *mgr, const u16 id)
{
    const u16 idx = id - 1;
    if (IVY_UNLIKELY(idx >= mgr->totalItem)) return "";
    return mgr->stringPool + mgr->texts[idx].nameOffset;
}

const char *Ivy_ItemManager_GetDesc(const IvyItemManager *mgr, const u16 id)
{
    const u16 idx = id - 1;
    if (IVY_UNLIKELY(idx >= mgr->totalItem)) return "";
    return mgr->stringPool + mgr->texts[idx].descOffset;
}
