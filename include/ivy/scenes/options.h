#ifndef IVY_SCENES_OPTIONS_H
#define IVY_SCENES_OPTIONS_H

#include "ivy/audio/wav.h"
#include "ivy/core/types.h"
#include "ivy/scenes/options_private.h"
#include "ivy/utils/forward.h"

#ifdef __cplusplus
extern "C" {
#endif

struct IvySceneOptionsData {
    _IvyOptionsLocale   locale;     // 64
    IvySound            sound;      // 48
    _IvyKeybindConfig   keybind;    // 12
    float               cursorY;    // 4
    float               targetY;    // 4
    u32                 selected;   // 4
};                                  // (136)

void Ivy_Scene_OptionsInit(IvyGame *game);
void Ivy_Scene_OptionsUpdate(IvyGame *game);
void Ivy_Scene_OptionsDrawWorld(IvyGame *game);
void Ivy_Scene_OptionsDrawUI(IvyGame *game);
void Ivy_Scene_OptionsRebuildTextures(IvyGame *game);
void Ivy_Scene_OptionsUnload(IvySceneManager *sceneManager);

#ifdef __cplusplus
}
#endif

#endif