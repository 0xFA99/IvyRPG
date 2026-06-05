#include "ivy/audio/buffer.h"
#include "ivy/audio/wav.h"
#include "ivy/core/game.h"
#include "ivy/core/keybind.h"
#include "ivy/core/types.h"
#include "ivy/core/virtual.h"
#include "ivy/scenes/options.h"
#include "ivy/scenes/options_private.h"
#include "ivy/scenes/types.h"
#include "ivy/systems/locale_manager.h"
#include "ivy/systems/profile_manager.h"
#include "ivy/systems/scene_manager.h"
#include "ivy/utils/forward.h"

#include "raylib/raylib.h"

#include <stdbool.h>

#define OPTIONS_SCREEN_SIZE_MAX 5
#define OPTIONS_KEYBIND_VISIBLE 8

static const struct { u16 width, height; } SCREEN_SIZE[OPTIONS_SCREEN_SIZE_MAX] = {
    {  640,  360 },
    {  960,  540 },
    { 1280,  720 },
    { 1600,  900 },
    { 1920, 1080 },
};

static u32 _ivyOptions_GetScreenSizeIndex(void)
{
    const u32 w = (u32)GetScreenWidth();
    const u32 h = (u32)GetScreenHeight();

    for (u32 i = 0; i < OPTIONS_SCREEN_SIZE_MAX; i++) {
        if (SCREEN_SIZE[i].width == w && SCREEN_SIZE[i].height == h)
            return i;
    }
    return 0;
}

static u8 _ivyOptions_ClampScrollOffset(const u8 selected, const u8 offset)
{
    if (selected < offset)
        return selected;

    if (selected >= offset + OPTIONS_KEYBIND_VISIBLE)
        return selected - OPTIONS_KEYBIND_VISIBLE + 1;

    return offset;
}

static void _ivyOptions_ApplyResolution(const IvyGame *game, const u32 index)
{
    SetWindowSize(SCREEN_SIZE[index].width, SCREEN_SIZE[index].height);
    Ivy_VirtualScreen_Update(game->viewport, (Vector2){ (float)SCREEN_SIZE[index].width, (float)SCREEN_SIZE[index].height });

    game->sceneManager->actionScene->needsRebuild = true;
}

void _ivyOptions_ReloadLocales(IvySceneOptionsData *restrict optionsData, const IvyLocale *restrict locale)
{
    static const _IvyOptionsLocaleIndexKey KEYS[OPTIONS_LOCALE_COUNT] = {
        [OPTIONS_LOCALE_SCREEN_SIZE] = OPTIONS_LOCALE_INDEX_SCREEN_SIZE_KEY,
        [OPTIONS_LOCALE_FULLSCREEN]  = OPTIONS_LOCALE_INDEX_FULLSCREEN,
        [OPTIONS_LOCALE_LANGUAGE]    = OPTIONS_LOCALE_INDEX_LANGUAGE,
        [OPTIONS_LOCALE_KEYBIND]     = OPTIONS_LOCALE_INDEX_KEYBIND,
        [OPTIONS_LOCALE_BACK]        = OPTIONS_LOCALE_INDEX_BACK,
    };

    for (u32 i = 0; i < OPTIONS_LOCALE_COUNT; i++) {
        optionsData->locale.strings[i] = IVY_TR(locale, (IvyLocaleKey)KEYS[i]);
        optionsData->locale.lengths[i] = IVY_TR_LEN(locale, (IvyLocaleKey)KEYS[i]);
    }
}

