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

static const u16 MENU_GAMEPLAY_PAUSE[GAMEPLAY_MENU_SIZE] = {
    GAMEPLAY_MENU_RESUME,
    GAMEPLAY_MENU_SAVE,
    GAMEPLAY_MENU_LOAD,
    GAMEPLAY_MENU_INVENTORY,
    GAMEPLAY_MENU_TITLE,
};

void Ivy_Scene_GameplayInit(IvyGame *game)
{
    IVY_ASSERT(game != NULL, "[Scene Gameplay] Game pointer is NULL!");

    IvySceneGameplayData *gd = Ivy_Arena_LinearAllocZero(&game->arena, sizeof(IvySceneGameplayData));
    IVY_ASSERT(gd != NULL, "[Scene Gameplay] Failed to allocate SceneGameplayData!");

    // --- World ---
    gd->tilemap      = Ivy_Tilemap_LoadMap(game->assets, &game->arena, ASSET_MAPS_MAP_1_METADATA_BIN, ASSET_MAPS_MAP_1_VERTEX_BIN);
    gd->collusionMap = Ivy_Collusion_Load(&game->arena, game->assets);
    gd->player       = Ivy_Player_Init(&game->arena, game->assets, (Vector2){ 10.0f, 16.0f });
    gd->camera       = Ivy_Camera_Init();
    gd->itemManager  = Ivy_ItemManager_Init(&game->arena, game->assets);
    gd->state        = PAUSE_MENU_CLOSED;

    // --- Isi awal inventory (test data) ---
    const IvyItemAttribute *shirtAttr = Ivy_ItemManager_GetAttribute(&gd->itemManager, 1);
    const IvyItemAttribute *pantsAttr = Ivy_ItemManager_GetAttribute(&gd->itemManager, 2);

    Ivy_Inventory_AddItem(&gd->player->inventory.bag, shirtAttr->id, shirtAttr->type, 3);
    Ivy_Inventory_AddItem(&gd->player->inventory.bag, pantsAttr->id, pantsAttr->type, 2);

    Ivy_Player_EquipItem(gd->player, game->assets, &gd->itemManager, 0);
    Ivy_Player_EquipItem(gd->player, game->assets, &gd->itemManager, 1);

    // --- Inventory UI ---
    gd->inventoryUI.sound[0]        = Ivy_Audio_LoadSoundWav(&game->arena, game->assets, ASSET_AUDIO_EQUIP2_WAV);
    gd->inventoryUI.sound[1]        = Ivy_Audio_LoadSoundWav(&game->arena, game->assets, ASSET_AUDIO_EQUIP3_WAV);
    gd->inventoryUI.selectedSlot    = INVENTORY_SLOT_NONE;
    gd->inventoryUI.scrollOffset    = 0;
    gd->inventoryUI.visibleRows     = 4;
    gd->inventoryUI.showDescription = true;

    // --- Pause menu ---
    for (u32 i = 0; i < GAMEPLAY_MENU_SIZE; i++) {
        gd->menu.menuStrings[i] = IVY_TR    (game->locale, (IvyLocaleKey)MENU_GAMEPLAY_PAUSE[i]);
        gd->menu.menuLengths[i] = IVY_TR_LEN(game->locale, (IvyLocaleKey)MENU_GAMEPLAY_PAUSE[i]);
    }
    gd->menu.selected = 0;
    gd->menu.sound    = Ivy_Audio_LoadSoundWav(&game->arena, game->assets, ASSET_AUDIO_CURSOR_WAV);

    // --- Audio & Textures ---
    gd->music = Ivy_Audio_LoadMusicOGG(&game->arena, game->assets, ASSET_MUSIC_POINT_AND_CLICK_OGG, 192560);
    Ivy_Audio_PlayAudioBuffer(gd->music.stream.buffer);

    gd->background                 = Ivy_Gfx_LoadTextureDDS(game->assets, ASSET_TEXTURES_BACKGROUND_DDS);
    gd->inventoryUI.background     = Ivy_Gfx_LoadTextureDDS(game->assets, ASSET_TEXTURES_MENU_EQUIPMENT_TEMPLATE_DDS);
    gd->iconsAtlas                 = Ivy_Gfx_LoadTextureDDS(game->assets, ASSET_TEXTURES_ICONS_DDS);

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