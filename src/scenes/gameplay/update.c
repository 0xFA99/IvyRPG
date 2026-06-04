#include "ivy/arena/types.h"
#include "ivy/audio/buffer.h"
#include "ivy/audio/ogg.h"
#include "ivy/audio/wav.h"
#include "ivy/core/game.h"
#include "ivy/core/keybind.h"
#include "ivy/core/types.h"
#include "ivy/entities/player.h"
#include "ivy/graphics/camera.h"
#include "ivy/graphics/tilemap.h"
#include "ivy/scenes/gameplay.h"
#include "ivy/scenes/types.h"
#include "ivy/systems/inventory.h"
#include "ivy/systems/scene_manager.h"
#include "ivy/utils/forward.h"

static const i8 EQUIP_NAVIGATION[13][4] = {
    //      L   R    U   D
     { 1,  3,  -1,  2 },
     {-1,  2,   0,  4 },
     { 1,  3,   0,  5 },
     { 2, -2,   0,  6 },
     {-1,  5,   1,  7 },
     { 4,  6,   2,  8 },
     { 5, -2,   3,  9 },
     {-1,  8,   4, 10 },
     { 7,  9,   5, 11 },
     { 8, -2,   6, 12 },
     {-1, 11,   7, -1 },
     {10, 12,   8, -1 },
     {11, -2,   9, -1 },
};

IVY_INLINE u8 EquipSlotToItemRow(const u8 equipSlot)
{
    return (equipSlot / 3) - 1;
}

static void GameplayInventory(IvyGame *game)
{
    IvySceneGameplayData *gameplayData = game->scenes->actionScene->data;
    IvyInventoryUI *ui = &gameplayData->inventoryUI;

    const IvyInventory *inventory = Ivy_Player_GetInventory(gameplayData->player);
    const IvyInventoryBag *bag = &inventory->bag;
    const u8 equipmentCount = bag->categoryCount[IVY_ITEM_TYPE_EQUIPMENT];

    // close inventory
    if (IsKeyPressed(game->keybind[IVY_KEY_CANCEL].currentKey)) {
        gameplayData->state = PAUSE_MENU_OPENED;
        ui->selectedSlot = INVENTORY_SLOT_NONE;

        Ivy_Audio_PlayAudioBuffer(ui->sound[1].data.stream.buffer);

        return;
    }

    // item list
    if (ui->focus == INVENTORY_FOCUS_ITEM_LIST && equipmentCount > 0)
    {
        if (ui->selectedSlot >= equipmentCount)
            ui->selectedSlot = 0;

        if (IsKeyPressed(game->keybind[IVY_KEY_CONFIRM].currentKey)) {
            const u8 catOffset = bag->categoryOffset[IVY_ITEM_TYPE_EQUIPMENT];
            const u8 bagIndex  = bag->categoryIndices[catOffset + ui->selectedSlot];

            bool wasEquipped = false;
            for (usize s = 0; s < IVY_SLOT_MAX; s++) {
                if (inventory->equipped.index[s] == bagIndex) {
                    Ivy_Player_UnequipItem(game, gameplayData, (u8)s);
                    wasEquipped = true;
                    break;
                }
            }

            if (!wasEquipped) {
                Ivy_Player_EquipItem(game, gameplayData, bagIndex);
            }

            Ivy_Audio_PlayAudioBuffer(ui->sound[0].data.stream.buffer);
            return;
        }

        if (IsKeyPressed(KEY_RIGHT)) {
            if ((ui->selectedSlot % 2 == 0) && (ui->selectedSlot + 1 < equipmentCount))
                ui->selectedSlot += 1;
        }
        // move to slot layout
        else if (IsKeyPressed(KEY_LEFT)) {
            if (ui->selectedSlot % 2 == 0) {
                ui->focus = INVENTORY_FOCUS_EQUIP_SLOTS;
                ui->selectedEquip = EQUIP_DEFAULT_SLOT;
            } else {
                ui->selectedSlot -= 1;
            }
        }
        else if (IsKeyPressed(KEY_DOWN)) {
            if (ui->selectedSlot + 2 < equipmentCount)
                ui->selectedSlot += 2;
        }
        else if (IsKeyPressed(KEY_UP)) {
            if (ui->selectedSlot >= 2)
                ui->selectedSlot -= 2;
        }
        return;
    }

    // slot layout
    if (ui->focus == INVENTORY_FOCUS_EQUIP_SLOTS)
    {
        if (IsKeyPressed(game->keybind[IVY_KEY_CONFIRM].currentKey)) {
            const u8 equipSlot = ui->selectedEquip;
            const u16 itemID = Ivy_Inventory_GetEquippedItemID(
                Ivy_Player_GetInventory(gameplayData->player), equipSlot
            );
            if (itemID != 0) {
                Ivy_Player_UnequipItem(game, gameplayData, equipSlot);
                Ivy_Audio_PlayAudioBuffer(ui->sound[0].data.stream.buffer);
            }
            return;
        }

        const i8 *nav = EQUIP_NAVIGATION[ui->selectedEquip];
        i8 next = -1;

        if      (IsKeyPressed(KEY_LEFT))  next = nav[0];
        else if (IsKeyPressed(KEY_RIGHT)) next = nav[1];
        else if (IsKeyPressed(KEY_UP))    next = nav[2];
        else if (IsKeyPressed(KEY_DOWN))  next = nav[3];

        // move to item list layout
        if (next == -2) {
            u8 targetSlot = EquipSlotToItemRow(ui->selectedEquip) * 2;
            if (targetSlot >= equipmentCount) targetSlot = 0;

            ui->selectedSlot = targetSlot;
            ui->focus        = INVENTORY_FOCUS_ITEM_LIST;
        }
        else if (next >= 0) {
            ui->selectedEquip = (u8)next;
        }
    }
}

