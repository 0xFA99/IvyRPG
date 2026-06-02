#include "ivy/arena/linear.h"
#include "ivy/audio/buffer.h"
#include "ivy/core/game.h"
#include "ivy/core/keybind.h"
#include "ivy/core/types.h"
#include "ivy/core/virtual.h"
#include "ivy/graphics/gfx.h"
#include "ivy/scenes/options.h"
#include "ivy/scenes/options_private.h"
#include "ivy/scenes/types.h"
#include "ivy/systems/asset_manager.h"
#include "ivy/systems/locale_manager.h"
#include "ivy/systems/scene_manager.h"
#include "ivy/systems/texture_manager.h"
#include "ivy/utils/file_ids.h"
#include "ivy/utils/forward.h"

#include <stdio.h>

#define OPTIONS_MAIN_MENU_TEXT_X      28.0f
#define OPTIONS_MAIN_MENU_VALUE_X    108.0f
#define OPTIONS_MAIN_MENU_CURSOR_X    10.0f
#define OPTIONS_MAIN_MENU_SPACING     16.0f
#define OPTIONS_MAIN_MENU_MARGIN_BOT  30.0f
#define OPTIONS_MAIN_MENU_TEXT_SIZE   14.0f
#define OPTIONS_MAIN_MENU_CURSOR_SPD   0.4f
#define OPTIONS_MAIN_MENU_CURSOR_SCALE 0.5f

#define OPTIONS_KEYBIND_POPUP_W         200.0f
#define OPTIONS_KEYBIND_POPUP_H         160.0f
#define OPTIONS_KEYBIND_POPUP_PAD        10.0f
#define OPTIONS_KEYBIND_POPUP_TEXT_SIZE  14.0f
#define OPTIONS_KEYBIND_POPUP_ITEM_H     14.0f
#define OPTIONS_KEYBIND_POPUP_VISIBLE     8
#define OPTIONS_KEYBIND_POPUP_VALUE_X   120.0f
#define OPTIONS_KEYBIND_POPUP_CURSOR_X   18.0f
#define OPTIONS_KEYBIND_POPUP_CURSOR_SPD  0.5f
#define OPTIONS_KEYBIND_POPUP_CURSOR_SCALE 0.5f

static float _lerpf(const float cur, const float target, const float t)
{
    return cur + (target - cur) * t;
}

static void _drawPopupBox(const IvyVirtualScreen *vs, const float x, const float y, const float w, const float h)
{
    const Vector2 pos = Ivy_Gfx_GetScreenPos(vs, (Vector2){ x, y });
    const float sw = w * vs->scale;
    const float sh = h * vs->scale;

    DrawRectangle(pos.x - 2, pos.y - 2, sw + 4,  sh + 4, (Color){ 60, 60,  80, 255 });
    DrawRectangle(pos.x, pos.y, sw, sh, (Color){ 30, 30,  45, 255 });
}

void Ivy_Scene_OptionsDrawWorld(IvyGame *game)
{
    (void)game;
    ClearBackground((Color){ 20, 20, 30, 225 });
}

