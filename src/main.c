#include "ivy/core/game.h"
#include "ivy/systems/scene_manager.h"

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

    IvyGame game = Ivy_Game_Init((Vector2){DEFAULT_SCREEN_WIDTH, DEFAULT_SCREEN_HEIGHT});

    while (!WindowShouldClose() && !game.scenes->shouldExit)
    {
        Ivy_Game_Update(&game);
        Ivy_Game_Draw(&game);
    }

    Ivy_Game_Destroy(&game);

    CloseWindow();
    return 0;
}
