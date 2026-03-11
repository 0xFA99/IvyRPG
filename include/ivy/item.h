#ifndef IVY_ITEM_H
#define IVY_ITEM_H

#include "ivy/types.h"
#include "raylib/raylib.h"

#define ITEM_MANAGER_CAPACITY 256

typedef enum {
    SLOT_HEAD = 0,
    SLOT_TOP,
    SLOT_ACC,
    SLOT_M_ARM,
    SLOT_S_ARM,
    SLOT_MID_EXT,
    SLOT_MID,
    SLOT_BOT,
    SLOT_TOP_EXT,
    SLOT_EXT_1,
    SLOT_MAX_SIZE
} EquipmentSlot;

static inline const char *EquipmentSlotName(const EquipmentSlot slot)
{
    switch (slot) {
        case SLOT_HEAD:    return "Head";
        case SLOT_TOP:     return "Top";
        case SLOT_ACC:     return "Accessory";
        case SLOT_M_ARM:   return "Main Arm";
        case SLOT_S_ARM:   return "Sub Arm";
        case SLOT_MID_EXT: return "Mid Ext";
        case SLOT_MID:     return "Mid";
        case SLOT_BOT:     return "Bottom";
        case SLOT_TOP_EXT: return "Top Ext";
        case SLOT_EXT_1:   return "Extra";
        default:           return "Unknown";
    }
}

typedef enum {
    ITEM_NONE = 0,
    ITEM_EQUIPMENT
} ItemType;

typedef struct {
    Texture2D       iconTexture;
    Texture2D       charTexture;
    Texture2D       portraitTex;
    Vector2         position;
    EquipmentSlot   slot;
} EquipmentData;

typedef struct {
    u32             id;
    ItemType        type;
    char            name[32];
    char            desc[64];

    union {
        EquipmentData equipment;
    } data;
} Item;

typedef struct {
    Item    items[ITEM_MANAGER_CAPACITY];
    u32     count;
} ItemManager;

ItemManager    *CreateItemManager(void);
void            DestroyItemManager(ItemManager *manager);

const Item     *ItemManagerFind(const ItemManager *manager, u32 id);
void            LoadItemsFromFile(ItemManager *manager, const char *filename);


#endif