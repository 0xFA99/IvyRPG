#include "ivy/core/game.h"
#include "ivy/arena/linear.h"
#include "ivy/systems/scene_manager.h"
#include "ivy/scenes/options.h"

#include "ivy/utils/file_ids.h"

extern void Options_ReloadMenuStrings(IvySceneOptionsData *restrict optionsData, const IvyLocale *restrict locale);
extern void Options_UpdateMainMenu(IvyGame *restrict game, IvySceneOptionsData *restrict optionsData);
extern void Options_DrawMainMenu(IvyGame *restrict game, IvySceneOptionsData *restrict optionsData);
extern void Options_DrawKeybindPopup(IvyGame *restrict game, IvySceneOptionsData *restrict optionsData);
extern void Options_DrawOptionsBackground(IvyGame *restrict game, IvySceneOptionsData *restrict optionsData);

void Ivy_Scene_OptionsInit(IvyGame *game)
{
    IvySceneOptionsData *optionsData = Ivy_Arena_LinearAllocZero(&game->arena, sizeof(*optionsData));
    IVY_ENSURE(optionsData);

    Options_ReloadMenuStrings(optionsData, game->locale);

    optionsData->keybindState        = KEYBIND_STATE_CLOSED;
    optionsData->keybindSelected     = 0;
    optionsData->keybindScrollOffset = 0;
    optionsData->keybindCursorY      = 0.0f;
    optionsData->keybindTargetY      = 0.0f;
    optionsData->waitingForKey       = false;

    optionsData->sound = Ivy_Audio_LoadSoundWav(&game->arena, game->assets, ASSET_AUDIO_CURSOR_WAV);

    game->scenes->actionScene->data = optionsData;
}

void Ivy_Scene_OptionsUpdate(IvyGame *game)
{
    Options_UpdateMainMenu(game, game->scenes->actionScene->data);
}

void Ivy_Scene_OptionsDrawWorld(IvyGame *game)
{
    (void)game;
    ClearBackground((Color){ 20, 20, 30, 225 });
}

void Ivy_Scene_OptionsDrawUI(IvyGame *game)
{
    IvySceneOptionsData *optionsData = game->scenes->actionScene->data;

    if (optionsData->keybindState != KEYBIND_STATE_CLOSED) {
        Options_DrawOptionsBackground(game, optionsData);
        Options_DrawKeybindPopup(game, optionsData);
        return;
    }

    Options_DrawMainMenu(game, optionsData);
}

void Ivy_Scene_OptionsRebuildTextures(IvyGame *game)
{
    (void)game;
}

void Ivy_Scene_OptionsUnload(IvySceneManager *sceneManager)
{
    const IvySceneOptionsData *optionsData = sceneManager->actionScene->data;

    Ivy_Audio_UnloadSound(&optionsData->sound);

    sceneManager->actionScene->data = NULL;
}
