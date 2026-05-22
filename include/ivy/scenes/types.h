#ifndef IVY_SCENES_TYPES_H
#define IVY_SCENES_TYPES_H

#include "ivy/core/types.h"
#include "ivy/utils/forward.h"

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    SCENE_TITLE = 0,
    SCENE_GAMEPLAY,
    SCENE_OPTIONS,
    SCENE_COUNT
} IvySceneType;

typedef struct {
    void (*Init)(IvyGame *game);            // 8
    void (*Update)(IvyGame *game);          // 8
    void (*DrawWorld)(IvyGame *game);       // 8
    void (*DrawUI)(IvyGame *game);          // 8
    void (*RebuildTextures)(IvyGame *game); // 8
    void (*Unload)(IvySceneManager *sm);    // 8
} IvySceneVTable;                           // 48
IVY_ASSERT_STATIC(sizeof(IvySceneVTable) == 48, "[IvySceneVTable] Size must be 48 bytes!");

struct IvyScene {
    const IvySceneVTable *table;            // 8
    IvyAssetManager      *assetManager;     // 8
    void                 *data;             // 8
    IvySceneType          type;             // 4
    bool                  needsRebuild;     // 1
    char                  padding[3];       // 3
};                                          // 32
IVY_ASSERT_STATIC(sizeof(IvyScene) == 32, "[IvyScene] Size must be 32 bytes!");

#ifdef __cplusplus
}
#endif

#endif