#include "ivy/core/keybind.h"
#include "ivy/core/game.h"
#include "ivy/audio/buffer.h"
#include "ivy/audio/ogg.h"
#include "ivy/scenes/gameplay.h"
#include "ivy/systems/scene_manager.h"
#include "ivy/entities/player.h"

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
