#include "ivy/scenes/gameplay.h"

#include "ivy/core/types.h"
#include "ivy/core/game.h"
#include "ivy/core/keybind.h"
#include "ivy/arena/linear.h"
#include "ivy/audio/buffer.h"
#include "ivy/audio/ogg.h"
#include "ivy/systems/scene_manager.h"
#include "ivy/graphics/tilemap.h"
#include "ivy/entities/player.h"
#include "ivy/graphics/collusion.h"
#include "ivy/graphics/gfx.h"
#include "ivy/systems/locale_manager.h"
#include "ivy/utils/file_ids.h"

enum {
    POPUP_WIDTH = 200,
    POPUP_HEIGHT = 160
};

const u16 MENU_GAMEPLAY_PAUSE[] = { 16, 17, 18, 19 };

void Ivy_Scene_GameplayInit(IvyGame *game)
{
    IVY_ASSERT(game != NULL, "[Scene Gameplay] Arena not found!");

    IvySceneGameplayData *gd = Ivy_Arena_LinearAllocZero(&game->arenas[IVY_ARENA_MAIN], sizeof(IvySceneGameplayData));
    IVY_ASSERT(gd != NULL, "[Scene Gameplay] Failed to allocate SceneGameplayData!");

    gd->tilemap = Ivy_Tilemap_LoadMap(game->assets, &game->arenas[IVY_ARENA_MAIN], ASSET_MAPS_MAP_1_METADATA_BIN, ASSET_MAPS_MAP_1_VERTEX_BIN);
    gd->collusionMap = Ivy_Collusion_Load(&game->arenas[IVY_ARENA_MAIN], game->assets);
    gd->player = Ivy_Player_Init(&game->arenas[IVY_ARENA_MAIN], game->assets, (Vector2){ 10.0f, 16.0f });
    gd->camera = Ivy_Camera_Init();

    gd->itemManager = Ivy_ItemManager_Init(&game->arenas[IVY_ARENA_MAIN], game->assets);

    gd->state = GAMEPLAY_CLOSE_MENU;

    Ivy_Player_EquipItem(gd->player, game->assets, &gd->itemManager);

    // locales
    for (u32 i = 0; i < 4; i++) {
        gd->menu.menuStrings[i] = IVY_TR(game->locale, (IvyLocaleKey)MENU_GAMEPLAY_PAUSE[i]);
        gd->menu.menuLengths[i] = IVY_TR_LEN(game->locale, (IvyLocaleKey)MENU_GAMEPLAY_PAUSE[i]);
    }

    gd->menu.selected = 0;
    gd->menu.sound = Ivy_Audio_LoadSoundWav(&game->arenas[IVY_ARENA_MAIN], game->assets, ASSET_AUDIO_CURSOR_WAV);

    // arena required 192560 bytes (hasil debug :P)
    gd->music = Ivy_Audio_LoadMusicOGG(&game->arenas[IVY_ARENA_MAIN], game->assets, ASSET_MUSIC_POINT_AND_CLICK_OGG, 192560);

    Ivy_Audio_PlayAudioBuffer(gd->music.stream.buffer);

    game->scenes->actionScene->data = gd;
}

