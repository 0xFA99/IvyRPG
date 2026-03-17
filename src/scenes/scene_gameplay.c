#include "ivy/game.h"
#include "ivy/scenes.h"
#include "ivy/utils.h"
#include "ivy/player/player.h"
#include "ivy/npc/npc.h"

#include <stdlib.h>

static bool showDebugCollision = false;

void SceneGameplayInit(Scene *s)
{
    SceneGameplayData *gd = malloc(sizeof(SceneGameplayData));
    IVY_ASSERT(gd, "[SceneGameplayData] Failed to allocate memory!");

    // START - TEST TILEMAP TIME
    const double startTime = GetTime();
    gd->tilemap = LoadTilemapById(1);
    TraceLog(LOG_INFO, "Map loaded in: %f ms\n", (GetTime() - startTime) * 1000.0);
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

    /* Build LUT setelah semua item selesai di-load */
    BuildItemTable(gd->itemManager);

    gd->player = InitPlayer(
        (Vector2){ (float)gd->tilemap->header.spawnPointX, (float)gd->tilemap->header.spawnPointY });

    for (u32 i = 0; i < gd->itemManager->count; i++)
        InventoryAdd(gd->player->inventory, gd->itemManager->items[i]);

    gd->gameCamera = InitGameCamera(VIRTUAL_WIDTH, VIRTUAL_HEIGHT);
    gd->gameCamera.camera2D.target = gd->player->movement.position;

    gd->inventoryUI = CreateInventoryUI();

    gd->npcManager = LoadNPCManager();
    AddRandomNPC(gd->npcManager, gd->itemManager->table, (Vector2){ 10, 13 });
    AddRandomNPC(gd->npcManager, gd->itemManager->table, (Vector2){ 12, 13 });

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
    SceneGameplayData *gd = game->sceneManager.activeScene.data.gameplay;

    if (gd->inventoryUI.isOpen) return;

    BeginMode2D(gd->gameCamera.camera2D);
    DrawTilemapFromCanva(gd->tilemap);

    typedef struct {
        float  y;
        bool   isPlayer;
        u32    npcIndex;
    } DrawEntry;

    const u32 totalEntities = 1 + gd->npcManager->npcCount;
    DrawEntry entries[NPC_SIZE_CAP + 1];

    entries[0].y         = gd->player->movement.position.y;
    entries[0].isPlayer  = true;
    entries[0].npcIndex  = 0;

    for (u32 i = 0; i < gd->npcManager->npcCount; i++) {
        entries[1 + i].y        = gd->npcManager->npc[i].movement.position.y;
        entries[1 + i].isPlayer = false;
        entries[1 + i].npcIndex = i;
    }

    for (u32 i = 1; i < totalEntities; i++) {
        DrawEntry key = entries[i];
        int j = (int)i - 1;
        while (j >= 0 && entries[j].y > key.y) {
            entries[j + 1] = entries[j];
            j--;
        }
        entries[j + 1] = key;
    }

    for (u32 i = 0; i < totalEntities; i++) {
        if (entries[i].isPlayer)
            DrawPlayer(gd->player, &game->viewport);
        else
            DrawNPC(&gd->npcManager->npc[entries[i].npcIndex]);
    }

    if (showDebugCollision) {
        DrawPlayerDebug(gd->player);
        for (u32 i = 0; i < gd->collision->rectCount; i++)
            DrawRectangleLinesEx(gd->collision->rect[i], 1.0f, (Color){ 255, 165, 0, 180 });
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
    UnloadNPCManager(gd->npcManager);
    free(gd);

    s->data.gameplay = NULL;
}