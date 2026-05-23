#include "ivy/audio/buffer.h"
#include "ivy/core/types.h"
#include "ivy/core/game.h"
#include "ivy/core/keybind.h"
#include "ivy/scenes/options.h"
#include "ivy/systems/scene_manager.h"

extern void Options_UpdateKeybindPopup(IvyGame *restrict game, IvySceneOptionsData *restrict optionsData);
extern void Options_ApplyResolution(const IvyGame *game, u32 index);
extern u32  Options_GetScreenSizeIndex(void);
extern void Options_ToggleFullscreenMode(IvyGame *game);
extern void Options_CycleLocale(IvyGame *restrict game, IvySceneOptionsData *restrict optionsData);

extern u32 SCREEN_SIZE_COUNT;

void Options_UpdateMainMenu(IvyGame *restrict game, IvySceneOptionsData *restrict optionsData)
{
    if (optionsData->keybindState != KEYBIND_STATE_CLOSED) {
        Options_UpdateKeybindPopup(game, optionsData);
        return;
    }

    const int direction = IsKeyPressed(game->keybind[IVY_KEY_DOWN].currentKey)
                        - IsKeyPressed(game->keybind[IVY_KEY_UP].currentKey);

    if (direction) {
        optionsData->selected = (optionsData->selected + direction + OPT_COUNT) % OPT_COUNT;
        Ivy_Audio_PlayAudioBuffer(optionsData->sound.data.stream.buffer);
    }

    if (IsKeyPressed(game->keybind[IVY_KEY_CONFIRM].currentKey))
    {
        switch (optionsData->selected)
        {
            case OPT_SCREEN_SIZE:
                Options_ApplyResolution(game, (Options_GetScreenSizeIndex() + 1) % SCREEN_SIZE_COUNT);
                optionsData->cursorY = optionsData->targetY = 0.0f;
                break;

            case OPT_FULLSCREEN:
                Options_ToggleFullscreenMode(game);
                optionsData->cursorY = optionsData->targetY = 0.0f;
                break;

            case OPT_LANGUAGE:
                Options_CycleLocale(game, optionsData);
                break;

            case OPT_KEYBIND:
                optionsData->keybindState        = KEYBIND_STATE_SELECTING;
                optionsData->keybindSelected     = 0;
                optionsData->keybindScrollOffset = 0;
                optionsData->keybindCursorY      = 0.0f;
                optionsData->keybindTargetY      = 0.0f;
                break;

            case OPT_BACK:
                Ivy_SceneManager_Transition(game, SCENE_TITLE);
                break;

            default: break;
        }
    }

    if (IsKeyPressed(KEY_ESCAPE)) {
        Ivy_SceneManager_Transition(game, SCENE_TITLE);
    }
}
