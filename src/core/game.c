#include "ivy/audio/device.h"
#include "ivy/arena/linear.h"
#include "ivy/core/game.h"
#include "ivy/core/keybind.h"
#include "ivy/core/types.h"
#include "ivy/core/virtual.h"
#include "ivy/graphics/gfx.h"
#include "ivy/scenes/types.h"
#include "ivy/systems/asset_manager.h"
#include "ivy/systems/locale_manager.h"
#include "ivy/systems/profile_manager.h"
#include "ivy/systems/scene_manager.h"
#include "ivy/systems/texture_manager.h"
#include "ivy/utils/file_ids.h"
#include "ivy/utils/forward.h"

#define IVY_ASSET_HEADER    "assets/header.bin"
#define IVY_ASSET_DATA      "assets/data.bin"
#define IVY_SAVE_PATH       "save.bin"

IvyGame Ivy_Game_Init(const Vector2 size)
{
    IvyGame game = {0};

    Ivy_Audio_InitDevice();

    // Arena
    Ivy_Arena_LinearInit(&game.arena, 2412512);

    // Save Manager
    game.saveManager = Ivy_SaveManager_Init(&game.arena, IVY_SAVE_PATH);

    // Game Asset & Locale
    game.assetManager = Ivy_AssetManager_Init(&game.arena, IVY_ASSET_HEADER, IVY_ASSET_DATA);
    game.textureManager = Ivy_TextureManager_Init(&game.arena, game.assetManager);

    // Load locale
    const u32 localeID = game.saveManager->save->profile.localeID;
    game.locale = Ivy_Locale_Load(game.assetManager, localeID, &game.arena);

    // Virtual Resolution
    game.viewport = Ivy_VirtualScreen_Init(&game.arena, size);
    SetTextureFilter(game.viewport->target.texture, TEXTURE_FILTER_POINT);

    // Fonts
    game.fonts[IVY_FONT_PRIMARY]    = Ivy_Gfx_LoadFont(&game.arena, game.assetManager, ASSET_FONTS_DENKONE_METADATA_BIN, ASSET_FONTS_DENKONE_ATLAS_DDS, IVY_FONT_SIZE);
    game.fonts[IVY_FONT_SECONDARY]  = Ivy_Gfx_LoadFont(&game.arena, game.assetManager, ASSET_FONTS_NOTOSANSCJK_METADATA_BIN, ASSET_FONTS_NOTOSANSCJK_ATLAS_DDS, IVY_FONT_SIZE);
    SetTextureFilter(game.fonts[IVY_FONT_PRIMARY].texture,   TEXTURE_FILTER_BILINEAR);
    SetTextureFilter(game.fonts[IVY_FONT_SECONDARY].texture, TEXTURE_FILTER_BILINEAR);

    // Keybind
    game.keybind = Ivy_Keybind_GetKeybindInfo();
    Ivy_Keybind_Load(game.saveManager);

    Ivy_Audio_InitPcmScratch(&game.arena);

    // Scene Manager
    game.sceneManager = Ivy_SceneManager_Init(&game, game.assetManager);

    return game;
}

void Ivy_Game_Update(IvyGame *game)
{
    Ivy_SceneManager_RebuildIfNeeded(game);

    // Urgent Reset Keybind
    if (IsKeyPressed(KEY_F5)) {
        Ivy_Keybind_Reset(game->saveManager);
        Ivy_SaveManager_Flush(game->saveManager);
    }

    if (IVY_LIKELY(game->sceneManager->actionScene->table->Update)) {
        game->sceneManager->actionScene->table->Update(game);
    }

    // Update Virtual Resolution
    if (IsWindowResized()) {
        Ivy_VirtualScreen_Update(game->viewport, (Vector2){(float)GetScreenWidth(), (float)GetScreenHeight()});
    }
}

void Ivy_Game_Draw(IvyGame *game)
{
    BeginTextureMode(game->viewport->target);
        ClearBackground(BLANK);
        game->sceneManager->actionScene->table->DrawWorld(game);
    EndTextureMode();

    BeginDrawing();
        ClearBackground(BLACK);
        BeginBlendMode(BLEND_ALPHA_PREMULTIPLY);
            Ivy_VirtualScreen_Draw(game->viewport);
        EndBlendMode();

        game->sceneManager->actionScene->table->DrawUI(game);
        DrawFPS(10, 10);
    EndDrawing();
}

void Ivy_Game_Destroy(IvyGame *game)
{
    IVY_ASSERT(game, "[Game] Instance is NULL!");

    const IvyScene *s = game->sceneManager->actionScene;
    if (IVY_LIKELY(s && s->table->Unload)) s->table->Unload(game->sceneManager);

    Ivy_Gfx_UnloadFont(&game->fonts[IVY_FONT_PRIMARY]);
    Ivy_Gfx_UnloadFont(&game->fonts[IVY_FONT_SECONDARY]);

    Ivy_VirtualScreen_Unload(game->viewport);

    Ivy_TextureManager_Destroy(game->textureManager);

    Ivy_Audio_CloseDevice();

    Ivy_Arena_LinearDestroy(&game->arena);
}
