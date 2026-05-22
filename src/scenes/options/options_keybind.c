#include "ivy/core/game.h"
#include "ivy/core/keybind.h"
#include "ivy/graphics/gfx.h"
#include "ivy/scenes/options.h"

extern void Options_DrawPopupBackground(const IvyVirtualScreen *vr, float x, float y, float w, float h);
extern int Options_WaitForKeyPress(void);
extern const char *IvyGetKeyName(int key);

extern const float CURSOR_SCALE;
extern const float KEYBIND_CURSOR_SPEED;

enum {
    CURSOR_X        = 18,
    TEXT_SIZE       = 14,
    POPUP_WIDTH     = 200,
    POPUP_HEIGHT    = 160,
    POPUP_PADDING   = 10,
    KEYBIND_ITEM_H  = 14,
    KEYBIND_VISIBLE = 8,
    KEYBIND_VALUE_X = 120
};

static int GetKeybindScrollOffset(const int selected, const int currentOffset)
{
    if (selected < currentOffset) {
        return selected;
    }

    if (selected >= currentOffset + KEYBIND_VISIBLE) {
        return selected - KEYBIND_VISIBLE + 1;
    }

    return currentOffset;
}

static void DrawKeybindTitle(const IvyGame *g, const IvyVirtualScreen *vr, const float popupX, const float popupY)
{
    const char *title       = "Keybind Settings";
    const float titleScale  = TEXT_SIZE * vr->scale * 1.2f;
    const Vector2 textSize  = MeasureTextEx(g->fonts[IVY_FONT_PRIMARY], title, titleScale, 1);
    const float titleX      = popupX + (POPUP_WIDTH / 2.0f) - (textSize.x / vr->scale / 2.0f);
    const Vector2 titlePos  = Ivy_Gfx_GetScreenPos(vr, (Vector2){ titleX, popupY + POPUP_PADDING });

    DrawTextEx(g->fonts[IVY_FONT_PRIMARY], title, titlePos, titleScale, 1, WHITE);
}

static void DrawSeparator(const IvyVirtualScreen *vr, const float popupX, const float popupY)
{
    const Vector2 sepPos = Ivy_Gfx_GetScreenPos(vr,
        (Vector2){ popupX + POPUP_PADDING, popupY + POPUP_PADDING + TEXT_SIZE * 2 });

    DrawRectangle((int)sepPos.x, (int)sepPos.y,
        (int)((POPUP_WIDTH - POPUP_PADDING * 2) * vr->scale), 1,
        (Color){ 100, 100, 120, 255 });
}

static void DrawCursor(const IvyGame *g, const IvyVirtualScreen *vr, const float popupX, IvySceneOptionsData *sd, const float listStartY)
{
    // Calculate text height for centering cursor
    const float textHeight = TEXT_SIZE * vr->scale * 0.8f;
    const Texture2D *cursor = &g->cursors[IVY_CURSOR_PRIMARY];
    const float cursorHeight = cursor->height * vr->scale * CURSOR_SCALE;
    const float verticalOffset = (textHeight - cursorHeight) * 0.25f;

    const float targetY = listStartY + (float)(sd->keybindSelected - sd->keybindScrollOffset) * KEYBIND_ITEM_H + verticalOffset;
    if (sd->keybindCursorY == 0.0f) sd->keybindCursorY = targetY;

    sd->keybindCursorY += (targetY - sd->keybindCursorY) * KEYBIND_CURSOR_SPEED;

    DrawTextureEx(*cursor,
        Ivy_Gfx_GetScreenPos(vr, (Vector2){ popupX + POPUP_PADDING, sd->keybindCursorY }),
        0.0f, vr->scale * CURSOR_SCALE, WHITE);
}

static void DrawKeybindItems(const IvyGame *g, const IvyVirtualScreen *vr, const float popupX, const float listStartY, const IvySceneOptionsData *sd)
{
    const IvyKeybindInfo *keybinds = Ivy_Keybind_GetKeybindInfo();
    const u32 startIdx = sd->keybindScrollOffset;
    u32 endIdx = startIdx + KEYBIND_VISIBLE;
    if (endIdx > IVY_KEY_MAX) endIdx = IVY_KEY_MAX;

    for (u32 i = startIdx; i < endIdx; i++) {
        const float itemY = listStartY + (float)(i - startIdx) * KEYBIND_ITEM_H;

        // Draw action name
        const Vector2 itemPos = Ivy_Gfx_GetScreenPos(vr,
            (Vector2){ popupX + POPUP_PADDING + CURSOR_X, itemY });

        const Color nameColor = (i == sd->keybindSelected) ? WHITE : GRAY;
        DrawTextEx(g->fonts[IVY_FONT_PRIMARY], keybinds[i].name, itemPos,
                   TEXT_SIZE * vr->scale * 0.8f, 1, nameColor);

        // Draw key name
        const Vector2 keyPos = Ivy_Gfx_GetScreenPos(vr,
            (Vector2){ popupX + KEYBIND_VALUE_X, itemY });
        const Color keyColor = (i == sd->keybindSelected && sd->keybindState == KEYBIND_STATE_WAITING_INPUT)
                        ? YELLOW : LIGHTGRAY;
        const char *keyText = (i == sd->keybindSelected && sd->keybindState == KEYBIND_STATE_WAITING_INPUT)
                             ? "..." : IvyGetKeyName(keybinds[i].currentKey);
        DrawTextEx(g->fonts[IVY_FONT_PRIMARY], keyText, keyPos,
                   TEXT_SIZE * vr->scale * 0.8f, 1, keyColor);
    }
}

