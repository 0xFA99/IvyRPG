#ifndef IVY_GAME_H
#define IVY_GAME_H

#include "ivy/scenes.h"

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
};

Game GameInit(u32 sw, u32 sh);
void GameUpdate(Game *game);
void GameDraw(Game *game);
void GameDestroy(const Game *game);

#endif