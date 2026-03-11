#include "ivy/game.h"
#include "ivy/utils.h"
#include "ivy/scenes.h"
#include "ivy/virtual.h"

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>

static const char *MENU_ITEMS[] = {
    "SCREEN SIZE",
    "FULLSCREEN",
    "BACK",
};
static const u32 MENU_COUNT = 3;

typedef struct {
    u32 width;
    u32 height;
    float scale;
} ScreenSize;

static const ScreenSize SCREEN_SIZES[] = {
    { 640, 360, 1.0f },
    { 960, 540, 1.5f },
    { 1280, 720, 2.0f },
    { 1600, 900, 2.5f },
    { 1920, 1080, 3.0f },
};
static const u32 SCREEN_SIZE_COUNT = 5;

static const float CURSOR_SPEED     = 0.15f;
static const float MENU_SPACING     = 16.0f;
static const float TEXT_SIZE        = 14.0f;
static const float CURSOR_SCALE     = 0.5f;
static const float CURSOR_X_OFFSET  = 10.0f;
static const float TEXT_X_OFFSET    = 28.0f;
static const float MARGIN_BOTTOM    = 50.0f;
static const float MARGIN_TOP       = 60.0f;
static const float VALUE_X_OFFSET   = 120.0f;

static int GetCurrentScreenSizeIndex(u32 width, u32 height)
{
    for (u32 i = 0; i < SCREEN_SIZE_COUNT; i++) {
        if (SCREEN_SIZES[i].width == width && SCREEN_SIZES[i].height == height) {
            return i;
        }
    }

    return 0;
}

void SceneOptionsInit(Scene *s)
{
    SceneOptionsData *sd = malloc(sizeof(SceneOptionsData));
    assert(sd && "[ERROR] Failed to allocate memory for SceneOptionsData!");

    *sd = (SceneOptionsData) {
        .selectedIndex = 0,
        .cursorY = 0.0f
    };

    s->data.options = sd;
}

void SceneOptionsUpdate(Game *game)
{
    SceneOptionsData *sd = game->sceneManager.activeScene.data.options;

    const int dir = IsKeyPressed(KEY_DOWN) - IsKeyPressed(KEY_UP);
    if (dir != 0) {
        sd->selectedIndex = (sd->selectedIndex + dir + MENU_COUNT) % MENU_COUNT;
    }

    if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE))
    {
        switch(sd->selectedIndex) {
            case 0: { // SCREEN SIZE
                int currentIdx = GetCurrentScreenSizeIndex(
                    game->screen.screenWidth,
                    game->screen.screenHeight
                );
                int newIdx = (currentIdx + 1) % SCREEN_SIZE_COUNT;

                game->screen.screenWidth  = SCREEN_SIZES[newIdx].width;
                game->screen.screenHeight = SCREEN_SIZES[newIdx].height;

                SetWindowSize(game->screen.screenWidth, game->screen.screenHeight);
                UpdateVirtualResolution(&game->viewport, game->screen.screenWidth, game->screen.screenHeight);
                SetTextureFilter(game->viewport.target.texture, TEXTURE_FILTER_POINT);

                sd->cursorY = 0.0f;
                ClearBackground(BLACK);
            } break;

            case 1: { // FULLSCREEN toggle
                ToggleFullscreen();

                game->screen.screenWidth  = GetScreenWidth();
                game->screen.screenHeight = GetScreenHeight();

                UpdateVirtualResolution(&game->viewport, game->screen.screenWidth, game->screen.screenHeight);
                SetTextureFilter(game->viewport.target.texture, TEXTURE_FILTER_POINT);

                sd->cursorY = 0.0f;
                ClearBackground(BLACK);
            } break;

            case 2: // BACK
                game->sceneManager.activeScene.type = SCENE_TITLE;
                game->sceneManager.sceneChanged = true;
                break;
        }
    }

    if (IsKeyPressed(KEY_ESCAPE)) {
        game->sceneManager.activeScene.type = SCENE_TITLE;
        game->sceneManager.sceneChanged = true;
    }
}

