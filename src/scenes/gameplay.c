#include "ivy/scenes/gameplay.h"

#include "ivy/core/types.h"
#include "ivy/core/game.h"
#include "ivy/core/keybind.h"
#include "ivy/arena/linear.h"
#include "ivy/audio/buffer.h"
#include "ivy/audio/ogg.h"
#include "ivy/audio/stream.h"
#include "ivy/systems/scene_manager.h"
#include "ivy/graphics/tilemap.h"
#include "ivy/entities/player.h"
#include "ivy/graphics/collusion.h"
#include "ivy/graphics/gfx.h"
#include "ivy/systems/locale_manager.h"
#include "ivy/utils/file_ids.h"
#include "ivy/systems/inventory.h"
#include "ivy/systems/profile_manager.h"

#include <stdio.h>

enum {
    POPUP_WIDTH      = 200,
    POPUP_HEIGHT     = 160,
    INVENTORY_WIDTH  = 320,
    INVENTORY_HEIGHT = 280,
    ITEM_ICON_SIZE   = 18,
    SLOT_PADDING     = 8
};

const u16 MENU_GAMEPLAY_PAUSE[] = { 16, 17, 18, 20, 19 };

void Ivy_Scene_GameplayInit(IvyGame *game)
{
    IVY_ASSERT(game != NULL, "[Scene Gameplay] Arena not found!");

    IvySceneGameplayData *gd = Ivy_Arena_LinearAllocZero(&game->arena, sizeof(IvySceneGameplayData));
    IVY_ASSERT(gd != NULL, "[Scene Gameplay] Failed to allocate SceneGameplayData!");

    gd->tilemap      = Ivy_Tilemap_LoadMap(game->assets, &game->arena, ASSET_MAPS_MAP_1_METADATA_BIN, ASSET_MAPS_MAP_1_VERTEX_BIN);
    gd->collusionMap = Ivy_Collusion_Load(&game->arena, game->assets);
    gd->player       = Ivy_Player_Init(&game->arena, game->assets, (Vector2){ 10.0f, 16.0f });
    gd->camera       = Ivy_Camera_Init();
    gd->itemManager  = Ivy_ItemManager_Init(&game->arena, game->assets);
    gd->state        = PAUSE_MENU_CLOSED;

    // Init inventory UI
    gd->inventoryUI.sound[0] = Ivy_Audio_LoadSoundWav(&game->arena, game->assets, ASSET_AUDIO_EQUIP2_WAV);
    gd->inventoryUI.sound[1] = Ivy_Audio_LoadSoundWav(&game->arena, game->assets, ASSET_AUDIO_EQUIP3_WAV);
    gd->inventoryUI.selectedSlot    = 255;
    gd->inventoryUI.scrollOffset    = 0;
    gd->inventoryUI.visibleRows     = 4;
    gd->inventoryUI.showDescription = true;

    // IvyInventory *inv = Ivy_Player_GetInventory(gd->player);

    const IvyItemAttribute *shirtAttr = Ivy_ItemManager_GetAttribute(&gd->itemManager, 1);
    const IvyItemAttribute *pantsAttr = Ivy_ItemManager_GetAttribute(&gd->itemManager, 2);
    // const IvyItemAttribute *pantyAttr = Ivy_ItemManager_GetAttribute(&gd->itemManager, 3);

    // i32 shirtBagIdx = -1, pantsBagIdx = -1, pantyBagIdx = -1;
    // i32 shirtBagIdx = -1;
    // if (shirtAttr) shirtBagIdx = Ivy_Inventory_AddItem(&gd->player->inventory.bag, 1, shirtAttr->type, 1);
    Ivy_Inventory_AddItem(&gd->player->inventory.bag, shirtAttr->id, shirtAttr->type, 3);
    Ivy_Inventory_AddItem(&gd->player->inventory.bag, pantsAttr->id, pantsAttr->type, 2);
    // if (pantsAttr) pantsBagIdx = Ivy_Inventory_AddItem(&gd->player->inventory.bag, 2, pantsAttr->type, 1);
    // if (pantyAttr) pantyBagIdx = Ivy_Inventory_AddItem(&gd->player->inventory.bag, 3, pantyAttr->type, 1);

    // if (shirtBagIdx >= 0) {
        Ivy_Player_EquipItem(gd->player, game->assets, &gd->itemManager, 0);
    // }
    Ivy_Player_EquipItem(gd->player, game->assets, &gd->itemManager, 1);

    for (u32 i = 0; i < GAMEPLAY_MENU_SIZE; i++) {
        gd->menu.menuStrings[i] = IVY_TR    (game->locale, (IvyLocaleKey)MENU_GAMEPLAY_PAUSE[i]);
        gd->menu.menuLengths[i] = IVY_TR_LEN(game->locale, (IvyLocaleKey)MENU_GAMEPLAY_PAUSE[i]);
    }

    gd->menu.selected = 0;
    gd->menu.sound    = Ivy_Audio_LoadSoundWav(&game->arena, game->assets, ASSET_AUDIO_CURSOR_WAV);
    gd->music         = Ivy_Audio_LoadMusicOGG(&game->arena, game->assets, ASSET_MUSIC_POINT_AND_CLICK_OGG, 192560);

    Ivy_Audio_PlayAudioBuffer(gd->music.stream.buffer);

    gd->background = Ivy_Gfx_LoadTextureDDS(game->assets, ASSET_TEXTURES_BACKGROUND_DDS);
    gd->inventoryUI.background = Ivy_Gfx_LoadTextureDDS(game->assets, ASSET_TEXTURES_MENU_EQUIPMENT_TEMPLATE_DDS);
    gd->iconsAtlas = Ivy_Gfx_LoadTextureDDS(game->assets, ASSET_TEXTURES_ICONS_DDS);

    game->scenes->actionScene->data = gd;
}

void Ivy_Scene_GameplayUnload(IvySceneManager *sm)
{
    if (IVY_UNLIKELY(!sm->actionScene || !sm->actionScene->data)) return;

    const IvySceneGameplayData *gd = sm->actionScene->data;

    Ivy_Tilemap_Unload(gd->tilemap);
    Ivy_Collusion_Unload(gd->collusionMap);
    Ivy_Player_Unload(gd->player);

    Ivy_Audio_UnloadSound(&gd->menu.sound);
    Ivy_Audio_UnloadStream(&gd->music);

    sm->actionScene->data = NULL;
}