static void _ivyOptions_DrawMainMenu(IvyGame *restrict game, IvySceneOptionsData *restrict optionsData)
{
    const IvyVirtualScreen *virtualScreen = game->viewport;
    const float scale = virtualScreen->scale;
    const Texture2D cursorTex = Ivy_TextureManager_Get(game->texManager, ASSET_TEXTURES_CURSOR_WHITE_DDS);

    const float menuY = VIRTUAL_HEIGHT - OPTIONS_MAIN_MENU_MARGIN_BOT - (OPTIONS_LOCALE_COUNT - 1) * OPTIONS_MAIN_MENU_SPACING;

    // Cursor
    {
        const float textH   = OPTIONS_MAIN_MENU_TEXT_SIZE * scale;
        const float cursorH = (float)cursorTex.height * scale * OPTIONS_MAIN_MENU_CURSOR_SCALE;
        const float vOff    = (textH - cursorH) * 0.25f;

        optionsData->targetY = menuY + (float)optionsData->selected * OPTIONS_MAIN_MENU_SPACING + vOff;
        if (optionsData->cursorY == 0.0f) optionsData->cursorY = optionsData->targetY;
        optionsData->cursorY = _lerpf(optionsData->cursorY, optionsData->targetY, OPTIONS_MAIN_MENU_CURSOR_SPD);

        DrawTextureEx(cursorTex,
            Ivy_Gfx_GetScreenPos(virtualScreen, (Vector2){ OPTIONS_MAIN_MENU_CURSOR_X, optionsData->cursorY }),
            0.0f, scale * OPTIONS_MAIN_MENU_CURSOR_SCALE, WHITE);
    }

    // Labels
    for (u32 i = 0; i < OPTIONS_LOCALE_COUNT; i++) {
        Ivy_Gfx_DrawLocaleText(game->fonts[IVY_FONT_PRIMARY],
            optionsData->locale.strings[i], optionsData->locale.lengths[i],
            Ivy_Gfx_GetScreenPos(virtualScreen, (Vector2){ OPTIONS_MAIN_MENU_TEXT_X, menuY + (float)i * OPTIONS_MAIN_MENU_SPACING }),
            OPTIONS_MAIN_MENU_TEXT_SIZE * scale, 1,
            i == optionsData->selected ? WHITE : GRAY);
    }

    // Values
    const float vSize = OPTIONS_MAIN_MENU_TEXT_SIZE * scale;
    char buf[32];

    snprintf(buf, sizeof(buf), "%dx%d", GetScreenWidth(), GetScreenHeight());
    DrawTextEx(game->fonts[IVY_FONT_PRIMARY], buf,
        Ivy_Gfx_GetScreenPos(virtualScreen, (Vector2){ OPTIONS_MAIN_MENU_VALUE_X, menuY + OPTIONS_MAIN_MENU_SPACING * OPTIONS_LOCALE_SCREEN_SIZE }),
        vSize, 1, YELLOW);

    DrawTextEx(game->fonts[IVY_FONT_PRIMARY],
        IsWindowFullscreen() ? IVY_TR(game->locale, COMMON_ON) : IVY_TR(game->locale, COMMON_OFF),
        Ivy_Gfx_GetScreenPos(virtualScreen, (Vector2){ OPTIONS_MAIN_MENU_VALUE_X, menuY + OPTIONS_MAIN_MENU_SPACING * OPTIONS_LOCALE_FULLSCREEN }),
        vSize, 1, YELLOW);

    DrawTextEx(game->fonts[IVY_FONT_PRIMARY],
        IVY_TR(game->locale, SETTINGS_LANGUAGE_VALUE),
        Ivy_Gfx_GetScreenPos(virtualScreen, (Vector2){ OPTIONS_MAIN_MENU_VALUE_X, menuY + OPTIONS_MAIN_MENU_SPACING * OPTIONS_LOCALE_LANGUAGE }),
        vSize, 1, YELLOW);

    DrawTextEx(game->fonts[IVY_FONT_PRIMARY], "Configure...",
        Ivy_Gfx_GetScreenPos(virtualScreen, (Vector2){ OPTIONS_MAIN_MENU_VALUE_X, menuY + OPTIONS_MAIN_MENU_SPACING * OPTIONS_LOCALE_KEYBIND }),
        vSize, 1, YELLOW);
}

