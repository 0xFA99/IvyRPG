#include "ivy/core/game.h"

#include "ivy/core/keybind.h"
#include "ivy/systems/asset_manager.h"
#include "ivy/systems/scene_manager.h"
#include "ivy/systems/locale_manager.h"
#include "ivy/graphics/gfx.h"
#include "ivy/utils/file_ids.h"

#include "raylib/rlgl.h"

#define IVY_ASSET_HEADER            "assets/header.bin"
#define IVY_ASSET_DATA              "assets/data.bin"
#define IVY_FONT_SIZE               64

IvyGame Ivy_Game_Init(const Vector2 size)
{
    IvyGame game = {0};

    // Arena
    Ivy_Arena_LinearInit(&game.arenas[IVY_ARENA_LOCALE], 112);
    Ivy_Arena_LinearInit(&game.arenas[IVY_ARENA_MAIN], 924928);

    // Game Asset & Locale
    game.assets = Ivy_AssetManager_Init(&game.arenas[IVY_ARENA_MAIN], IVY_ASSET_HEADER, IVY_ASSET_DATA);
    game.locale = Ivy_Locale_Load(game.assets, ASSET_LOCALES_EN_BIN, &game.arenas[IVY_ARENA_LOCALE]);

    // Virtual Resolution
    game.viewport = Ivy_VirtualScreen_Init(&game.arenas[IVY_ARENA_MAIN], size);
    SetTextureFilter(game.viewport->target.texture, TEXTURE_FILTER_POINT);

    // Fonts
    game.fonts[IVY_FONT_PRIMARY]    = Ivy_Gfx_LoadFont(&game.arenas[IVY_ARENA_MAIN], game.assets, ASSET_FONTS_DENKONE_METADATA_BIN, ASSET_FONTS_DENKONE_ATLAS_DDS, IVY_FONT_SIZE);
    game.fonts[IVY_FONT_SECONDARY]  = Ivy_Gfx_LoadFont(&game.arenas[IVY_ARENA_MAIN], game.assets, ASSET_FONTS_NOTOSANSCJK_METADATA_BIN, ASSET_FONTS_NOTOSANSCJK_ATLAS_DDS, IVY_FONT_SIZE);
    SetTextureFilter(game.fonts[IVY_FONT_PRIMARY].texture,   TEXTURE_FILTER_BILINEAR);
    SetTextureFilter(game.fonts[IVY_FONT_SECONDARY].texture, TEXTURE_FILTER_BILINEAR);

    // Cursors
    game.cursors[IVY_CURSOR_PRIMARY]   = Ivy_Gfx_LoadTextureDDS(game.assets, ASSET_TEXTURES_CURSOR_WHITE_DDS);
    game.cursors[IVY_CURSOR_SECONDARY] = Ivy_Gfx_LoadTextureDDS(game.assets, ASSET_TEXTURES_CURSOR_YELLOW_DDS);
    SetTextureFilter(game.cursors[IVY_CURSOR_PRIMARY],   TEXTURE_FILTER_BILINEAR);
    SetTextureFilter(game.cursors[IVY_CURSOR_SECONDARY], TEXTURE_FILTER_BILINEAR);

    // Keybind
    game.keybind = Ivy_Keybind_GetKeybindInfo();

    // Scene Manager
    game.scenes = Ivy_SceneManager_Init(&game, game.assets);

    return game;
}

void Ivy_Game_Update(IvyGame *game)
{
    Ivy_SceneManager_RebuildIfNeeded(game);

    if (IVY_LIKELY(game->scenes->actionScene->table->Update)) {
        game->scenes->actionScene->table->Update(game);
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
        game->scenes->actionScene->table->DrawWorld(game);
    EndTextureMode();

    BeginDrawing();
        ClearBackground(BLACK);
        Ivy_VirtualScreen_Draw(game->viewport);
        game->scenes->actionScene->table->DrawUI(game);
        DrawFPS(10, 10);
    EndDrawing();
}

void Ivy_Game_Destroy(IvyGame *game)
{
    IVY_ASSERT(game, "[Game] Instance is NULL!");

    const IvyScene *s = game->scenes->actionScene;
    if (IVY_LIKELY(s && s->table->Unload)) s->table->Unload(game->scenes);

    Ivy_Gfx_UnloadFont(&game->fonts[IVY_FONT_PRIMARY]);
    Ivy_Gfx_UnloadFont(&game->fonts[IVY_FONT_SECONDARY]);

    rlUnloadTexture(game->cursors[IVY_CURSOR_PRIMARY].id);
    rlUnloadTexture(game->cursors[IVY_CURSOR_SECONDARY].id);

    Ivy_VirtualScreen_Unload(game->viewport);

    Ivy_Arena_LinearDestroy(&game->arenas[IVY_ARENA_LOCALE]);
    Ivy_Arena_LinearDestroy(&game->arenas[IVY_ARENA_MAIN]);
}
