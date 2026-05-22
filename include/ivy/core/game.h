#ifndef IVY_CORE_GAME_H
#define IVY_CORE_GAME_H

#include "ivy/arena/linear.h"
#include "ivy/core/virtual.h"

#include "raylib/raylib.h"

#ifdef __cplusplus
extern "C" {
#endif

enum {
    IVY_FONT_PRIMARY = 0,
    IVY_FONT_SECONDARY,
    IVY_FONT_MAX
};

enum {
    IVY_CURSOR_PRIMARY = 0,
    IVY_CURSOR_SECONDARY,
    IVY_CURSOR_MAX
};

enum {
    IVY_ARENA_MAIN = 0,
    IVY_ARENA_LOCALE,
    IVY_ARENA_MAX
};

struct IvyGame {
    IvyArenaLinear          arenas[IVY_ARENA_MAX];      // 64

    const IvyKeybindInfo   *keybind;                    // 8
    IvyAssetManager        *assets;                     // 8
    IvySceneManager        *scenes;                     // 8
    IvyVirtualScreen       *viewport;                   // 8
    IvyLocale              *locale;                     // 8

    Font                    fonts[IVY_FONT_MAX];        // 96
    Texture2D               cursors[IVY_CURSOR_MAX];    // 40
};                                                      // 240
IVY_ASSERT_STATIC(sizeof(IvyGame) == 240, "[IvyGame] Size must be 240 bytes!");

IvyGame Ivy_Game_Init(Vector2 size);
void    Ivy_Game_Update(IvyGame *game);
void    Ivy_Game_Draw(IvyGame *game);
void    Ivy_Game_Destroy(IvyGame *game);

#ifdef __cplusplus
}
#endif

#endif