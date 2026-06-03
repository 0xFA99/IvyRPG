#include "ivy/arena/linear.h"
#include "ivy/audio/buffer.h"
#include "ivy/audio/ogg.h"
#include "ivy/audio/stream.h"
#include "ivy/audio/wav.h"
#include "ivy/core/game.h"
#include "ivy/core/types.h"
#include "ivy/entities/player.h"
#include "ivy/graphics/camera.h"
#include "ivy/graphics/collusion.h"
#include "ivy/graphics/gfx.h"
#include "ivy/graphics/tilemap.h"
#include "ivy/scenes/gameplay.h"
#include "ivy/scenes/types.h"
#include "ivy/systems/inventory.h"
#include "ivy/systems/item_manager.h"
#include "ivy/systems/locale_manager.h"
#include "ivy/systems/scene_manager.h"
#include "ivy/utils/file_ids.h"
#include "ivy/utils/forward.h"

#include "raylib/rlgl.h"

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

    IvySceneGameplayData *gameplayData = Ivy_Arena_LinearAllocZero(&game->arena, sizeof(IvySceneGameplayData));
    IVY_ASSERT(gameplayData != NULL, "[Scene Gameplay] Failed to allocate SceneGameplayData!");

    gameplayData->tilemap      = Ivy_Tilemap_LoadMap(game->assets, &game->arena, ASSET_MAPS_MAP_1_METADATA_BIN, ASSET_MAPS_MAP_1_VERTEX_BIN);
    gameplayData->collusionMap = Ivy_Collusion_Load(&game->arena, game->assets);
    gameplayData->player       = Ivy_Player_Init(&game->arena, game->assets, (Vector2){ 10.0f, 16.0f });
    gameplayData->camera       = Ivy_Camera_Init();
    gameplayData->itemManager  = Ivy_ItemManager_Init(&game->arena, game->assets);
    gameplayData->state        = PAUSE_MENU_CLOSED;

    // test
    const IvyItemAttribute *shirtAttr = Ivy_ItemManager_GetAttribute(&gameplayData->itemManager, 1);
    const IvyItemAttribute *pantsAttr = Ivy_ItemManager_GetAttribute(&gameplayData->itemManager, 2);
    const IvyItemAttribute *pantyAttr = Ivy_ItemManager_GetAttribute(&gameplayData->itemManager, 3);
    const IvyItemAttribute *cloak = Ivy_ItemManager_GetAttribute(&gameplayData->itemManager, 4);

    Ivy_Inventory_AddItem(&gameplayData->player->inventory.bag, shirtAttr->id, shirtAttr->type, 3);
    Ivy_Inventory_AddItem(&gameplayData->player->inventory.bag, pantsAttr->id, pantsAttr->type, 2);
    Ivy_Inventory_AddItem(&gameplayData->player->inventory.bag, pantyAttr->id, pantyAttr->type, 2);
    Ivy_Inventory_AddItem(&gameplayData->player->inventory.bag, cloak->id, cloak->type, 2);

    Ivy_Player_EquipItem(game, gameplayData, 0);
    Ivy_Player_EquipItem(game, gameplayData, 1);

    gameplayData->inventoryUI.sound[0]        = Ivy_Audio_LoadSoundWav(&game->arena, game->assets, ASSET_AUDIO_EQUIP2_WAV);
    gameplayData->inventoryUI.sound[1]        = Ivy_Audio_LoadSoundWav(&game->arena, game->assets, ASSET_AUDIO_EQUIP3_WAV);
    gameplayData->inventoryUI.selectedSlot    = INVENTORY_SLOT_NONE;
    gameplayData->inventoryUI.scrollOffset    = 0;
    gameplayData->inventoryUI.visibleRows     = 4;
    gameplayData->inventoryUI.showDescription = true;

    for (u32 i = 0; i < GAMEPLAY_MENU_SIZE; i++) {
        gameplayData->menu.menuStrings[i] = IVY_TR(game->locale, (IvyLocaleKey)MENU_GAMEPLAY_PAUSE[i]);
        gameplayData->menu.menuLengths[i] = IVY_TR_LEN(game->locale, (IvyLocaleKey)MENU_GAMEPLAY_PAUSE[i]);
    }

    gameplayData->menu.selected = 0;
    gameplayData->menu.sound    = Ivy_Audio_LoadSoundWav(&game->arena, game->assets, ASSET_AUDIO_CURSOR_WAV);

    // audio
    gameplayData->music = Ivy_Audio_LoadMusicOGG(&game->arena, game->assets, ASSET_MUSIC_POINT_AND_CLICK_OGG, 192560);
    Ivy_Audio_PlayAudioBuffer(gameplayData->music.stream.buffer);

    gameplayData->inventoryUI.background     = Ivy_Gfx_LoadTextureDDS(game->assets, ASSET_TEXTURES_MENU_EQUIPMENT_TEMPLATE_DDS);
    game->scenes->actionScene->data = gameplayData;
}

void Ivy_Scene_GameplayUnload(IvySceneManager *sm)
{
    if (IVY_UNLIKELY(!sm->actionScene || !sm->actionScene->data)) return;

    const IvySceneGameplayData *gd = sm->actionScene->data;

    rlUnloadTexture(gd->inventoryUI.background.id);

    Ivy_Tilemap_Unload(gd->tilemap);
    Ivy_Collusion_Unload(gd->collusionMap);
    Ivy_Player_Unload(gd->player);

    Ivy_Audio_UnloadSound(&gd->menu.sound);
    Ivy_Audio_UnloadStream(&gd->music);

    Ivy_Audio_UnloadSound(&gd->inventoryUI.sound[0]);
    Ivy_Audio_UnloadSound(&gd->inventoryUI.sound[1]);

    sm->actionScene->data = NULL;
}
