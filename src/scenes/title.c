#include "ivy/arena/linear.h"
#include "ivy/audio/buffer.h"
#include "ivy/audio/ogg.h"
#include "ivy/audio/stream.h"
#include "ivy/audio/wav.h"
#include "ivy/core/game.h"
#include "ivy/core/keybind.h"
#include "ivy/core/types.h"
#include "ivy/core/virtual.h"
#include "ivy/graphics/gfx.h"
#include "ivy/scenes/title.h"
#include "ivy/scenes/types.h"
#include "ivy/systems/locale_manager.h"
#include "ivy/systems/scene_manager.h"
#include "ivy/systems/texture_manager.h"
#include "ivy/utils/file_ids.h"
#include "ivy/utils/forward.h"

#define TITLE_MAIN_MENU_LOCALE_OFFSET 5
#define TITLE_MAIN_MENU_CURSOR_X      10
#define TITLE_MAIN_MENU_TEXT_X        28
#define TITLE_MAIN_MENU_SPACING       16
#define TITLE_MAIN_MENU_TEXT_SIZE     14
#define TITLE_MAIN_MENU_MARGIN_BOTTOM 36

void Ivy_Scene_TitleInit(IvyGame *game)
{
    IvySceneTitleData *titleData = Ivy_Arena_LinearAllocZero(&game->arena, sizeof(*titleData));
    IVY_ENSURE(titleData);

    const IvyLocale *locale = game->locale;
    for (u32 i = 0; i < MENU_COUNT; i++) {
        const u32 key = i + TITLE_MAIN_MENU_LOCALE_OFFSET;
        titleData->menuStrings[i] = IVY_TR(locale, (IvyLocaleKey)key);
        titleData->menuLengths[i] = IVY_TR_LEN(locale, (IvyLocaleKey)key);
    }

    titleData->targetY = 0.0f;

    titleData->music = Ivy_Audio_LoadMusicOGG(&game->arena, game->assetManager, ASSET_MUSIC_BARREN_AMBIENCE_OGG, 199769);
    PlayMusicStream(titleData->music);

    titleData->sound = Ivy_Audio_LoadSoundWav(&game->arena, game->assetManager, ASSET_AUDIO_CURSOR_WAV);
    game->sceneManager->actionScene->data = titleData;
}

void Ivy_Scene_TitleUpdate(IvyGame *game)
{
    IvySceneTitleData *titleData = game->sceneManager->actionScene->data;

    Ivy_Audio_UpdateMusicOGG(&titleData->music);

    const int direction = IsKeyPressed(Ivy_Keybind_GetCurrentKey(game->keybind, IVY_KEY_DOWN))
                        - IsKeyPressed(Ivy_Keybind_GetCurrentKey(game->keybind, IVY_KEY_UP));

    if (direction != 0) {
        Ivy_Audio_PlayAudioBuffer(titleData->sound.data.stream.buffer);
        titleData->selected = (titleData->selected + direction + MENU_COUNT) % MENU_COUNT;
    }

    // Confirm
    if (IsKeyPressed(game->keybind[IVY_KEY_CONFIRM].currentKey))
    {
        switch (titleData->selected) {
            case MENU_NEW_GAME:
            case MENU_CONTINUE:
                Ivy_SceneManager_Transition(game, SCENE_GAMEPLAY);
                break;

            case MENU_OPTIONS:
                Ivy_SceneManager_Transition(game, SCENE_OPTIONS);
                break;

            case MENU_EXIT:
                game->sceneManager->shouldExit = true;
                break;

            default: break;
        }
    }

    if (IsKeyPressed(KEY_ESCAPE)) game->sceneManager->shouldExit = true;
}

void Ivy_Scene_TitleDrawWorld(IvyGame *game)
{
    const Texture2D background = Ivy_TextureManager_Get(game->textureManager, ASSET_TEXTURES_BACKGROUND_DDS);

    DrawTexturePro(background,
        (Rectangle){0, 0, (float)background.width, (float)background.height},
        (Rectangle){0, 0, VIRTUAL_WIDTH, VIRTUAL_HEIGHT},
        (Vector2){0}, 0.0f, WHITE);
}

void Ivy_Scene_TitleDrawUI(IvyGame *game)
{
    IvySceneTitleData *titleData     = game->sceneManager->actionScene->data;
    const IvyVirtualScreen *viewport = game->viewport;
    const Texture2D cursorTex        = Ivy_TextureManager_Get(game->textureManager, ASSET_TEXTURES_CURSOR_WHITE_DDS);
    const float scale                = viewport->scale;

    // Menu start position
    const float menuY = VIRTUAL_HEIGHT - TITLE_MAIN_MENU_MARGIN_BOTTOM - (MENU_COUNT - 1) * TITLE_MAIN_MENU_SPACING;

    // Calculate text height for centering cursor
    const float textHeight = TITLE_MAIN_MENU_TEXT_SIZE * scale;
    const float cursorHeight = (float)cursorTex.height * scale * 0.5f;
    const float verticalOffset = (textHeight - cursorHeight) * 0.25f;

    // Cursor target
    if (titleData->targetY == 0.0f) {
        titleData->targetY = menuY + (float)titleData->selected * TITLE_MAIN_MENU_SPACING + verticalOffset;
        titleData->cursorY = titleData->targetY;
    } else {
        titleData->targetY = menuY + (float)titleData->selected * TITLE_MAIN_MENU_SPACING + verticalOffset;
    }

    // Smooth cursor
    titleData->cursorY += (titleData->targetY - titleData->cursorY) * 0.4f;

    // Draw cursor
    DrawTextureEx(cursorTex,
        Ivy_Gfx_GetScreenPos(viewport, (Vector2){ TITLE_MAIN_MENU_CURSOR_X, titleData->cursorY }),
        0.0f, scale * 0.5f, WHITE);

    // Draw menu items
    for (u32 i = 0; i < MENU_COUNT; i++) {
        Ivy_Gfx_DrawLocaleText(
            game->fonts[IVY_FONT_PRIMARY],
            titleData->menuStrings[i],
            titleData->menuLengths[i],
            Ivy_Gfx_GetScreenPos(viewport, (Vector2){TITLE_MAIN_MENU_TEXT_X, menuY + (float)i * TITLE_MAIN_MENU_SPACING}),
            TITLE_MAIN_MENU_TEXT_SIZE * scale,
            1,
            i == titleData->selected ? WHITE : GRAY
        );
    }
}

void Ivy_Scene_TitleRebuildTextures(IvyGame *game)
{
    (void)game;
}

void Ivy_Scene_TitleUnload(IvySceneManager *sceneManager)
{
    if (!sceneManager->actionScene->data) return;

    const IvySceneTitleData *sd = sceneManager->actionScene->data;
    Ivy_Audio_UnloadStream(&sd->music);
    Ivy_Audio_UnloadSound(&sd->sound);

    sceneManager->actionScene->data = NULL;
}
