#ifndef IVY_SCENES_OPTIONS_H
#define IVY_SCENES_OPTIONS_H

#include "ivy/core/types.h"
#include "ivy/utils/forward.h"
#include "ivy/audio/wav.h"

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    OPT_SCREEN_SIZE = 0,
    OPT_FULLSCREEN,
    OPT_LANGUAGE,
    OPT_KEYBIND,
    OPT_BACK,
    OPT_COUNT       // 5
} IvyOptionsMenu;

typedef enum {
    KEYBIND_STATE_CLOSED = 0,
    KEYBIND_STATE_SELECTING,
    KEYBIND_STATE_WAITING_INPUT,
    KEYBIND_STATE_COUNT
} IvyKeybindState;

struct IvySceneOptionsData {
    const char *menuStrings[OPT_COUNT];     // 8 * 5 = 40
    u32         menuLengths[OPT_COUNT];     // 4 * 5 = 20
    float       cursorY;                    // 4
    float       targetY;                    // 4
    u32         selected;                   // 4

    // Keybind popup data
    IvyKeybindState keybindState;           // 4
    u32             keybindSelected;        // 4
    u32             keybindScrollOffset;    // 4
    float           keybindCursorY;         // 4
    float           keybindTargetY;         // 4
    bool            waitingForKey;          // 1
    char            padding[3];
    IvySound        sound;
};                                          // 104

void Ivy_Scene_OptionsInit(IvyGame *game);
void Ivy_Scene_OptionsUpdate(IvyGame *game);
void Ivy_Scene_OptionsDrawWorld(IvyGame *game);
void Ivy_Scene_OptionsDrawUI(IvyGame *game);
void Ivy_Scene_OptionsRebuildTextures(IvyGame *game);
void Ivy_Scene_OptionsUnload(IvySceneManager *sm);

#ifdef __cplusplus
}
#endif
#endif