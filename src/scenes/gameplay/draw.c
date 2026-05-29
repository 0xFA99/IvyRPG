#include "ivy/core/game.h"
#include "ivy/entities/player.h"
#include "ivy/graphics/collusion.h"
#include "ivy/graphics/gfx.h"
#include "ivy/graphics/tilemap.h"
#include "ivy/scenes/gameplay.h"
#include "ivy/systems/scene_manager.h"

enum {
    POPUP_WIDTH  = 200,
    POPUP_HEIGHT = 160
};

void Ivy_Scene_GameplayDrawWorld(IvyGame *game)
{
    const IvySceneGameplayData *gameplayData = game->scenes->actionScene->data;

    BeginMode2D(gameplayData->camera.view);
#ifdef IVY_DEBUG
    Ivy_Tilemap_Render(gameplayData->tilemap);
    Ivy_Collusion_Draw(gameplayData->collusionMap);
#else
    Ivy_Collusion_Draw(gameplayData->collusionMap);
    Ivy_Tilemap_Render(gameplayData->tilemap);
#endif
    Ivy_Player_Render(gameplayData->player);
    EndMode2D();
}

void Ivy_Scene_GameplayRebuildTextures(IvyGame *game)
{
    const IvySceneGameplayData *gameplayData = game->scenes->actionScene->data;
    Ivy_Player_BakeAtlas(gameplayData->player, game->assets, &gameplayData->itemManager);
}

static void DrawMenuBackground(const IvySceneGameplayData *restrict gameplayData, const IvyVirtualScreen *restrict viewport)
{
    // background dimmer
    DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), (Color){ 0, 0, 0, 180 });

    // main menu
    const float bgWidth = (float)gameplayData->background.width;
    const float bgHeight = (float)gameplayData->background.height;

    const float bgVirtualX = (VIRTUAL_WIDTH - bgWidth) * 0.5f;
    const float bgVirtualY = (VIRTUAL_HEIGHT - bgHeight) * 0.5f;
    const Vector2 bgScreenPos = Ivy_Gfx_GetScreenPos(viewport, (Vector2){ bgVirtualX, bgVirtualY });

    const float scale = viewport->scale;

    DrawTexturePro(
        gameplayData->background,
        (Rectangle){ 0, 0, bgWidth, bgHeight },
        (Rectangle){ bgScreenPos.x, bgScreenPos.y, bgWidth * scale, bgHeight * scale },
        (Vector2){ 0 }, 0.0f, WHITE
    );
}

static void DrawMenuSelectionCursor(const IvyGame *restrict game, const IvyVirtualScreen *restrict viewport, const float textVirtualX, const float itemVirtualY)
{
    const Texture2D *cursor = &game->cursors[IVY_CURSOR_SECONDARY];
    const float cursorScale = viewport->scale * 0.5f;

    const float cursorPaddingX = 4.0f;
    const float cursorVirtualX = textVirtualX - ((float)cursor->width * 0.5f) - cursorPaddingX;
    const float cursorVirtualY = itemVirtualY + (14.0f - ((float)cursor->height * 0.5f)) * 0.5f;

    const Vector2 cursorScreenPos = Ivy_Gfx_GetScreenPos(viewport, (Vector2){ cursorVirtualX, cursorVirtualY });

    DrawTexturePro(
        *cursor,
        (Rectangle){ 0, 0, (float)cursor->width, (float)cursor->height },
        (Rectangle){ cursorScreenPos.x, cursorScreenPos.y, (float)cursor->width * cursorScale, (float)cursor->height * cursorScale },
        (Vector2){ 0 }, 0.0f, WHITE
    );
}

