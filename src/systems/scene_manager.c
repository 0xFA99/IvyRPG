#include "ivy/arena/linear.h"
#include "ivy/core/game.h"
#include "ivy/core/types.h"
#include "ivy/scenes/gameplay.h"
#include "ivy/scenes/options.h"
#include "ivy/scenes/title.h"
#include "ivy/scenes/types.h"
#include "ivy/systems/scene_manager.h"
#include "ivy/utils/forward.h"

static const IvySceneVTable scene_vtables[] = {
    [SCENE_TITLE] = {
        Ivy_Scene_TitleInit,
        Ivy_Scene_TitleUpdate,
        Ivy_Scene_TitleDrawWorld,
        Ivy_Scene_TitleDrawUI,
        Ivy_Scene_TitleRebuildTextures,
        Ivy_Scene_TitleUnload
    },
    [SCENE_GAMEPLAY] = {
        Ivy_Scene_GameplayInit,
        Ivy_Scene_GameplayUpdate,
        Ivy_Scene_GameplayDrawWorld,
        Ivy_Scene_GameplayDrawUI,
        Ivy_Scene_GameplayRebuildTextures,
        Ivy_Scene_GameplayUnload
    },
    [SCENE_OPTIONS] = {
        Ivy_Scene_OptionsInit,
        Ivy_Scene_OptionsUpdate,
        Ivy_Scene_OptionsDrawWorld,
        Ivy_Scene_OptionsDrawUI,
        Ivy_Scene_OptionsRebuildTextures,
        Ivy_Scene_OptionsUnload
    },
};

static void Ivy_SceneManager_Load(IvyGame *game, const IvySceneType nextType)
{
    if (IVY_LIKELY(game->scenes->actionScene && game->scenes->actionScene->table->Unload))
        game->scenes->actionScene->table->Unload(game->scenes);

    Ivy_Audio_ResetSystemBuffers();

    Ivy_Arena_LinearRestore(&game->arena, game->scenes->snapshot);

    IvyScene *scene = Ivy_Arena_LinearAllocZero(&game->arena, sizeof(IvyScene));
    IVY_ENSURE(scene != NULL);

    game->scenes->actionScene = scene;
    scene->type = nextType;

    IVY_ASSERT(nextType >= 0 && nextType < SCENE_COUNT, "Invalid Scene Type!");
    scene->table = &scene_vtables[nextType];

    if (IVY_LIKELY(scene->table->Init)) {
        scene->table->Init(game);
    }

    scene->needsRebuild = true;
    game->scenes->sceneChanged = true;
}

void Ivy_SceneManager_RebuildIfNeeded(IvyGame *game)
{
    IvyScene *scene = game->scenes->actionScene;

    if (scene && scene->needsRebuild && scene->table->RebuildTextures)
    {
        scene->table->RebuildTextures(game);
        scene->needsRebuild = false;
    }
}

IvySceneManager *Ivy_SceneManager_Init(IvyGame *restrict game, IvyAssetManager *restrict assets)
{
    IVY_ASSERT(game != NULL, "[SceneManager] Arena not found!");

    IvySceneManager *sceneManager = Ivy_Arena_LinearAllocZero(&game->arena, sizeof(IvySceneManager));
    IVY_ENSURE(sceneManager != NULL);

    sceneManager->snapshot = Ivy_Arena_LinearGetSnapshot(&game->arena);
    game->assets = assets;
    game->scenes = sceneManager;

    Ivy_SceneManager_Load(game, SCENE_TITLE);

    return sceneManager;
}

void Ivy_SceneManager_Transition(IvyGame *restrict game, const IvySceneType nextScene)
{
    IVY_ASSERT(game, "[SceneManager] Instance is NULL!");
    IVY_ASSERT(nextScene >= 0 && nextScene < SCENE_COUNT, "[SceneManager] Invalid scene type!");

    Ivy_SceneManager_Load(game, nextScene);
}