void SceneOptionsDrawWorld(Game *game)
{
    ClearBackground((Color){ 20, 20, 30, 255 });
}

void SceneOptionsDrawUI(Game *game)
{
    SceneOptionsData *sd = game->sceneManager.activeScene.data.options;
    const float virtualScale = game->viewport.scale;
    const Texture2D *cursor = &game->cursors[IVY_CURSOR_PRIMARY];

    const float menuStartY = VIRTUAL_HEIGHT - MARGIN_BOTTOM - ((float)MENU_COUNT - 1) * MENU_SPACING;

    const float cursorVirtualHeight = (float)cursor->height * CURSOR_SCALE;
    const float verticalOffset = (TEXT_SIZE - cursorVirtualHeight) * 0.5f;

    const float targetY = menuStartY + (float)sd->selectedIndex * MENU_SPACING + verticalOffset;

    if (sd->cursorY == 0.0f) sd->cursorY = targetY;
    else sd->cursorY += (targetY - sd->cursorY) * CURSOR_SPEED;

    // Draw cursor
    Vector2 cursorVirtualPos = { CURSOR_X_OFFSET, sd->cursorY };
    Vector2 cursorScreenPos  = GetScreenPos(&game->viewport, cursorVirtualPos);
    DrawTextureEx(*cursor, cursorScreenPos, 0.0f, virtualScale * CURSOR_SCALE, WHITE);

    // Draw equipment menu
    for (u32 i = 0; i < MENU_COUNT; i++) {
        Vector2 textVirtualPos = { TEXT_X_OFFSET, menuStartY + (float)i * MENU_SPACING };
        Vector2 textScreenPos  = GetScreenPos(&game->viewport, textVirtualPos);
        Color textColor = (i == sd->selectedIndex) ? WHITE : GRAY;

        DrawTextEx(game->fonts[IVY_FONT_PRIMARY], MENU_ITEMS[i],
                   textScreenPos, TEXT_SIZE * virtualScale, 1, textColor);
    }

    char valueBuffer[32];

    if (sd->selectedIndex == 0) { // SCREEN SIZE
        snprintf(valueBuffer, sizeof(valueBuffer), "%dx%d",
                 game->screen.screenWidth, game->screen.screenHeight);

        Vector2 valueVirtualPos = { TEXT_X_OFFSET + VALUE_X_OFFSET, menuStartY };
        Vector2 valueScreenPos  = GetScreenPos(&game->viewport, valueVirtualPos);
        DrawTextEx(game->fonts[IVY_FONT_PRIMARY], valueBuffer,
                   valueScreenPos, TEXT_SIZE * virtualScale, 1, YELLOW);
    }
    else if (sd->selectedIndex == 1) { // FULLSCREEN
        const char *status = IsWindowFullscreen() ? "ON" : "OFF";

        Vector2 valueVirtualPos = { TEXT_X_OFFSET + VALUE_X_OFFSET, menuStartY + MENU_SPACING };
        Vector2 valueScreenPos  = GetScreenPos(&game->viewport, valueVirtualPos);
        DrawTextEx(game->fonts[IVY_FONT_PRIMARY], status,
                   valueScreenPos, TEXT_SIZE * virtualScale, 1, YELLOW);
    }

    Vector2 titleVirtualPos = { TEXT_X_OFFSET, MARGIN_TOP };
    Vector2 titleScreenPos  = GetScreenPos(&game->viewport, titleVirtualPos);
    DrawTextEx(game->fonts[IVY_FONT_PRIMARY], "OPTIONS",
               titleScreenPos, TEXT_SIZE * virtualScale * 1.5f, 1, WHITE);
}

void SceneOptionsUnload(Scene *s)
{
    if (s->data.options) {
        free(s->data.options);
        s->data.options = NULL;
    }
}
void SceneOptionsRebuildTextures(Game *game) { (void)game; }