void Ivy_Scene_GameplayUpdate(IvyGame *g)
{
    IvySceneGameplayData *gd = g->scenes->actionScene->data;

    // UpdateMusicStream(gd->music);
    Ivy_Audio_UpdateMusicOGG(&gd->music);

    if (IsKeyPressed(KEY_I) && gd->state != GAMEPLAY_OPEN_MENU) {
        gd->state = GAMEPLAY_OPEN_MENU;
        gd->menu.selected = 0;
    }
    else if (IsKeyPressed(KEY_ESCAPE) && gd->state == GAMEPLAY_OPEN_MENU) {
        gd->state = GAMEPLAY_CLOSE_MENU;
    }

    if (gd->state == GAMEPLAY_OPEN_MENU) {
        const int direction = IsKeyPressed(g->keybind[IVY_KEY_DOWN].currentKey)
                            - IsKeyPressed(g->keybind[IVY_KEY_UP].currentKey);

        if (IsKeyPressed(g->keybind[IVY_KEY_CONFIRM].currentKey)) {
            switch (gd->menu.selected)
            {
                case 0: // RESUME
                    gd->state = GAMEPLAY_CLOSE_MENU;
                    break;

                // TODO: Save & Load
                // case 1: break;
                // case 2: break;

                case 3: // BACK TO TITLE
                    Ivy_SceneManager_Transition(g, SCENE_TITLE);
                    break;

                default: break;
            }
        }

        if (direction) {
            gd->menu.selected = (gd->menu.selected + direction + GAMEPLAY_MENU_SIZE) % GAMEPLAY_MENU_SIZE;
            Ivy_Audio_PlayAudioBuffer(gd->menu.sound.data.stream.buffer);
        }

        return;
    }

    const float frameTime = GetFrameTime();
    Ivy_Player_Update(gd->player, frameTime, gd->collusionMap);
    Ivy_Camera_Update(&gd->camera, Ivy_Player_GetPosition(gd->player));
}

void Ivy_Scene_GameplayDrawWorld(IvyGame *g)
{
    IvySceneGameplayData *gd = g->scenes->actionScene->data;

    BeginMode2D(gd->camera.view);
#ifdef IVY_DEBUG
    Ivy_Tilemap_Render(gd->tilemap);
    Ivy_Collusion_Draw(gd->collusionMap);
#else
    Ivy_Collusion_Draw(gd->collusionMap);
    Ivy_Tilemap_Render(gd->tilemap);
#endif
    Ivy_Player_Render(gd->player);
    EndMode2D();
}

void Ivy_Scene_GameplayRebuildTextures(IvyGame *g)
{
    IvySceneGameplayData *gd = g->scenes->actionScene->data;
    Ivy_Player_BakeAtlas(gd->player);
}

void Ivy_Scene_GameplayDrawUI(IvyGame *g)
{
    const IvySceneGameplayData *gd = g->scenes->actionScene->data;
    const IvyVirtualScreen *viewport = g->viewport;

    if (gd->state != GAMEPLAY_CLOSE_MENU)
    {
        DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), (Color){ 0, 0, 0, 180 });

        const float x = (VIRTUAL_WIDTH - POPUP_WIDTH) * 0.5f;
        const float y = (VIRTUAL_HEIGHT - POPUP_HEIGHT) * 0.5f;
        const Vector2 popupPos = Ivy_Gfx_GetScreenPos(viewport, (Vector2){ x, y });

        DrawRectangleRec(
            (Rectangle) {
                .x      = popupPos.x,
                .y      = popupPos.y,
                .width  = POPUP_WIDTH * viewport->scale,
                .height = POPUP_HEIGHT * viewport->scale
            },
            (Color){ 30, 30, 45, 200 }
        );

        const float menuY = VIRTUAL_HEIGHT * 0.5f;

        for (u32 i = 0; i < 4; i++)
        {
            const Vector2 textPos = Ivy_Gfx_GetScreenPos(viewport, (Vector2){ x + 16, menuY + ((float)i * 16) });

            Ivy_Gfx_DrawLocaleText(
                g->fonts[IVY_FONT_PRIMARY],
                gd->menu.menuStrings[i],
                gd->menu.menuLengths[i],
                textPos,
                14 * viewport->scale,
                1,
                i == (u32)gd->menu.selected ? WHITE : GRAY
            );
        }
    }
}

void Ivy_Scene_GameplayUnload(IvySceneManager *sm)
{
    if (IVY_UNLIKELY(!sm->actionScene || !sm->actionScene->data)) return;

    const IvySceneGameplayData *gd = sm->actionScene->data;

    Ivy_Tilemap_Unload(gd->tilemap);
    Ivy_Collusion_Unload(gd->collusionMap);
    Ivy_Player_Unload(gd->player);
}
