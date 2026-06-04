#ifndef IVY_SCENES_GAMEPLAY_H
#define IVY_SCENES_GAMEPLAY_H

#include "ivy/audio/wav.h"
#include "ivy/core/types.h"
#include "ivy/entities/door.h"
#include "ivy/graphics/camera.h"
#include "ivy/systems/item_manager.h"
#include "ivy/utils/forward.h"

#define INVENTORY_SLOT_NONE 255
#define EQUIP_DEFAULT_SLOT  3

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    PAUSE_MENU_CLOSED = 0,
    PAUSE_MENU_OPENED,
    PAUSE_MENU_INVENTORY
} IvyGameplayState;

typedef enum {
    GAMEPLAY_MENU_RESUME    = 16,
    GAMEPLAY_MENU_SAVE      = 17,
    GAMEPLAY_MENU_LOAD      = 18,
    GAMEPLAY_MENU_INVENTORY = 20,
    GAMEPLAY_MENU_TITLE     = 19,
    GAMEPLAY_MENU_SIZE      = 5
} IvyGameplayMenuIndex;

typedef enum {
    INVENTORY_FOCUS_ITEM_LIST = 0,
    INVENTORY_FOCUS_EQUIP_SLOTS
} IvyInventoryFocus;

typedef struct {
    Texture2D background;
    IvySound sound[2];
    u8 selectedSlot;
    u8 scrollOffset;
    u8 selectedEquip;
    u8 focus;
    u8 visibleRows;
    u8 totalRows;
    bool showDescription;
} IvyInventoryUI;

typedef struct {
    const char *menuStrings[GAMEPLAY_MENU_SIZE];
    u32         menuLengths[GAMEPLAY_MENU_SIZE];
    char        selected;
    IvySound    sound;
} IvyGameplayMenu;

struct IvySceneGameplayData {
    IvyTilemap  *tilemap;
    IvyPlayer   *player;
    IvyCamera   camera;
    IvyCollusionMap *collusionMap;
    IvyItemManager itemManager;
    IvyGameplayState state;
    IvyGameplayMenu menu;
    IvyInventoryUI inventoryUI;
    Music music;
    IvyDoor door;
};

void Ivy_Scene_GameplayInit             (IvyGame *game);
void Ivy_Scene_GameplayUpdate           (IvyGame *game);
void Ivy_Scene_GameplayDrawWorld        (IvyGame *game);
void Ivy_Scene_GameplayDrawUI           (IvyGame *game);
void Ivy_Scene_GameplayRebuildTextures  (IvyGame *game);
void Ivy_Scene_GameplayUnload           (IvySceneManager *sm);

#ifdef __cplusplus
}
#endif

#endif