static void DrawMenuItems(const IvyGame *restrict game, const IvySceneGameplayData *restrict gameplayData, const IvyVirtualScreen *restrict viewport)
{
    const float menuStartY   = VIRTUAL_HEIGHT * 0.1f;
    const float itemSpacing  = 16.0f;
    const float textVirtualX = 32.0f;

    for (u32 i = 0; i < GAMEPLAY_MENU_SIZE; i++)
    {
        const float itemVirtualY = (float)i * itemSpacing + menuStartY;
        const Vector2 textScreenPos = Ivy_Gfx_GetScreenPos(viewport, (Vector2){ textVirtualX, itemVirtualY });

        const bool isSelected = (i == (u32)gameplayData->menu.selected);
        const Color textColor = isSelected ? WHITE : GRAY;

        if (isSelected) {
            DrawMenuSelectionCursor(game, viewport, textVirtualX, itemVirtualY);
        }

        Ivy_Gfx_DrawLocaleText(
            game->fonts[IVY_FONT_PRIMARY],
            gameplayData->menu.menuStrings[i],
            gameplayData->menu.menuLengths[i],
            textScreenPos,
            14.0f * viewport->scale,
            1,
            textColor
        );
    }
}

static void DrawInventoryLayout(const IvySceneGameplayData *restrict gameplayData, const IvyVirtualScreen *restrict viewport)
{
    const float backgroundWidth = gameplayData->inventoryUI.background.width;
    const float backgroundHeight = gameplayData->inventoryUI.background.height - 28;

    const float bgVirtualX = (VIRTUAL_WIDTH - backgroundWidth) * 0.5f;
    const float bgVirtualY = (VIRTUAL_HEIGHT - backgroundHeight) * 0.5f;
    const Vector2 backgroundPos = Ivy_Gfx_GetScreenPos(viewport, (Vector2){ bgVirtualX, bgVirtualY });

    const float scale = viewport->scale;

    DrawTexturePro(
        gameplayData->inventoryUI.background,
        (Rectangle) { 0, 0, backgroundWidth, backgroundHeight },
        (Rectangle) { backgroundPos.x, backgroundPos.y, backgroundWidth * scale, backgroundHeight * scale },
        (Vector2) {0},
        0.0f,
        WHITE
    );
}

// TODO: Test Template slots coordinate.
static void DrawSlotTemplate(const IvyGame *restrict game, const IvyVirtualScreen *restrict viewport)
{
    static const Vector2 slotPositions[13] = {
        { 215.0f, 30.0f },

        { 173.0f, 62.0f },
        { 215.0f, 61.0f },
        { 258.0f, 62.0f },

        { 168.0f, 94.0f },
        { 215.0f, 99.0f },
        { 262.0f, 94.0f },

        { 167.0f, 133.0f },
        { 215.0f, 137.0f },
        { 263.0f, 133.0f },

        { 166.0f, 169.0f },
        { 215.0f, 172.0f },
        { 264.0f, 169.0f }
    };

    const float slotSize = 24.0f;

    for (int i = 0; i < 13; i++)
    {
        const Vector2 screenPos = Ivy_Gfx_GetScreenPos(viewport, slotPositions[i]);

        const Rectangle slotRect = {
            .x      = screenPos.x,
            .y      = screenPos.y,
            .width  = slotSize * viewport->scale,
            .height = slotSize * viewport->scale
        };

        // Draw slot background
        DrawRectangleRec(slotRect, (Color){ 60, 60, 60, 200 });

        // Draw slot border
        DrawRectangleLinesEx(slotRect, 2.0f, (Color){ 180, 180, 180, 255 });

        // Draw slot index number
        char indexText[4];
        sprintf(indexText, "%d", i);
        DrawTextEx(
            game->fonts[IVY_FONT_SECONDARY],
            indexText,
            (Vector2){ screenPos.x + 8.0f, screenPos.y + 4.0f },
            12.0f * viewport->scale,
            1.0f,
            (Color){ 200, 200, 200, 255 }
        );
    }
}