static void DrawInstructions(const IvyGame *g, const IvyVirtualScreen *vr, const float popupX, const float popupY, const IvySceneOptionsData *sd)
{
    const char *instruction = sd->keybindState == KEYBIND_STATE_WAITING_INPUT ? "Press any key..." : "Enter: Change          Esc: Back";
    const float instrScale  = TEXT_SIZE * vr->scale * 0.8f;
    const Vector2 instrSize = MeasureTextEx(g->fonts[IVY_FONT_PRIMARY], instruction, instrScale, 1);
    const float instrX      = popupX + (POPUP_WIDTH * 0.5f) - (instrSize.x / vr->scale * 0.5f);
    const Vector2 instrPos  = Ivy_Gfx_GetScreenPos(vr, (Vector2){ instrX, popupY + POPUP_HEIGHT - POPUP_PADDING - TEXT_SIZE });

    DrawTextEx(g->fonts[IVY_FONT_PRIMARY], instruction, instrPos, instrScale, 1,
               (Color){ 180, 180, 200, 255 });
}

void Options_UpdateKeybindPopup(const IvyGame *restrict game, IvySceneOptionsData *restrict optionsData)
{
    const IvyKeybindInfo *keybinds = Ivy_Keybind_GetKeybindInfo();

    switch (optionsData->keybindState) {
        case KEYBIND_STATE_SELECTING: {
            const int direction = IsKeyPressed(game->keybind[IVY_KEY_DOWN].currentKey)
                                - IsKeyPressed(game->keybind[IVY_KEY_UP].currentKey);

            if (direction) {
                optionsData->keybindSelected = (optionsData->keybindSelected + direction + IVY_KEY_MAX) % IVY_KEY_MAX;
                optionsData->keybindScrollOffset = GetKeybindScrollOffset(
                    (int)optionsData->keybindSelected, (int)optionsData->keybindScrollOffset);

                PlaySound(optionsData->sound.data);
            }

            if (IsKeyPressed(game->keybind[IVY_KEY_CONFIRM].currentKey)) {
                optionsData->keybindState = KEYBIND_STATE_WAITING_INPUT;
                optionsData->waitingForKey = true;
            }

            if (IsKeyPressed(game->keybind[IVY_KEY_CANCEL].currentKey) || IsKeyPressed(KEY_ESCAPE)) {
                optionsData->keybindState = KEYBIND_STATE_CLOSED;
            }
            break;
        }

        case KEYBIND_STATE_WAITING_INPUT: {
            if (optionsData->waitingForKey) {
                const int newKey = Options_WaitForKeyPress();
                if (newKey != -1) {
                    Ivy_Keybind_Update(keybinds[optionsData->keybindSelected].key, newKey);
                    optionsData->waitingForKey = false;
                    optionsData->keybindState = KEYBIND_STATE_SELECTING;
                }
            }

            if (IsKeyPressed(KEY_ESCAPE)) {
                optionsData->waitingForKey = false;
                optionsData->keybindState = KEYBIND_STATE_SELECTING;
            }
            break;
        }
        default: break;
    }
}

void Options_DrawKeybindPopup(const IvyGame *restrict g, IvySceneOptionsData *restrict sd)
{
    const IvyVirtualScreen *vr = g->viewport;

    const float popupX = (VIRTUAL_WIDTH - POPUP_WIDTH) * 0.5f;
    const float popupY = (VIRTUAL_HEIGHT - POPUP_HEIGHT) * 0.5f;

    Options_DrawPopupBackground(vr, popupX, popupY, POPUP_WIDTH, POPUP_HEIGHT);

    DrawKeybindTitle(g, vr, popupX, popupY);
    DrawSeparator(vr, popupX, popupY);

    const float listStartY = popupY + POPUP_PADDING + TEXT_SIZE * 2.5f;
    DrawCursor(g, vr, popupX, sd, listStartY);
    DrawKeybindItems(g, vr, popupX, listStartY, sd);
    DrawInstructions(g, vr, popupX, popupY, sd);
}