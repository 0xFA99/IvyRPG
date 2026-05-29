#ifndef IVY_SCENES_GAMEPLAY_H
#define IVY_SCENES_GAMEPLAY_H

#include "ivy/utils/forward.h"
#include "ivy/systems/item_manager.h"
#include "ivy/graphics/camera.h"
#include "ivy/audio/wav.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    PAUSE_MENU_CLOSED = 0,
    PAUSE_MENU_OPENED,
} IvyGameplayState;

typedef enum {
    GAMEPLAY_MENU_RESUME    = 16,
    GAMEPLAY_MENU_SAVE      = 17,
    GAMEPLAY_MENU_LOAD      = 18,
    GAMEPLAY_MENU_INVENTORY = 20,
    GAMEPLAY_MENU_TITLE     = 19,
    GAMEPLAY_MENU_SIZE      = 5
} IvyGameplayMenuIndex;

typedef struct {
    Texture2D background;
    u8 selectedSlot;
    u8 scrollOffset;
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
    Texture2D background;
    Texture2D iconsAtlas;
    IvyTilemap  *tilemap;
    IvyPlayer   *player;
    IvyCamera   camera;
    IvyCollusionMap *collusionMap;
    IvyItemManager itemManager;

    IvyGameplayState state;

    IvyGameplayMenu menu;
    IvyInventoryUI inventoryUI;

    Music music;

    // IvyGameCamera    gameCamera;
    // Camera2D camera;
    // IvyCollision    *collision;
    // IvyPlayer       *player;
    // IvyItemManager  *itemManager;
    // IvyInventoryUI   inventoryUI;
    // IvyNPCManager   *npcManager;
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