static void GameplayUpdateStatePause(IvyGame *game)
{
    IvySceneGameplayData *gameplayData = game->scenes->actionScene->data;

    const int direction = IsKeyPressed(game->keybind[IVY_KEY_DOWN].currentKey)
                        - IsKeyPressed(game->keybind[IVY_KEY_UP].currentKey);

    if (direction) {
        gameplayData->menu.selected = (gameplayData->menu.selected + direction + GAMEPLAY_MENU_SIZE) % GAMEPLAY_MENU_SIZE;
        Ivy_Audio_PlayAudioBuffer(gameplayData->menu.sound.data.stream.buffer);
    }

    if (!IsKeyPressed(game->keybind[IVY_KEY_CONFIRM].currentKey)) return;

    switch (gameplayData->menu.selected) {
        case 0:
            gameplayData->state = PAUSE_MENU_CLOSED;
            break;

        case 3:
            gameplayData->state = PAUSE_MENU_INVENTORY;
            gameplayData->inventoryUI.selectedSlot = 0;
            gameplayData->inventoryUI.focus = INVENTORY_FOCUS_ITEM_LIST;
            break;

        case 4:
            Ivy_SceneManager_Transition(game, SCENE_TITLE);
            break;

        default: break;
    }
}

void Ivy_Scene_GameplayUpdate(IvyGame *game)
{
    IvySceneGameplayData *gameplayData = game->scenes->actionScene->data;

    Ivy_Audio_UpdateMusicOGG(&gameplayData->music);

    if (gameplayData->state == PAUSE_MENU_INVENTORY) {
        GameplayInventory(game);
        return;
    }

    const int pauseKey = game->keybind[IVY_KEY_CANCEL].currentKey;
    if (IsKeyPressed(pauseKey)) {
        if (gameplayData->state == PAUSE_MENU_CLOSED) {
            gameplayData->state = PAUSE_MENU_OPENED;
            gameplayData->menu.selected = 0;
        }
        else if (gameplayData->state == PAUSE_MENU_OPENED) {
            gameplayData->state = PAUSE_MENU_CLOSED;
            gameplayData->inventoryUI.selectedSlot = INVENTORY_SLOT_NONE;
        }
    }

    if (gameplayData->state == PAUSE_MENU_OPENED) {
        GameplayUpdateStatePause(game);
        return;
    }

    IvyDoor *door = &gameplayData->door;

    Ivy_Door_Update(door);

    if (IsKeyPressed(KEY_E)) {
        if (Ivy_Door_CanInteract(door, gameplayData->player)) {
            Ivy_Door_Interact(door);
            Ivy_Door_Update(door);
        }
    }

    Ivy_Player_Update(gameplayData->player, gameplayData->collusionMap, &gameplayData->door, GetFrameTime());

    const Vector2 playerPosition = Ivy_Player_GetPosition(gameplayData->player);
    const Vector2 mapSize = Ivy_Tilemap_GetDimensions(gameplayData->tilemap);
    Ivy_Camera_Update(&gameplayData->camera, playerPosition, mapSize.x * IVY_TILE_SIZE, mapSize.y * IVY_TILE_SIZE);
}
