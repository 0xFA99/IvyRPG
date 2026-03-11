#include "ivy/scenes.h"
#include <stddef.h>

void UpdateScene(SceneManager *sm)
{
    Scene *s = &sm->activeScene;

    if (s->Unload) {
        s->Unload(s);
        s->data.title = NULL;
    }

    switch (s->type)
    {
        case SCENE_TITLE: {
            s->Init             = SceneTitleInit;
            s->Update           = SceneTitleUpdate;
            s->DrawWorld        = SceneTitleDrawWorld;
            s->RebuildTextures  = SceneTitleRebuildTextures;
            s->DrawUI           = SceneTitleDrawUI;
            s->Unload           = SceneTitleUnload;
        } break;

        case SCENE_GAMEPLAY: {
            s->Init             = SceneGameplayInit;
            s->Update           = SceneGameplayUpdate;
            s->DrawWorld        = SceneGameplayDrawWorld;
            s->RebuildTextures  = SceneGameplayRebuildTextures;
            s->DrawUI           = SceneGameplayDrawUI;
            s->Unload           = SceneGameplayUnload;
        } break;

        case SCENE_OPTIONS: {
            s->Init             = SceneOptionsInit;
            s->Update           = SceneOptionsUpdate;
            s->DrawWorld        = SceneOptionsDrawWorld;
            s->RebuildTextures  = SceneOptionsRebuildTextures;
            s->DrawUI           = SceneOptionsDrawUI;
            s->Unload           = SceneOptionsUnload;
        } break;

        default: break;
    }

    if (s->Init) s->Init(s);
    sm->sceneChanged = false;
}