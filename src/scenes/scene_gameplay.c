#include "ivy/game.h"
#include "ivy/scenes.h"
#include "ivy/utils.h"
#include "ivy/player/player.h"

#include <assert.h>
#include <stdlib.h>

static bool showDebugCollision = false;

void SceneGameplayInit(Scene *s)
{
    SceneGameplayData *gd = malloc(sizeof(SceneGameplayData));
    assert(gd && "[ERROR] Failed to allocate memory for SceneGameplayData!");

    // START - TEST TILEMAP TIME
    double startTime = GetTime();
    gd->tilemap     = LoadTilemapById(1);
    printf("Map loaded in: %f ms\n", (GetTime() - startTime) * 1000.0);
    // END - TEST TILEMAP TIME

    gd->collision   = InitCollisionAllLayers(gd->tilemap);
    gd->itemManager = CreateItemManager();

    LoadItemsFromFile(gd->itemManager, "assets/items/equipments/twin_braids.bin");
    LoadItemsFromFile(gd->itemManager, "assets/items/equipments/red_cape.bin");
    LoadItemsFromFile(gd->itemManager, "assets/items/equipments/civilian_shirt.bin");
    LoadItemsFromFile(gd->itemManager, "assets/items/equipments/civilian_bot.bin");
    LoadItemsFromFile(gd->itemManager, "assets/items/equipments/leather_bag.bin");
    LoadItemsFromFile(gd->itemManager, "assets/items/equipments/black_gothic_shirt.bin");
    LoadItemsFromFile(gd->itemManager, "assets/items/equipments/black_gothic_skirt.bin");
    LoadItemsFromFile(gd->itemManager, "assets/items/equipments/red_gothic_shirt.bin");
    LoadItemsFromFile(gd->itemManager, "assets/items/equipments/red_gothic_skirt.bin");
    LoadItemsFromFile(gd->itemManager, "assets/items/equipments/maid_shirt.bin");
    LoadItemsFromFile(gd->itemManager, "assets/items/equipments/maid_skirt.bin");
    LoadItemsFromFile(gd->itemManager, "assets/items/equipments/maid_bando.bin");
    LoadItemsFromFile(gd->itemManager, "assets/items/equipments/black_gothic_bando.bin");
    LoadItemsFromFile(gd->itemManager, "assets/items/equipments/red_gothic_bando.bin");
    LoadItemsFromFile(gd->itemManager, "assets/items/equipments/wooden_shield.bin");

    gd->player = InitPlayer(
        gd->tilemap->header.spawnPointX,
        gd->tilemap->header.spawnPointY,
        gd->tilemap->header.tileWidth
    );

    for (u32 i = 0; i < gd->itemManager->count; i++)
        InventoryAdd(gd->player->inventory, gd->itemManager->items[i]);

    gd->gameCamera = InitGameCamera(VIRTUAL_WIDTH, VIRTUAL_HEIGHT);
    gd->gameCamera.camera2D.target = gd->player->movement.position;

    gd->inventoryUI = CreateInventoryUI();

    s->data.gameplay = gd;

}

void SceneGameplayUpdate(Game *game)
{
    SceneGameplayData *gd = game->sceneManager.activeScene.data.gameplay;

    if (IsKeyPressed(KEY_I)) {
        if (!gd->inventoryUI.isOpen) gd->inventoryUI.pendingOpen = true;
        else InventoryUIClose(&gd->inventoryUI);
    }

    if (gd->inventoryUI.isOpen) {
        if (InventoryUIUpdate(&gd->inventoryUI, gd->player))
            InventoryUIClose(&gd->inventoryUI);

        return;
    }

    if (IsKeyPressed(KEY_ESCAPE)) {
        game->sceneManager.activeScene.type = SCENE_TITLE;
        game->sceneManager.sceneChanged     = true;

        return;
    }

    if (IsKeyPressed(KEY_F1)) showDebugCollision = !showDebugCollision;

    const float ft = GetFrameTime();
    UpdatePlayer(gd->player, ft, gd->collision, gd->tilemap->header.tileWidth);
    UpdateGameCamera(&gd->gameCamera, gd->player, gd->tilemap, ft);
}

void SceneGameplayDrawWorld(Game *game)
{
    const SceneGameplayData *gd = game->sceneManager.activeScene.data.gameplay;

    if (gd->inventoryUI.isOpen) return;

    BeginMode2D(gd->gameCamera.camera2D);
        DrawTilemapFromCanva(gd->tilemap);
        DrawPlayer(gd->player, &game->viewport);

        if (showDebugCollision) {
            DrawPlayerDebug(gd->player);
            for (u32 i = 0; i < gd->collision->rectCount; i++) {
                DrawRectangleLinesEx(gd->collision->rect[i], 1.0f, (Color){ 255, 165, 0, 180 });
            }
        }
    EndMode2D();
}

void SceneGameplayRebuildTextures(Game *game)
{
    SceneGameplayData *gd = game->sceneManager.activeScene.data.gameplay;
    Player *player        = gd->player;

    RebuildPortrait(&player->portrait, &player->graphics, &player->equipment);

    if (gd->inventoryUI.pendingOpen) {
        InventoryUIOpen(&gd->inventoryUI, &game->viewport);
    }
}

void SceneGameplayDrawUI(Game *game)
{
    const SceneGameplayData *gd = game->sceneManager.activeScene.data.gameplay;

    DrawPortraitHUD(&gd->player->portrait, &game->viewport);

    if (gd->inventoryUI.isOpen) {
        InventoryUIDraw(
            &gd->inventoryUI,
            gd->player,
            &game->viewport,
            &game->fonts[IVY_FONT_PRIMARY]
        );

        return;
    }

    if (showDebugCollision) {
        const Vector2 pos = GetScreenPos(&game->viewport, (Vector2){ 10.0f, 10.0f });
        DrawTextEx(game->fonts[IVY_FONT_PRIMARY], "DEBUG: ON (F1)", pos, 14.0f * game->viewport.scale, 1, GREEN);
    }

    {
        const Vector2 pos = GetScreenPos(&game->viewport, (Vector2){ 10.0f, VIRTUAL_HEIGHT - 14.0f });

        DrawTextEx(game->fonts[IVY_FONT_PRIMARY], "[I] Inventory",
                   pos, 9.0f * game->viewport.scale, 1,
                   (Color){ 200, 200, 200, 180 });
    }
}

void SceneGameplayUnload(Scene *s)
{
    if (!s->data.gameplay) return;

    SceneGameplayData *gd = s->data.gameplay;
    DestroyInventoryUI(&gd->inventoryUI);
    DestroyCollision(gd->collision);
    UnloadTilemap(gd->tilemap);
    DestroyPlayer(gd->player);
    DestroyItemManager(gd->itemManager);
    free(gd);

    s->data.gameplay = NULL;
}