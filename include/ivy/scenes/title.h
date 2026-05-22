#ifndef IVY_SCENES_TITLE_H
#define IVY_SCENES_TITLE_H

#include "ivy/core/types.h"
#include "ivy/utils/forward.h"
#include "ivy/audio/wav.h"

#include "raylib/raylib.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    MENU_NEW_GAME = 0,
    MENU_CONTINUE,
    MENU_OPTIONS,
    MENU_EXIT,
    MENU_COUNT
} IvyTitleMenu;

typedef struct {
    const char *menuStrings[MENU_COUNT];    // 8 * 4 = 32
    u32        menuLengths[MENU_COUNT];     // 4 * 4 = 16
    Texture2D  background;                  // 20
    float      cursorY;                     // 4
    float      targetY;                     // 4
    u32        selected;                    // 4
    IvySound   sound;                       // 48
    Music      music;                       // 56
} IvySceneTitleData;                        // 128

void Ivy_Scene_TitleInit(IvyGame *game);
void Ivy_Scene_TitleUpdate(IvyGame *game);
void Ivy_Scene_TitleDrawWorld(IvyGame *game);
void Ivy_Scene_TitleDrawUI(IvyGame *game);
void Ivy_Scene_TitleRebuildTextures(IvyGame *game);
void Ivy_Scene_TitleUnload(IvySceneManager *sm);

#ifdef __cplusplus
}
#endif
#endif