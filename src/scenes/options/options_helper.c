#include "ivy/core/game.h"
#include "ivy/systems/locale_manager.h"
#include "ivy/systems/scene_manager.h"
#include "ivy/scenes/options.h"
#include "ivy/graphics/gfx.h"

#include <stdio.h>

#include "ivy/systems/profile_manager.h"

extern const float  CURSOR_SPEED;
extern const float  CURSOR_SCALE;
extern const u32    LOCALE_ASSETS[];
extern const u32    LOCALE_COUNT;
extern const struct { u16 w, h; } SCREEN_SIZES[];
extern const u32    SCREEN_SIZE_COUNT;
extern const u16    MENU_LOCALE_KEYS[];

enum {
    CURSOR_X        = 10,
    TEXT_X          = 28,
    VALUE_X         = 108,
    MENU_SPACING    = 16,
    TEXT_SIZE       = 14,
    MARGIN_BOTTOM   = 30,
    POPUP_WIDTH     = 200,
    POPUP_HEIGHT    = 160,
    POPUP_PADDING   = 10,
    KEYBIND_ITEM_H  = 14,
    KEYBIND_VISIBLE = 8,
    KEYBIND_VALUE_X = 120
};

void Options_ReloadMenuStrings(IvySceneOptionsData *restrict sd, const IvyLocale *restrict loc)
{
    for (u32 i = 0; i < OPT_COUNT; i++)
    {
        sd->menuStrings[i] = IVY_TR(loc, (IvyLocaleKey)MENU_LOCALE_KEYS[i]);
        sd->menuLengths[i] = IVY_TR_LEN(loc, (IvyLocaleKey)MENU_LOCALE_KEYS[i]);
    }
}

u32 Options_GetScreenSizeIndex(void)
{
    const u32 w = (u32)GetScreenWidth();
    const u32 h = (u32)GetScreenHeight();

    for (u32 i = 0; i < SCREEN_SIZE_COUNT; i++) {
        if (SCREEN_SIZES[i].w == w && SCREEN_SIZES[i].h == h) return i;
    }

    return 0;
}

void Options_ApplyResolution(const IvyGame *game, const u32 index)
{
    SetWindowSize(SCREEN_SIZES[index].w, SCREEN_SIZES[index].h);
    Ivy_VirtualScreen_Update(game->viewport, (Vector2){ (float)SCREEN_SIZES[index].w, (float)SCREEN_SIZES[index].h });
    SetTextureFilter(game->viewport->target.texture, TEXTURE_FILTER_POINT);

    game->scenes->actionScene->needsRebuild = true;
}

void Options_ToggleFullscreenMode(const IvyGame *g)
{
    ToggleFullscreen();
    Ivy_VirtualScreen_Update(g->viewport, (Vector2){ (float)GetScreenWidth(), (float)GetScreenHeight() });
    SetTextureFilter(g->viewport->target.texture, TEXTURE_FILTER_POINT);
}

void Options_CycleLocale(IvyGame *restrict g, IvySceneOptionsData *restrict sd)
{
    Ivy_Locale_Next(g->locale);

    const u32 nextHash = g->locale->hashID;

    g->saveManager->save->profile.localeID = nextHash;

    Ivy_Locale_Update(g->assets, g->locale, nextHash);
    IVY_ENSURE(g->locale);

    Ivy_SaveManager_Flush(g->saveManager);
    Options_ReloadMenuStrings(sd, g->locale);
}

const char *IvyGetKeyName(const int key)
{
    switch (key) {
        case KEY_SPACE: return "Space";
        case KEY_ENTER: return "Enter";
        case KEY_ESCAPE: return "Esc";
        case KEY_UP: return "Up";
        case KEY_DOWN: return "Down";
        case KEY_LEFT: return "Left";
        case KEY_RIGHT: return "Right";
        case KEY_TAB: return "Tab";
        case KEY_BACKSPACE: return "Backspace";
        case KEY_LEFT_SHIFT: case KEY_RIGHT_SHIFT: return "Shift";
        case KEY_LEFT_CONTROL: case KEY_RIGHT_CONTROL: return "Ctrl";
        case KEY_LEFT_ALT: case KEY_RIGHT_ALT: return "Alt";
        default: break;
    }
    if (key >= KEY_A && key <= KEY_Z) {
        static char buf[2];
        buf[0] = (char)('A' + (key - KEY_A));
        buf[1] = '\0';
        return buf;
    }
    if (key >= KEY_ZERO && key <= KEY_NINE) {
        static char buf[2];
        buf[0] = (char)('0' + (key - KEY_ZERO));
        buf[1] = '\0';
        return buf;
    }
    if (key >= KEY_F1 && key <= KEY_F12) {
        static char buf[4];
        snprintf(buf, sizeof(buf), "F%d", (key - KEY_F1 + 1));
        return buf;
    }
    return "???";
}

int Options_WaitForKeyPress(void)
{
    for (int key = KEY_SPACE; key <= KEY_RIGHT_ALT; key++) {
        if (IsKeyPressed(key)) {
            if (key == KEY_ESCAPE) continue;
            return key;
        }
    }
    for (int key = KEY_A; key <= KEY_Z; key++) {
        if (IsKeyPressed(key)) return key;
    }
    for (int key = KEY_ZERO; key <= KEY_NINE; key++) {
        if (IsKeyPressed(key)) return key;
    }
    for (int key = KEY_F1; key <= KEY_F12; key++) {
        if (IsKeyPressed(key)) return key;
    }
    return -1;
}

void Options_DrawPopupBackground(const IvyVirtualScreen *vs, const float x, const float y, const float w, const float h)
{
    DrawRectangle(0, 0, (int)(VIRTUAL_WIDTH * vs->scale), (int)(VIRTUAL_HEIGHT * vs->scale),
        (Color){ 0, 0, 0, 180 });

    const Vector2 popupPos = Ivy_Gfx_GetScreenPos(vs, (Vector2){ x, y });
    const float scaledW    = w * vs->scale;
    const float scaledH    = h * vs->scale;

    DrawRectangle((int)popupPos.x - 2, (int)popupPos.y - 2, (int)(scaledW + 4), (int)(scaledH + 4), (Color){ 60, 60, 80, 255 });
    DrawRectangle((int)popupPos.x, (int)popupPos.y, (int)scaledW, (int)scaledH, (Color){ 30, 30, 45, 255 });
}

void Options_DrawOptionsBackground(const IvyGame *restrict g, const IvySceneOptionsData *restrict sd)
{
    const IvyVirtualScreen *vs = g->viewport;
    const float scale = vs->scale;
    const float menuY = VIRTUAL_HEIGHT - MARGIN_BOTTOM - (OPT_COUNT - 1) * MENU_SPACING;

    for (u32 i = 0; i < OPT_COUNT; i++)
    {
        const Color dimColor = { 80, 80, 80, 100 };

        Ivy_Gfx_DrawLocaleText(g->fonts[IVY_FONT_PRIMARY],
            sd->menuStrings[i], sd->menuLengths[i],
            Ivy_Gfx_GetScreenPos(vs, (Vector2){ TEXT_X, menuY + (float)i * MENU_SPACING }),
            TEXT_SIZE * scale, 1, dimColor);
    }
}
