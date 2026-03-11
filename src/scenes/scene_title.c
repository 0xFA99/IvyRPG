#include "ivy/game.h"
#include "ivy/utils.h"
#include "ivy/scenes.h"

#include <assert.h>
#include <stdlib.h>

static const char *MENU_ITEMS[] = {
    "NEW GAME",
    "CONTINUE",
    "OPTIONS",
    "EXIT",
};
static const u32 MENU_COUNT = sizeof(MENU_ITEMS) / sizeof(MENU_ITEMS[0]);

static const float CURSOR_SPEED     = 0.15f;
static const float MENU_SPACING     = 16.0f;
static const float TEXT_SIZE        = 14.0f;
static const float CURSOR_SCALE     = 0.5f;
static const float CURSOR_X_OFFSET  = 10.0f;
static const float TEXT_X_OFFSET    = 28.0f;
static const float MARGIN_BOTTOM    = 36.0f;

void SceneTitleInit(Scene *s)
{
    SceneTitleData *sd = malloc(sizeof(SceneTitleData));
    assert(sd && "[ERROR] Failed to allocate memory for SceneTitleData!");

    *sd = (SceneTitleData) {
        .selectedIndex  = 0,
        .cursorY        = 0.0f,
        .background     = LoadTextureFromImageBin("assets/background.bin")
    };

    s->data.title = sd;
}

void SceneTitleUpdate(Game *game)
{
    SceneTitleData *sd = game->sceneManager.activeScene.data.title;

    const int dir = IsKeyPressed(KEY_DOWN) - IsKeyPressed(KEY_UP);
    if (dir != 0) {
        sd->selectedIndex = (sd->selectedIndex + dir + MENU_COUNT) % MENU_COUNT;
    }

    if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE))
    {
        switch(sd->selectedIndex) {
            case 0: // NEW GAME
                game->sceneManager.activeScene.type = SCENE_GAMEPLAY;
                break;
            case 1: // CONTINUE
                // TODO: Load save game
                game->sceneManager.activeScene.type = SCENE_GAMEPLAY;
                break;
            case 2: // OPTIONS
                game->sceneManager.activeScene.type = SCENE_OPTIONS;
                break;
            case 3: // EXIT
                game->sceneManager.activeScene.type = SCENE_EXIT;
                game->sceneManager.isRunning = false;
                break;
            default:
                break;
        }

        game->sceneManager.sceneChanged = true;
    }

    if (IsKeyPressed(KEY_ESCAPE)) {
        game->sceneManager.activeScene.type = SCENE_EXIT;
        game->sceneManager.isRunning = false;
        game->sceneManager.sceneChanged = true;
    }
}

void SceneTitleDrawWorld(Game *game)
{
    const SceneTitleData *sd = game->sceneManager.activeScene.data.title;

    DrawTexturePro(sd->background,
        (Rectangle){ 0, 0, (float)sd->background.width, (float)sd->background.height },
        (Rectangle){ 0, 0, VIRTUAL_WIDTH, VIRTUAL_HEIGHT },
        (Vector2){ 0, 0 }, 0.0f, WHITE);
}

void SceneTitleDrawUI(Game *game)
{
    SceneTitleData *sd = game->sceneManager.activeScene.data.title;
    const float virtualScale = game->viewport.scale;
    const Texture2D *cursor = &game->cursors[IVY_CURSOR_PRIMARY];

    const float menuStartY = VIRTUAL_HEIGHT - MARGIN_BOTTOM - ((float)MENU_COUNT - 1) * MENU_SPACING;
    const float cursorVirtualHeight = (float)cursor->height * CURSOR_SCALE;
    const float verticalOffset = (TEXT_SIZE - cursorVirtualHeight) * 0.5f;
    const float targetY = menuStartY + (float)sd->selectedIndex * MENU_SPACING + verticalOffset;

    if (sd->cursorY == 0.0f) sd->cursorY = targetY;
    else sd->cursorY += (targetY - sd->cursorY) * CURSOR_SPEED;

    const Vector2 cursorVirtualPos = { CURSOR_X_OFFSET, sd->cursorY };
    const Vector2 cursorScreenPos  = GetScreenPos(&game->viewport, cursorVirtualPos);
    DrawTextureEx(*cursor, cursorScreenPos, 0.0f, virtualScale * CURSOR_SCALE, WHITE);

    for (u32 i = 0; i < MENU_COUNT; i++) {
        const Vector2 textVirtualPos = { TEXT_X_OFFSET, menuStartY + (float)i * MENU_SPACING };
        const Vector2 textScreenPos  = GetScreenPos(&game->viewport, textVirtualPos);
        const Color textColor = (i == sd->selectedIndex) ? WHITE : GRAY;

        DrawTextEx(game->fonts[IVY_FONT_PRIMARY], MENU_ITEMS[i], textScreenPos, TEXT_SIZE * virtualScale, 1, textColor);
    }
}

void SceneTitleUnload(Scene *s)
{
    if (s->data.title) {
        UnloadTexture(s->data.title->background);
        free(s->data.title);
        s->data.title = NULL;
    }
}
void SceneTitleRebuildTextures(Game *game) { (void)game; }