#include "ivy/game.h"

#define DEFAULT_SCREEN_TITLE "Ivy RPG"
#define DEFAULT_SCREEN_WIDTH 1280
#define DEFAULT_SCREEN_HEIGHT 720
#define DEFAULT_FPS 60

int main(void)
{
    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_MSAA_4X_HINT);

    InitWindow(DEFAULT_SCREEN_WIDTH, DEFAULT_SCREEN_HEIGHT, DEFAULT_SCREEN_TITLE);
    SetTargetFPS(DEFAULT_FPS);
    SetExitKey(0);

    Game game = GameInit(DEFAULT_SCREEN_WIDTH, DEFAULT_SCREEN_HEIGHT);

    while (!WindowShouldClose())
    {
        GameUpdate(&game);
        GameDraw(&game);
    }

    GameDestroy(&game);

    CloseWindow();
    return 0;
}