#ifndef IVY_GAME_H
#define IVY_GAME_H

#include "ivy/scenes.h"
#include "ivy/locale.h"

#ifdef IVY_DEBUG
#define IVY_ASSERT(condition, ...)                                                                                      \
    do {                                                                                                                \
        if (!(condition)) {                                                                                             \
            fprintf(stderr, "\n[ASSERT] %s:%d\n", __FILE__, __LINE__);                                                  \
            fprintf(stderr, "  " __VA_ARGS__ "\n");                                                                     \
            abort();                                                                                                    \
        }                                                                                                               \
    } while (0)
#else
#define IVY_ASSERT(condition, ...) ((void)0)
#endif

typedef struct {
    u32 screenWidth;
    u32 screenHeight;
} ScreenData;

struct Game {
    ScreenData          screen;
    VirtualResolution   viewport;
    Font                fonts[2];
    Texture2D           cursors[2];
    SceneManager        sceneManager;
    Locale              *locale;
};

Game GameInit(u32 sw, u32 sh);
void GameUpdate(Game *game);
void GameDraw(Game *game);
void GameDestroy(const Game *game);

#endif