#include "ivy/core/keybind.h"
#include "ivy/core/game.h"
#include "ivy/audio/buffer.h"
#include "ivy/audio/ogg.h"
#include "ivy/scenes/gameplay.h"
#include "ivy/systems/scene_manager.h"
#include "ivy/entities/player.h"

static void GameplayInventory(IvyGame *game)
{
    IvySceneGameplayData *gameplayData = game->scenes->actionScene->data;
    IvyInventoryUI *ui = &gameplayData->inventoryUI;

    const IvyInventory *inventory = Ivy_Player_GetInventory(gameplayData->player);
    const IvyInventoryBag *bag = &inventory->bag;
    const u8 targetCategory = IVY_ITEM_TYPE_EQUIPMENT;
    const u8 equipmentCount = bag->categoryCount[targetCategory];

    if (equipmentCount > 0) {
        if (gameplayData->inventoryUI.selectedSlot >= equipmentCount) {
            gameplayData->inventoryUI.selectedSlot = 0;
        }

        if (IsKeyPressed(KEY_RIGHT))
        {
            if (gameplayData->inventoryUI.selectedSlot + 1 < equipmentCount) {
                gameplayData->inventoryUI.selectedSlot += 1;
                Ivy_Audio_PlayAudioBuffer(gameplayData->inventoryUI.sound[0].data.stream.buffer);
            }
        }
        else if (IsKeyPressed(KEY_LEFT))
        {
            if (gameplayData->inventoryUI.selectedSlot > 0) {
                gameplayData->inventoryUI.selectedSlot -= 1;
                Ivy_Audio_PlayAudioBuffer(gameplayData->inventoryUI.sound[0].data.stream.buffer);
            }
        }
        else if (IsKeyPressed(KEY_DOWN))
        {
            if (gameplayData->inventoryUI.selectedSlot + 2 < equipmentCount) {
                gameplayData->inventoryUI.selectedSlot += 2;
                Ivy_Audio_PlayAudioBuffer(gameplayData->inventoryUI.sound[0].data.stream.buffer);
            }
        }
        else if (IsKeyPressed(KEY_UP))
        {
            if (gameplayData->inventoryUI.selectedSlot >= 2) {
                gameplayData->inventoryUI.selectedSlot -= 2;
                Ivy_Audio_PlayAudioBuffer(gameplayData->inventoryUI.sound[0].data.stream.buffer);
            }
        }
    }

    if (IsKeyPressed(game->keybind[IVY_KEY_CANCEL].currentKey)) {
        gameplayData->state = PAUSE_MENU_OPENED;
        ui->selectedSlot = 255;
        Ivy_Audio_PlayAudioBuffer(gameplayData->inventoryUI.sound[1].data.stream.buffer);
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

    if (IsKeyPressed(game->keybind[IVY_KEY_CONFIRM].currentKey)) {
        switch (gameplayData->menu.selected) {
            case 0:
                gameplayData->state = PAUSE_MENU_CLOSED;
                break;

            case 3:
                gameplayData->state = PAUSE_MENU_INVENTORY;
                gameplayData->inventoryUI.selectedSlot = 0;
                break;

            case 4:
                Ivy_SceneManager_Transition(game, SCENE_TITLE);
                break;

            default: break;
        }
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
    if (IsKeyPressed(pauseKey))
    {
        if (gameplayData->state == PAUSE_MENU_CLOSED) {
            gameplayData->state = PAUSE_MENU_OPENED;
            gameplayData->menu.selected = 0;
        }
        else if (gameplayData->state == PAUSE_MENU_CLOSED) {
            gameplayData->state = PAUSE_MENU_CLOSED;
        }
    }

    if (gameplayData->state == PAUSE_MENU_OPENED) {
        GameplayUpdateStatePause(game);
        return;
    }

    Ivy_Player_Update(gameplayData->player, GetFrameTime(), gameplayData->collusionMap);
    Ivy_Camera_Update(&gameplayData->camera, Ivy_Player_GetPosition(gameplayData->player));
}
