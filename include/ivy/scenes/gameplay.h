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
    GAMEPLAY_CLOSE_MENU = 0,
    GAMEPLAY_OPEN_MENU
} IvyGameplayState;

typedef enum {
    GAMEPLAY_MENU_RESUME = 16,
    GAMEPLAY_MENU_SAVE = 17,
    GAMEPLAY_MENU_LOAD = 18,
    GAMEPLAY_MENU_TITLE = 19,
    GAMEPLAY_MENU_SIZE = 4
} IvyGameplayMenuIndex;

typedef struct {
    const char *menuStrings[GAMEPLAY_MENU_SIZE];
    u32         menuLengths[GAMEPLAY_MENU_SIZE];
    char        selected;

    IvySound    sound;
} IvyGameplayMenu;

typedef struct {
    IvyTilemap  *tilemap;
    IvyPlayer   *player;
    IvyCamera   camera;
    IvyCollusionMap *collusionMap;
    IvyItemManager itemManager;

    IvyGameplayState state;

    IvyGameplayMenu menu;

    Music music;

    // IvyGameCamera    gameCamera;
    // Camera2D camera;
    // IvyCollision    *collision;
    // IvyPlayer       *player;
    // IvyItemManager  *itemManager;
    // IvyInventoryUI   inventoryUI;
    // IvyNPCManager   *npcManager;
} IvySceneGameplayData;

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