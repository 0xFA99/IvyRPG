#include "ivy/core/game.h"
#include "ivy/arena/linear.h"
#include "ivy/audio/wav.h"
#include "ivy/systems/scene_manager.h"
#include "ivy/scenes/options.h"
#include "ivy/utils/file_ids.h"

void Ivy_Scene_OptionsInit(IvyGame *game)
{
    IvySceneOptionsData *optionsData = Ivy_Arena_LinearAllocZero(&game->arena, sizeof(IvySceneOptionsData));
    IVY_ENSURE(optionsData);

    _ivyOptions_ReloadLocales(optionsData, game->locale);

    optionsData->sound = Ivy_Audio_LoadSoundWav(&game->arena, game->assets, ASSET_AUDIO_CURSOR_WAV);

    game->scenes->actionScene->data = optionsData;
}

void Ivy_Scene_OptionsRebuildTextures(IvyGame *game)
{
    (void)game;
}

void Ivy_Scene_OptionsUnload(IvySceneManager *sceneManager)
{
    const IvySceneOptionsData *od = sceneManager->actionScene->data;
    Ivy_Audio_UnloadSound(&od->sound);
    sceneManager->actionScene->data = NULL;
}