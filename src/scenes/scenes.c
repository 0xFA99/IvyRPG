#include "ivy/scenes.h"
#include "raylib/raylib.h"
#include <stddef.h>

#define FADE_SPEED 2.5f

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

void BeginSceneTransition(SceneManager *sm, SceneType nextScene)
{
    if (sm->transitioning) return;

    sm->transitioning = true;
    sm->fadingOut     = true;
    sm->fadeAlpha     = 0.0f;
    sm->pendingScene  = nextScene;
}

void UpdateSceneTransition(SceneManager *sm)
{
    if (!sm->transitioning) return;

    const float dt = GetFrameTime();

    if (sm->fadingOut) {
        sm->fadeAlpha += FADE_SPEED * dt;
        if (sm->fadeAlpha >= 1.0f) {
            sm->fadeAlpha = 1.0f;
            sm->fadingOut = false;

            sm->activeScene.type = sm->pendingScene;
            UpdateScene(sm);
        }
    } else {
        sm->fadeAlpha -= FADE_SPEED * dt;
        if (sm->fadeAlpha <= 0.0f) {
            sm->fadeAlpha     = 0.0f;
            sm->transitioning = false;
        }
    }
}

void DrawSceneTransition(const SceneManager *sm)
{
    if (!sm->transitioning || sm->fadeAlpha <= 0.0f) return;

    DrawRectangle(
        0, 0,
        GetScreenWidth(), GetScreenHeight(),
        (Color){ 0, 0, 0, (unsigned char)(sm->fadeAlpha * 255) }
    );
}