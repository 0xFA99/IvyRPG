#include "ivy/game.h"
#include "ivy/utils.h"
#include "ivy/scenes.h"

#include <stddef.h>

Game GameInit(const u32 sw, const u32 sh)
{
    Game game = {0};

    game.screen.screenWidth  = sw;
    game.screen.screenHeight = sh;

    game.viewport = InitVirtualScreen(sw, sh);
    SetTextureFilter(game.viewport.target.texture, TEXTURE_FILTER_POINT);

    game.fonts[IVY_FONT_PRIMARY]   = LoadFontBin(PRIMARY_FONT_PATH, LOAD_FONT_SIZE);
    game.fonts[IVY_FONT_SECONDARY] = LoadFontBin(SECONDARY_FONT_PATH, LOAD_FONT_SIZE);

    SetTextureFilter(game.fonts[IVY_FONT_PRIMARY].texture,   TEXTURE_FILTER_BILINEAR);
    SetTextureFilter(game.fonts[IVY_FONT_SECONDARY].texture, TEXTURE_FILTER_BILINEAR);

    game.cursors[IVY_CURSOR_PRIMARY]   = LoadTextureFromImageBin(PRIMARY_CURSOR_PATH);
    game.cursors[IVY_CURSOR_SECONDARY] = LoadTextureFromImageBin(SECONDARY_CURSOR_PATH);

    SetTextureFilter(game.cursors[IVY_CURSOR_PRIMARY],   TEXTURE_FILTER_POINT);
    SetTextureFilter(game.cursors[IVY_CURSOR_SECONDARY], TEXTURE_FILTER_POINT);

    SceneManager *sm = &game.sceneManager;
    *sm = (SceneManager) {
        .activeScene = (Scene){
            .type      = SCENE_TITLE,
            .data      = {NULL},
            .Init      = SceneTitleInit,
            .Update    = SceneTitleUpdate,
            .DrawWorld = SceneTitleDrawWorld,
            .RebuildTextures = SceneTitleRebuildTextures,
            .DrawUI    = SceneTitleDrawUI,
            .Unload    = SceneTitleUnload
        },
        .deltaTime    = 0.0f,
        .sceneChanged = false,
        .isRunning    = true,

        .fadeAlpha    = 1.0f,
        .fadingOut    = false,
        .transitioning = true,
        .pendingScene = SCENE_TITLE
    };

    sm->activeScene.Init(&sm->activeScene);

    return game;
}

void GameUpdate(Game *game)
{
    if (IsWindowResized()) {
        game->screen.screenWidth  = GetScreenWidth();
        game->screen.screenHeight = GetScreenHeight();
        UpdateVirtualResolution(&game->viewport,
            game->screen.screenWidth, game->screen.screenHeight);
        SetTextureFilter(game->viewport.target.texture, TEXTURE_FILTER_POINT);
    }

    if (!game->sceneManager.transitioning)
        game->sceneManager.activeScene.Update(game);

    if (game->sceneManager.sceneChanged) {
        BeginSceneTransition(&game->sceneManager,
            game->sceneManager.activeScene.type);
        game->sceneManager.sceneChanged = false;
    }

    UpdateSceneTransition(&game->sceneManager);
}

void GameDraw(Game *game)
{
    BeginTextureMode(game->viewport.target);
        ClearBackground(BLACK);
        game->sceneManager.activeScene.DrawWorld(game);
    EndTextureMode();

    game->sceneManager.activeScene.RebuildTextures(game);

    BeginDrawing();
        ClearBackground(BLACK);
        DrawVirtualResolution(&game->viewport);
        game->sceneManager.activeScene.DrawUI(game);
        DrawSceneTransition(&game->sceneManager);
    EndDrawing();
}

void GameDestroy(const Game *game)
{
    UnloadFont(game->fonts[IVY_FONT_PRIMARY]);
    UnloadFont(game->fonts[IVY_FONT_SECONDARY]);
    UnloadTexture(game->cursors[IVY_CURSOR_PRIMARY]);
    UnloadTexture(game->cursors[IVY_CURSOR_SECONDARY]);
    UnloadRenderTexture(game->viewport.target);
}