static void _ivyOptions_DrawMenuDimmed(const IvyGame *restrict game, const IvySceneOptionsData *restrict od)
{
    const IvyVirtualScreen *vs = game->viewport;
    const float scale = vs->scale;
    const float menuY = VIRTUAL_HEIGHT - OPTIONS_MAIN_MENU_MARGIN_BOT - (OPTIONS_LOCALE_COUNT - 1) * OPTIONS_MAIN_MENU_SPACING;

    for (u32 i = 0; i < OPTIONS_LOCALE_COUNT; i++) {
        Ivy_Gfx_DrawLocaleText(game->fonts[IVY_FONT_PRIMARY],
            od->locale.strings[i], od->locale.lengths[i],
            Ivy_Gfx_GetScreenPos(vs, (Vector2){ OPTIONS_MAIN_MENU_TEXT_X, menuY + (float)i * OPTIONS_MAIN_MENU_SPACING }),
            OPTIONS_MAIN_MENU_TEXT_SIZE * scale, 1, (Color){ 80, 80, 80, 100 });
    }
}

static void _ivyOptions_DrawKeybindPopup(const IvyGame *restrict game, IvySceneOptionsData *restrict od)
{
    const IvyVirtualScreen *virtualScreen = game->viewport;
    const float scale = virtualScreen->scale;
    const float popupX = (VIRTUAL_WIDTH  - OPTIONS_KEYBIND_POPUP_W) * 0.5f;
    const float popupY = (VIRTUAL_HEIGHT - OPTIONS_KEYBIND_POPUP_H) * 0.5f;

    // Dim overlay + box
    DrawRectangle(0, 0, (int)(VIRTUAL_WIDTH  * scale), (int)(VIRTUAL_HEIGHT * scale), (Color){ 0, 0, 0, 180 });
    _drawPopupBox(virtualScreen, popupX, popupY, OPTIONS_KEYBIND_POPUP_W, OPTIONS_KEYBIND_POPUP_H);

    // Title
    {
        const char   *title  = "Keybind Settings";
        const float   tScale = OPTIONS_KEYBIND_POPUP_TEXT_SIZE * scale * 1.2f;
        const Vector2 tSize  = MeasureTextEx(game->fonts[IVY_FONT_PRIMARY], title, tScale, 1);
        const float   titleX = popupX + OPTIONS_KEYBIND_POPUP_W * 0.5f - (tSize.x / scale * 0.5f);

        DrawTextEx(game->fonts[IVY_FONT_PRIMARY], title,
            Ivy_Gfx_GetScreenPos(virtualScreen, (Vector2){ titleX, popupY + OPTIONS_KEYBIND_POPUP_PAD }),
            tScale, 1, WHITE);
    }

    // Separator
    {
        const Vector2 sep = Ivy_Gfx_GetScreenPos(virtualScreen, (Vector2){ popupX + OPTIONS_KEYBIND_POPUP_PAD, popupY + OPTIONS_KEYBIND_POPUP_PAD + OPTIONS_KEYBIND_POPUP_TEXT_SIZE * 2.0f });
        DrawRectangle((int)sep.x, (int)sep.y,
            (int)((OPTIONS_KEYBIND_POPUP_W - OPTIONS_KEYBIND_POPUP_PAD * 2.0f) * scale), 1,
            (Color){ 100, 100, 120, 255 });
    }

    const float listStartY = popupY + OPTIONS_KEYBIND_POPUP_PAD + OPTIONS_KEYBIND_POPUP_TEXT_SIZE * 2.5f;

    // Cursor
    {
        const Texture2D cursorTex = Ivy_TextureManager_Get(game->texManager, ASSET_TEXTURES_CURSOR_WHITE_DDS);
        const float textH   = OPTIONS_KEYBIND_POPUP_TEXT_SIZE * scale * 0.8f;
        const float cursorH = (float)cursorTex.height * scale * OPTIONS_KEYBIND_POPUP_CURSOR_SCALE;
        const float vOff    = (textH - cursorH) * 0.25f;

        const float targetY = listStartY
                            + (float)(od->keybind.selected - od->keybind.scrollOffset) * OPTIONS_KEYBIND_POPUP_ITEM_H
                            + vOff;

        float cursorYf = (float)od->keybind.cursorY;
        if (cursorYf == 0.0f) cursorYf = targetY;
        cursorYf = _lerpf(cursorYf, targetY, OPTIONS_KEYBIND_POPUP_CURSOR_SPD);
        od->keybind.cursorY = (u16)cursorYf;

        DrawTextureEx(cursorTex,
            Ivy_Gfx_GetScreenPos(virtualScreen, (Vector2){ popupX + OPTIONS_KEYBIND_POPUP_PAD, cursorYf }),
            0.0f, scale * OPTIONS_KEYBIND_POPUP_CURSOR_SCALE, WHITE);
    }

    // Item list
    {
        const IvyKeybindInfo *keybinds = Ivy_Keybind_GetKeybindInfo();
        const u32   start     = od->keybind.scrollOffset;
        const u32   end       = (start + OPTIONS_KEYBIND_POPUP_VISIBLE < IVY_KEY_MAX)
                                ? start + OPTIONS_KEYBIND_POPUP_VISIBLE : IVY_KEY_MAX;
        const float itemScale = OPTIONS_KEYBIND_POPUP_TEXT_SIZE * scale * 0.8f;

        for (u32 i = start; i < end; i++) {
            const float  itemY   = listStartY + (float)(i - start) * OPTIONS_KEYBIND_POPUP_ITEM_H;
            const bool   isSel   = (i == (u32)od->keybind.selected);
            const bool   waiting = isSel && od->keybind.state == OPTIONS_CONFIG_KEY_WAITING;

            DrawTextEx(game->fonts[IVY_FONT_PRIMARY], keybinds[i].name,
                Ivy_Gfx_GetScreenPos(virtualScreen, (Vector2){ popupX + OPTIONS_KEYBIND_POPUP_PAD + OPTIONS_KEYBIND_POPUP_CURSOR_X, itemY }),
                itemScale, 1, isSel ? WHITE : GRAY);

            DrawTextEx(game->fonts[IVY_FONT_PRIMARY],
                waiting ? "..." : Ivy_Keybind_GetKeyName(Ivy_Keybind_GetCurrentKey(game->keybind, i)),
                Ivy_Gfx_GetScreenPos(virtualScreen, (Vector2){ popupX + OPTIONS_KEYBIND_POPUP_VALUE_X, itemY }),
                itemScale, 1, waiting ? YELLOW : LIGHTGRAY);
        }
    }

    // Instructions
    {
        const char   *instr  = od->keybind.state == OPTIONS_CONFIG_KEY_WAITING
                             ? "Press any key..."
                             : "Enter: Change          Esc: Back";
        const float   iScale = OPTIONS_KEYBIND_POPUP_TEXT_SIZE * scale * 0.8f;
        const Vector2 iSize  = MeasureTextEx(game->fonts[IVY_FONT_PRIMARY], instr, iScale, 1);
        const float   instrX = popupX + OPTIONS_KEYBIND_POPUP_W * 0.5f - (iSize.x / scale * 0.5f);
        const float   instrY = popupY + OPTIONS_KEYBIND_POPUP_H - OPTIONS_KEYBIND_POPUP_PAD - OPTIONS_KEYBIND_POPUP_TEXT_SIZE;

        DrawTextEx(game->fonts[IVY_FONT_PRIMARY], instr,
            Ivy_Gfx_GetScreenPos(virtualScreen, (Vector2){ instrX, instrY }),
            iScale, 1, (Color){ 180, 180, 200, 255 });
    }
}

void Ivy_Scene_OptionsDrawUI(IvyGame *game)
{
    IvySceneOptionsData *od = game->scenes->actionScene->data;

    if (od->keybind.state != OPTIONS_CONFIG_KEY_CLOSED) {
        _ivyOptions_DrawMenuDimmed(game, od);
        _ivyOptions_DrawKeybindPopup(game, od);
        return;
    }

    _ivyOptions_DrawMainMenu(game, od);
}