static void _ivyOptions_UpdateKeybind(IvyGame *restrict game, IvySceneOptionsData *restrict od)
{
    const int confirmKey = Ivy_Keybind_GetCurrentKey(game->keybind, IVY_KEY_CONFIRM);
    const int cancelKey  = Ivy_Keybind_GetCurrentKey(game->keybind, IVY_KEY_CANCEL);

    switch (od->keybind.state)
    {
        case OPTIONS_CONFIG_KEY_SELECTING: {
            const int dir = IsKeyPressed(Ivy_Keybind_GetCurrentKey(game->keybind, IVY_KEY_DOWN))
                          - IsKeyPressed(Ivy_Keybind_GetCurrentKey(game->keybind, IVY_KEY_UP));

            if (dir) {
                od->keybind.selected = (u8)((od->keybind.selected + dir + IVY_KEY_MAX) % IVY_KEY_MAX);
                od->keybind.scrollOffset = _ivyOptions_ClampScrollOffset(
                    od->keybind.selected, od->keybind.scrollOffset);

                Ivy_Audio_PlayAudioBuffer(od->sound.data.stream.buffer);
            }

            if (IsKeyPressed(confirmKey)) {
                od->keybind.state     = OPTIONS_CONFIG_KEY_WAITING;
                od->keybind.isWaiting = true;
            }

            if (IsKeyPressed(cancelKey) || IsKeyPressed(KEY_ESCAPE))
                od->keybind.state = OPTIONS_CONFIG_KEY_CLOSED;

            break;
        }

        case OPTIONS_CONFIG_KEY_WAITING: {
            if (!od->keybind.isWaiting) break;

            const int pressed = GetKeyPressed();
            if (pressed != 0 && pressed != cancelKey && pressed != KEY_ESCAPE) {
                Ivy_Keybind_Update(game->saveManager, game->keybind[od->keybind.selected].key, pressed);
                Ivy_SaveManager_Flush(game->saveManager);

                od->keybind.isWaiting = false;
                od->keybind.state     = OPTIONS_CONFIG_KEY_SELECTING;
            }

            if (IsKeyPressed(KEY_ESCAPE)) {
                od->keybind.isWaiting = false;
                od->keybind.state     = OPTIONS_CONFIG_KEY_SELECTING;
            }
            break;
        }

        default: break;
    }
}

static void _ivyOptions_UpdateLocale(IvyGame *restrict game, IvySceneOptionsData *restrict od)
{
    Ivy_Locale_Next(game->locale);

    const u32 hash = game->locale->hashID;
    game->saveManager->save->profile.localeID = hash;
    Ivy_Locale_Update(game->assetManager, game->locale, hash);
    Ivy_SaveManager_Flush(game->saveManager);

    _ivyOptions_ReloadLocales(od, game->locale);
}

void Ivy_Scene_OptionsUpdate(IvyGame *game)
{
    IvySceneOptionsData *optionsData = game->sceneManager->actionScene->data;

    if (optionsData->keybind.state != OPTIONS_CONFIG_KEY_CLOSED) {
        _ivyOptions_UpdateKeybind(game, optionsData);
        return;
    }

    const int confirmKey = Ivy_Keybind_GetCurrentKey(game->keybind, IVY_KEY_CONFIRM);
    const int cancelKey  = Ivy_Keybind_GetCurrentKey(game->keybind, IVY_KEY_CANCEL);

    const int direction = IsKeyPressed(Ivy_Keybind_GetCurrentKey(game->keybind, IVY_KEY_DOWN))
                        - IsKeyPressed(Ivy_Keybind_GetCurrentKey(game->keybind, IVY_KEY_UP));

    if (direction) {
        optionsData->selected = (u32)((optionsData->selected + direction + OPTIONS_LOCALE_COUNT) % OPTIONS_LOCALE_COUNT);
        Ivy_Audio_PlayAudioBuffer(optionsData->sound.data.stream.buffer);
    }

    if (IsKeyPressed(confirmKey))
    {
        switch (optionsData->selected)
        {
            case OPTIONS_LOCALE_SCREEN_SIZE:
                _ivyOptions_ApplyResolution(game, (_ivyOptions_GetScreenSizeIndex() + 1) % OPTIONS_SCREEN_SIZE_MAX);
                optionsData->cursorY = optionsData->targetY = 0.0f;
                break;

            case OPTIONS_LOCALE_FULLSCREEN:
                ToggleFullscreen();
                Ivy_VirtualScreen_Update(game->viewport, (Vector2){ (float)GetScreenWidth(), (float)GetScreenHeight() });
                SetTextureFilter(game->viewport->target.texture, TEXTURE_FILTER_POINT);
                break;

            case OPTIONS_LOCALE_LANGUAGE:
                _ivyOptions_UpdateLocale(game, optionsData);
                break;

            case OPTIONS_LOCALE_KEYBIND:
                optionsData->keybind.state        = OPTIONS_CONFIG_KEY_SELECTING;
                optionsData->keybind.selected     = 0;
                optionsData->keybind.scrollOffset = 0;
                optionsData->keybind.cursorY      = 0;
                optionsData->keybind.targetY      = 0;
                break;

            case OPTIONS_LOCALE_BACK:
                Ivy_SceneManager_Transition(game, SCENE_TITLE);
                break;

            default: break;
        }
    }

    if (IsKeyPressed(cancelKey) || IsKeyPressed(KEY_ESCAPE)) {
        Ivy_SceneManager_Transition(game, SCENE_TITLE);
    }
}
