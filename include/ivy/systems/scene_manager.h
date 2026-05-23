#ifndef IVY_SYSTEM_SCENE_MANAGER_H
#define IVY_SYSTEM_SCENE_MANAGER_H

#include "ivy/core/types.h"
#include "ivy/utils/forward.h"
#include "ivy/arena/linear.h"
#include "ivy/scenes/types.h"

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

struct IvySceneManager {
    IvyScene               *actionScene;    // 8
    IvyArenaLinearSnapshot  snapshot;       // 8
    bool                    sceneChanged;   // 1
    bool                    shouldExit;     // 1
    char                    padding[6];     // 6
};                                          // 24
IVY_ASSERT_STATIC(sizeof(IvySceneManager) == 24, "[IvyScheneManager] Size must be 24 bytes!");

IvySceneManager *Ivy_SceneManager_Init(IvyGame *restrict game, IvyAssetManager *restrict am);
void             Ivy_SceneManager_RebuildIfNeeded(IvyGame *game);
void             Ivy_SceneManager_Transition(IvyGame *restrict game, IvySceneType nextScene);

#ifdef __cplusplus
}
#endif

#endif