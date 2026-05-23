#include "ivy/core/game.h"
#include "ivy/core/keybind.h"
#include "ivy/audio/buffer.h"
#include "ivy/systems/locale_manager.h"
#include "ivy/systems/scene_manager.h"
#include "ivy/scenes/title.h"

#include "ivy/audio/ogg.h"
#include "ivy/audio/stream.h"
#include "ivy/graphics/gfx.h"
#include "ivy/utils/file_ids.h"

#include "raylib/rlgl.h"

enum {
    LOCALE_OFFSET  = 5,
    CURSOR_X       = 10,
    TEXT_X         = 28,
    MENU_SPACING   = 16,
    TEXT_SIZE      = 14,
    MARGIN_BOTTOM  = 36,
};

static const float CURSOR_SPEED = 0.4f;
static const float CURSOR_SCALE = 0.5f;

void Ivy_Scene_TitleInit(IvyGame *g)
{
    IvySceneTitleData *sd = Ivy_Arena_LinearAllocZero(&g->arena, sizeof(*sd));
    IVY_ENSURE(sd);

    sd->background = Ivy_Gfx_LoadTextureDDS(g->assets, ASSET_TEXTURES_BACKGROUND_DDS);
    SetTextureFilter(sd->background, TEXTURE_FILTER_POINT);

    const IvyLocale *loc = g->locale;
    for (u32 i = 0; i < MENU_COUNT; i++) {
        const u32 key = i + LOCALE_OFFSET;
        sd->menuStrings[i] = IVY_TR(loc, (IvyLocaleKey)key);
        sd->menuLengths[i] = IVY_TR_LEN(loc, (IvyLocaleKey)key);
    }

    sd->targetY = 0.0f;

    sd->music = Ivy_Audio_LoadMusicOGG(&g->arena, g->assets, ASSET_MUSIC_BARREN_AMBIENCE_OGG, 199769);
    PlayMusicStream(sd->music);

    sd->sound = Ivy_Audio_LoadSoundWav(&g->arena, g->assets, ASSET_AUDIO_CURSOR_WAV);
    g->scenes->actionScene->data = sd;
}

void Ivy_Scene_TitleUpdate(IvyGame *g)
{
    IvySceneTitleData *sd = g->scenes->actionScene->data;

    Ivy_Audio_UpdateMusicOGG(&sd->music);

    // Navigation (branchless modulo)
    const int dir = IsKeyPressed(KEY_DOWN) - IsKeyPressed(KEY_UP);

    if (dir != 0) {
        Ivy_Audio_PlayAudioBuffer(sd->sound.data.stream.buffer);
        sd->selected = (sd->selected + dir + MENU_COUNT) % MENU_COUNT;
    }

    // Confirm
    if (IsKeyPressed(g->keybind[IVY_KEY_CONFIRM].currentKey)) {
        switch (sd->selected) {
            case MENU_NEW_GAME:
            case MENU_CONTINUE:
                Ivy_SceneManager_Transition(g, SCENE_GAMEPLAY);
                break;
            case MENU_OPTIONS:
                Ivy_SceneManager_Transition(g, SCENE_OPTIONS);
                break;
            case MENU_EXIT: {
                g->scenes->shouldExit = true;
            } break;
            default: break;
        }
    }

    if (IsKeyPressed(KEY_ESCAPE)) g->scenes->shouldExit = true;
}

void Ivy_Scene_TitleDrawWorld(IvyGame *g)
{
    const IvySceneTitleData *sd = g->scenes->actionScene->data;
    DrawTexturePro(sd->background,
        (Rectangle){0, 0, (float)sd->background.width, (float)sd->background.height},
        (Rectangle){0, 0, VIRTUAL_WIDTH, VIRTUAL_HEIGHT},
        (Vector2){0}, 0.0f, WHITE);
}

void Ivy_Scene_TitleDrawUI(IvyGame *g)
{
    IvySceneTitleData *sd = g->scenes->actionScene->data;
    const IvyVirtualScreen *vr = g->viewport;
    const Texture2D *cursor = &g->cursors[IVY_CURSOR_PRIMARY];
    const float scale = vr->scale;

    // Menu start position
    const float menuY = VIRTUAL_HEIGHT - MARGIN_BOTTOM - (MENU_COUNT - 1) * MENU_SPACING;

    // Calculate text height for centering cursor
    const float textHeight = TEXT_SIZE * scale;
    const float cursorHeight = (float)cursor->height * scale * CURSOR_SCALE;
    const float verticalOffset = (textHeight - cursorHeight) * 0.25f;

    // Cursor target
    if (sd->targetY == 0.0f) {
        sd->targetY = menuY + (float)sd->selected * MENU_SPACING + verticalOffset;
        sd->cursorY = sd->targetY;
    } else {
        sd->targetY = menuY + (float)sd->selected * MENU_SPACING + verticalOffset;
    }

    // Smooth cursor
    sd->cursorY += (sd->targetY - sd->cursorY) * CURSOR_SPEED;

    // Draw cursor
    DrawTextureEx(*cursor,
        Ivy_Gfx_GetScreenPos(vr, (Vector2){CURSOR_X, sd->cursorY}),
        0.0f, scale * CURSOR_SCALE, WHITE);

    // Draw menu items
    for (u32 i = 0; i < MENU_COUNT; i++) {
        Ivy_Gfx_DrawLocaleText(
            g->fonts[IVY_FONT_PRIMARY],
            sd->menuStrings[i],
            sd->menuLengths[i],
            Ivy_Gfx_GetScreenPos(vr, (Vector2){TEXT_X, menuY + (float)i * MENU_SPACING}),
            TEXT_SIZE * scale,
            1,
            i == sd->selected ? WHITE : GRAY
        );
    }
}

void Ivy_Scene_TitleRebuildTextures(IvyGame *g)
{
    (void)g;
}

void Ivy_Scene_TitleUnload(IvySceneManager *sm)
{
    if (!sm->actionScene->data) return;

    const IvySceneTitleData *sd = sm->actionScene->data;
    rlUnloadTexture(sd->background.id);
    Ivy_Audio_UnloadStream(&sd->music);
    Ivy_Audio_UnloadSound(&sd->sound);

    sm->actionScene->data = NULL;
}
