#include "ivy/entities/player.h"
#include "ivy/core/game.h"
#include "ivy/systems/texture_manager.h"
#include "ivy/scenes/gameplay.h"
#include "ivy/graphics/gfx.h"
#include "ivy/utils/file_ids.h"
#include "raylib/rlgl.h"

#define PLAYER_SPRITESHEET_ATLAS_WIDTH 768
#define PLAYER_SPRITESHEET_ATLAS_HEIGHT 512

// Temporary Test
static const u8 BAKE_SLOT_ORDER[] = {
    IVY_SLOT_EXT_1,
    IVY_SLOT_BOT,
    IVY_SLOT_MID,
    IVY_SLOT_ACC,
    IVY_SLOT_HAIR,
    IVY_SLOT_M_ARM,
    IVY_SLOT_S_ARM
};

void Ivy_Player_BakeAtlas(IvyGame *restrict game, IvySceneGameplayData *restrict gameplayData)
{
    IvyPlayer *player = gameplayData->player;
    IvyAssetManager *assetManager = game->assets;
    const IvyItemManager *itemManager = &gameplayData->itemManager;
    const IvyTextureManager *textureManager = game->texManager;

    if (player->graphics.atlasReady) {
        rlUnloadTexture(player->graphics.atlas.texture.id);
        rlUnloadFramebuffer(player->graphics.atlas.id);
    }

    player->graphics.atlas = LoadRenderTexture(
        PLAYER_SPRITESHEET_ATLAS_WIDTH,
        PLAYER_SPRITESHEET_ATLAS_HEIGHT
    );

    BeginTextureMode(player->graphics.atlas);
    ClearBackground(BLANK);
    DrawTexture(Ivy_TextureManager_Get(textureManager, ASSET_TEXTURES_SPRITESHEETS_BASE_BODY_DDS), 0, 0, WHITE);
    DrawTexture(Ivy_TextureManager_Get(textureManager, ASSET_TEXTURES_SPRITESHEETS_BASE_HEAD_DDS), 0, 0, WHITE);

    for (u32 i = 0; i < (u32)(sizeof(BAKE_SLOT_ORDER) / sizeof(BAKE_SLOT_ORDER[0])); i++) {
        const u8  eSlot  = BAKE_SLOT_ORDER[i];
        const u16 itemID = Ivy_Inventory_GetEquippedItemID(&player->inventory, eSlot);

        // if (itemID == 0) {
        //     if (eSlot == IVY_SLOT_HAIR) {
        //         DrawTexture(player->graphics.baseHair, 0, 0, WHITE);
        //     }
        //     continue;
        // }
        if (eSlot != 0) {
            const IvyItemVisual *vis = Ivy_ItemManager_GetVisual(itemManager, itemID);
            if (!vis) continue;

            const Texture2D tex = Ivy_Gfx_LoadTextureDDS(assetManager, vis->spriteSheetID);
            DrawTexture(tex, 0, 0, WHITE);
        }
    }

    DrawTexture(Ivy_TextureManager_Get(textureManager, ASSET_TEXTURES_SPRITESHEETS_BASE_HAIR_DDS), 0, 0, WHITE);

    EndTextureMode();

    player->graphics.atlasReady = true;
}

IvyPlayer *Ivy_Player_Init(IvyArenaLinear *restrict arena, IvyAssetManager *restrict assetManager, const Vector2 position)
{
    IvyPlayer *player = Ivy_Arena_LinearAllocZero(arena, sizeof(IvyPlayer));

    player->movement.tilePosition = position;
    player->movement.position = (Vector2){ position.x * IVY_TILE_SIZE, position.y * IVY_TILE_SIZE };

    player->graphics.direction = IVY_DIRECTION_DOWN;
    player->graphics.atlasReady = false;

    player->stepSound[0] = Ivy_Audio_LoadSoundWav(arena, assetManager, ASSET_AUDIO_STEP1_WAV);
    player->stepSound[1] = Ivy_Audio_LoadSoundWav(arena, assetManager, ASSET_AUDIO_STEP1_WAV);
    player->stepSound[2] = Ivy_Audio_LoadSoundWav(arena, assetManager, ASSET_AUDIO_STEP2_WAV);
    player->stepSound[3] = Ivy_Audio_LoadSoundWav(arena, assetManager, ASSET_AUDIO_STEP3_WAV);

    player->inventory = Ivy_Inventory_Init();

    return player;
}

Vector2 Ivy_Player_GetPosition(const IvyPlayer *player)
{
    return player->movement.position;
}

IvyInventory *Ivy_Player_GetInventory(IvyPlayer *player)
{
    return &player->inventory;
}

void Ivy_Player_EquipItem(IvyGame *restrict game, IvySceneGameplayData *restrict gameplayData, const u8 bagIndex)
{
    IvyPlayer *player = gameplayData->player;
    const IvyItemManager *itemManager = &gameplayData->itemManager;
    const IvyInventoryBag *bag = &player->inventory.bag;

    if (bagIndex >= bag->count) return;

    const u16 itemID = bag->slot[bagIndex].itemID;
    const IvyItemAttribute *attr = Ivy_ItemManager_GetAttribute(itemManager, itemID);
    if (!attr) return;

    Ivy_Inventory_Equip(&player->inventory, bagIndex, attr->slot);
    Ivy_Player_BakeAtlas(game, gameplayData);
}

void Ivy_Player_UnequipItem(IvyGame *restrict game, IvySceneGameplayData *restrict gameplayData, const u8 equipSlot)
{
    IvyPlayer *player = gameplayData->player;
    Ivy_Inventory_Unequip(&player->inventory, equipSlot);
    Ivy_Player_BakeAtlas(game, gameplayData);
}

void Ivy_Player_Unload(IvyPlayer *player)
{
    rlUnloadTexture(player->graphics.atlas.texture.id);
    rlUnloadFramebuffer(player->graphics.atlas.id);

    player->graphics.atlasReady = false;

    for (int i = 0; i < 4; i++) {
        if (player->stepSound[i].data.stream.buffer != NULL) {
            Ivy_Audio_UnloadSound(&player->stepSound[i]);
            player->stepSound[i].data.stream.buffer = NULL;
        }
    }
}
