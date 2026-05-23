#include "ivy/core/types.h"
#include "ivy/core/game.h"
#include "ivy/core/virtual.h"
#include "ivy/systems/locale_manager.h"
#include "ivy/scenes/options.h"
#include "ivy/graphics/gfx.h"

#include <stdio.h>


extern const float CURSOR_SPEED;
extern const float CURSOR_SCALE;

enum {
    TEXT_X        = 28,
    VALUE_X       = 108,
    CURSOR_X      = 10,
    MENU_SPACING  = 16,
    MARGIN_BOTTOM = 30,
    TEXT_SIZE     = 14
};


void Options_DrawMainMenu(const IvyGame *restrict game, IvySceneOptionsData *restrict optionsData)
{
    const IvyVirtualScreen *virtual = game->viewport;
    const Texture2D *cursor = &game->cursors[IVY_CURSOR_PRIMARY];
    const float scale = virtual->scale;

    const float menuY = VIRTUAL_HEIGHT - MARGIN_BOTTOM - (OPT_COUNT - 1) * MENU_SPACING;

    // Calculate text height for centering cursor
    const float textHeight = TEXT_SIZE * scale;
    const float cursorHeight = (float)cursor->height * scale * CURSOR_SCALE;
    const float verticalOffset = (textHeight - cursorHeight) * 0.25f;

    optionsData->targetY = menuY + (float)optionsData->selected * MENU_SPACING + verticalOffset;

    if (optionsData->cursorY == 0.0f) {
        optionsData->cursorY = optionsData->targetY;
    }

    optionsData->cursorY += (optionsData->targetY - optionsData->cursorY) * CURSOR_SPEED;

    DrawTextureEx(*cursor, Ivy_Gfx_GetScreenPos(virtual, (Vector2){ CURSOR_X, optionsData->cursorY }), 0.0f, scale * CURSOR_SCALE, WHITE);

    for (u32 i = 0; i < OPT_COUNT; i++) {
        Ivy_Gfx_DrawLocaleText(game->fonts[IVY_FONT_PRIMARY],
            optionsData->menuStrings[i], optionsData->menuLengths[i],
            Ivy_Gfx_GetScreenPos(virtual, (Vector2){ TEXT_X, menuY + (float)i * MENU_SPACING }),
            TEXT_SIZE * scale, 1,
            i == optionsData->selected ? WHITE : GRAY);
    }

    const float valueScale = TEXT_SIZE * scale;
    char buffer[32];

    snprintf(buffer, sizeof(buffer), "%dx%d", GetScreenWidth(), GetScreenHeight());
    DrawTextEx(game->fonts[IVY_FONT_PRIMARY], buffer,
        Ivy_Gfx_GetScreenPos(virtual, (Vector2){ VALUE_X, menuY }),
        valueScale, 1, YELLOW);

    DrawTextEx(game->fonts[IVY_FONT_PRIMARY],
    IsWindowFullscreen() ? IVY_TR(game->locale, COMMON_ON) : IVY_TR(game->locale, COMMON_OFF),
    Ivy_Gfx_GetScreenPos(virtual, (Vector2){ VALUE_X, menuY + MENU_SPACING }),
    valueScale, 1, YELLOW);

    DrawTextEx(game->fonts[IVY_FONT_PRIMARY],
        IVY_TR(game->locale, SETTINGS_LANGUAGE_VALUE),
        Ivy_Gfx_GetScreenPos(virtual, (Vector2){ VALUE_X, menuY + MENU_SPACING * 2 }),
        valueScale, 1, YELLOW);

    DrawTextEx(game->fonts[IVY_FONT_PRIMARY],
        "Configure...",
        Ivy_Gfx_GetScreenPos(virtual, (Vector2){ VALUE_X, menuY + MENU_SPACING * 3 }),
        valueScale, 1, YELLOW);
}