// TODO: Test template items list.
static void DrawItemListTemplate(const IvyGame *restrict game, const IvyVirtualScreen *viewport)
{
    const IvySceneGameplayData *gd = game->scenes->actionScene->data;
    const IvyInventory *inventory = Ivy_Player_GetInventory(gd->player);
    const IvyInventoryBag *bag = &inventory->bag;

    static const Vector2 itemPositions[12] = {
        // Row 1
        { 311.5f, 30.0f },   // Col 1
        { 461.5f, 30.0f },   // Col 2
        // Row 2
        { 311.5f, 58.0f },
        { 461.5f, 58.0f },
        // Row 3
        { 311.5f, 86.0f },
        { 461.5f, 86.0f },
        // R1.54
        { 311.5f, 114.0f },
        { 461.5f, 114.0f },
        // R1.55
        { 311.5f, 142.0f },
        { 461.5f, 142.0f },
        // R1.56
        { 311.5f, 170.0f },
        { 461.5f, 170.0f },
    };

    const float itemSlotWidth = 144.0f;
    const float itemSlotHeight = 28.0f;

    static int selectedIndex = 0;

    const Vector2 frameScreenPos = Ivy_Gfx_GetScreenPos(viewport, (Vector2){ 305.0f, 24.0f });
    const Rectangle frameRect = {
        .x      = frameScreenPos.x,
        .y      = frameScreenPos.y,
        .width  = 307.0f * viewport->scale,
        .height = 181.0f * viewport->scale
    };
    DrawRectangleLinesEx(frameRect, 2.0f, (Color){ 100, 100, 100, 255 });

    const u8 targetCategory = IVY_ITEM_TYPE_EQUIPMENT;
    const u8 equipmentCount = bag->categoryCount[targetCategory];
    const u8 equipmentOffset = bag->categoryOffset[targetCategory];

    for (int i = 0; i < 12; i++)
    {
        const Vector2 screenPos = Ivy_Gfx_GetScreenPos(viewport, itemPositions[i]);

        const Rectangle slotRect = {
            .x      = screenPos.x,
            .y      = screenPos.y,
            .width  = itemSlotWidth * viewport->scale,
            .height = itemSlotHeight * viewport->scale
        };

        if (i == selectedIndex)
        {
            DrawTexturePro(gd->inventoryUI.background, (Rectangle) { 0, 360, 143.5f, 28 }, slotRect, (Vector2){0}, 0.0f, WHITE);
            DrawTexturePro(gd->inventoryUI.background, (Rectangle) { 143.5f, 360, 143.5f, 28 }, slotRect, (Vector2){0}, 0.0f, WHITE);
        }

        if (i < equipmentCount)
        {
            const u8 internalBagIndex = bag->categoryIndices[equipmentOffset + i];
            const u16 itemID = bag->slot[internalBagIndex].itemID;

            const char *itemName = Ivy_ItemManager_GetName(&gd->itemManager, itemID);
            const IvyItemVisual *visual = Ivy_ItemManager_GetVisual(&gd->itemManager, itemID);

            // draw icon
            DrawTexturePro(gd->iconsAtlas,
                visual->icon,
                (Rectangle) {
                    .x = screenPos.x + 8,
                    .y = screenPos.y + 2,
                    .width = 24 * viewport->scale,
                    .height = 24 * viewport->scale
                },
                (Vector2){0},
                0.0f,
                WHITE
            );

            // item name
            const float fontSize = 12.0f * viewport->scale;
            const Vector2 textSize = MeasureTextEx(game->fonts[IVY_FONT_SECONDARY], itemName, fontSize, 1.0f);

            DrawTextEx(
                game->fonts[IVY_FONT_SECONDARY],
                itemName,
                (Vector2){
                    screenPos.x + 72.0f,
                    screenPos.y + (slotRect.height - textSize.y) * 0.1f
                },
                fontSize,
                1.0f,
                (Color){ 220, 220, 220, 255 }
            );
        }
    }
}

void Ivy_Scene_GameplayDrawUI(IvyGame *game)
{
    const IvySceneGameplayData *gameplayData = game->scenes->actionScene->data;
    if (gameplayData->state != PAUSE_MENU_OPENED) return;

    const IvyVirtualScreen *viewport = game->viewport;

    DrawMenuBackground(gameplayData, viewport);

    // TODO: TEST
    if (gameplayData->menu.selected == 3)
    {
        DrawInventoryLayout(gameplayData, viewport);
        DrawSlotTemplate(game, viewport);
        DrawItemListTemplate(game, viewport);
    }

    DrawMenuItems(game, gameplayData, viewport);
}
