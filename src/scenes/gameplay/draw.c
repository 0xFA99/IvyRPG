#include "ivy/core/game.h"
#include "ivy/entities/player.h"
#include "ivy/graphics/collusion.h"
#include "ivy/graphics/gfx.h"
#include "ivy/graphics/tilemap.h"
#include "ivy/scenes/gameplay.h"
#include "ivy/systems/scene_manager.h"

enum {
    POPUP_WIDTH  = 200,
    POPUP_HEIGHT = 160,

    EQUIP_SLOT_COUNT = 13,
    ITEM_LIST_COUNT  = 12,

    EQUIP_SLOT_SIZE    = 24,
    ITEM_SLOT_WIDTH    = 144,
    ITEM_SLOT_HEIGHT   = 28,
    ITEM_ICON_SIZE     = 24,
    ITEM_ICON_OFFSET_X = 8,
    ITEM_ICON_OFFSET_Y = 2,
    ITEM_TEXT_OFFSET_X = 32,
    ITEM_FONT_SIZE     = 12
};

static const Rectangle EQUIP_SELECTED_SRC  = { 287.0f, 360.0f, 34.0f, 34.0f };

static const Rectangle ITEM_SELECTED_SRC_L = { 0.0f, 360.0f, 143.5f, 28.0f };
static const Rectangle ITEM_SELECTED_SRC_R = { 143.5f, 360.0f, 143.5f, 28.0f };

static const Vector2 EQUIP_SLOT_POSITIONS[EQUIP_SLOT_COUNT] = {
    { 215.0f, 30.0f },
    { 173.0f, 62.0f }, { 215.0f,  61.0f }, { 258.0f,  62.0f },
    { 168.0f, 94.0f }, { 215.0f,  99.0f }, { 262.0f,  94.0f },
    { 167.0f, 133.0f }, { 215.0f, 137.0f }, { 263.0f, 133.0f },
    { 166.0f, 169.0f }, { 215.0f, 172.0f }, { 264.0f, 169.0f },
};

static const Vector2 ITEM_LIST_POSITIONS[ITEM_LIST_COUNT] = {
    { 311.5f,  30.0f }, { 461.5f,  30.0f },
    { 311.5f,  58.0f }, { 461.5f,  58.0f },
    { 311.5f,  86.0f }, { 461.5f,  86.0f },
    { 311.5f, 114.0f }, { 461.5f, 114.0f },
    { 311.5f, 142.0f }, { 461.5f, 142.0f },
    { 311.5f, 170.0f }, { 461.5f, 170.0f },
};

IVY_INLINE void DrawAtlasRegion(const Texture2D *atlas, const Rectangle src, const Rectangle dst)
{
    DrawTexturePro(*atlas, src, dst, (Vector2){ 0 }, 0.0f, WHITE);
}

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
    const IvySceneGameplayData *gd = game->scenes->actionScene->data;
    Ivy_Player_BakeAtlas(gd->player, game->assets, &gd->itemManager);
}

static void DrawMenuBackground(const IvySceneGameplayData *restrict gd, const IvyVirtualScreen *restrict viewport)
{
    DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), (Color){ 0, 0, 0, 180 });

    const float bgW = (float)gd->background.width;
    const float bgH = (float)gd->background.height;
    const float scale = viewport->scale;

    const Vector2 pos = Ivy_Gfx_GetScreenPos(viewport, (Vector2){
        (VIRTUAL_WIDTH  - bgW) * 0.5f,
        (VIRTUAL_HEIGHT - bgH) * 0.5f
    });

    DrawAtlasRegion(
        &gd->background,
        (Rectangle){ 0, 0, bgW, bgH },
        (Rectangle){ pos.x, pos.y, bgW * scale, bgH * scale }
    );
}

static void DrawMenuSelectionCursor(const IvyGame *restrict game, const IvyVirtualScreen *restrict viewport,
                                    const float textVirtualX, const float itemVirtualY)
{
    const Texture2D *cursor = &game->cursors[IVY_CURSOR_SECONDARY];
    const float cursorScale = viewport->scale * 0.5f;

    const Vector2 pos = Ivy_Gfx_GetScreenPos(viewport, (Vector2){
        textVirtualX - ((float)cursor->width * 0.5f) - 4.0f,
        itemVirtualY + (14.0f - ((float)cursor->height * 0.5f)) * 0.5f
    });

    DrawAtlasRegion(
        cursor,
        (Rectangle){ 0, 0, (float)cursor->width, (float)cursor->height },
        (Rectangle){ pos.x, pos.y, (float)cursor->width  * cursorScale, (float)cursor->height * cursorScale }
    );
}

static void DrawMenuItems(const IvyGame *restrict game, const IvySceneGameplayData *restrict gameplayData,
                          const IvyVirtualScreen *restrict viewport)
{
    const float menuStartY  = VIRTUAL_HEIGHT * 0.1f;
    const float itemSpacing = 16.0f;
    const float textX       = 32.0f;

    for (u32 i = 0; i < GAMEPLAY_MENU_SIZE; i++)
    {
        const float itemY = (float)i * itemSpacing + menuStartY;
        const bool isSelected = ((u32)gameplayData->menu.selected == i);

        if (isSelected) DrawMenuSelectionCursor(game, viewport, textX, itemY);

        Ivy_Gfx_DrawLocaleText(
            game->fonts[IVY_FONT_PRIMARY],
            gameplayData->menu.menuStrings[i],
            gameplayData->menu.menuLengths[i],
            Ivy_Gfx_GetScreenPos(viewport, (Vector2){ textX, itemY }),
            14.0f * viewport->scale,
            1,
            isSelected ? WHITE : GRAY
        );
    }
}

static void DrawInventoryLayout(const IvySceneGameplayData *restrict gameplayData, const IvyVirtualScreen *restrict viewport)
{
    const float bgW = (float)gameplayData->inventoryUI.background.width;
    const float bgH = (float)gameplayData->inventoryUI.background.height - 34.0f;
    const float scale = viewport->scale;

    const Vector2 pos = Ivy_Gfx_GetScreenPos(viewport, (Vector2){
        (VIRTUAL_WIDTH  - bgW) * 0.5f,
        (VIRTUAL_HEIGHT - bgH) * 0.5f
    });

    DrawAtlasRegion(
        &gameplayData->inventoryUI.background,
        (Rectangle){ 0, 0, bgW, bgH },
        (Rectangle){ pos.x, pos.y, bgW * scale, bgH * scale }
    );
}

static void DrawEquipSlots(const IvyGame *restrict game, const IvyVirtualScreen *restrict viewport)
{
    const IvySceneGameplayData *gd = game->scenes->actionScene->data;
    const IvyInventoryUI       *ui = &gd->inventoryUI;
    const float scale = viewport->scale;

    if (ui->focus != INVENTORY_FOCUS_EQUIP_SLOTS) return;

    const float highlightSize = EQUIP_SLOT_SIZE + 12.0f;

    for (int i = 0; i < EQUIP_SLOT_COUNT; i++)
    {
        if (i != ui->selectedEquip) continue;

        const Vector2 screenPos = Ivy_Gfx_GetScreenPos(viewport, EQUIP_SLOT_POSITIONS[i]);
        DrawAtlasRegion(
            &gd->inventoryUI.background,
            EQUIP_SELECTED_SRC,
            (Rectangle){
                screenPos.x - 6.0f * scale,
                screenPos.y - 6.0f * scale,
                highlightSize * scale,
                highlightSize * scale
            }
        );
        break;
    }
}

static void DrawItemList(const IvyGame *restrict game, const IvyVirtualScreen *restrict viewport)
{
    const IvySceneGameplayData *gameplayData = game->scenes->actionScene->data;
    const IvyInventoryUI *ui = &gameplayData->inventoryUI;
    const IvyInventoryBag *bag = &Ivy_Player_GetInventory(gameplayData->player)->bag;
    const float scale = viewport->scale;

    const u8 catOffset = bag->categoryOffset[IVY_ITEM_TYPE_EQUIPMENT];
    const u8 catCount  = bag->categoryCount [IVY_ITEM_TYPE_EQUIPMENT];

    for (u8 i = 0; i < catCount; i++)
    {
        const Vector2  screenPos = Ivy_Gfx_GetScreenPos(viewport, ITEM_LIST_POSITIONS[i]);
        const Rectangle slotDst = { screenPos.x, screenPos.y, ITEM_SLOT_WIDTH * scale, ITEM_SLOT_HEIGHT * scale };

        // draw highlight
        if (ui->focus == INVENTORY_FOCUS_ITEM_LIST && i == ui->selectedSlot) {
            DrawAtlasRegion(&gameplayData->inventoryUI.background, ITEM_SELECTED_SRC_L, slotDst);
            DrawAtlasRegion(&gameplayData->inventoryUI.background, ITEM_SELECTED_SRC_R, slotDst);
        }

        const u8 bagIndex = bag->categoryIndices[catOffset + i];
        const u16 itemID  = bag->slot[bagIndex].itemID;

        const IvyItemVisual *visual = Ivy_ItemManager_GetVisual(&gameplayData->itemManager, itemID);
        const char *itemName = Ivy_ItemManager_GetName(&gameplayData->itemManager, itemID);

        DrawAtlasRegion(
            &gameplayData->iconsAtlas,
            visual->icon,
            (Rectangle){
                screenPos.x + ITEM_ICON_OFFSET_X * scale,
                screenPos.y + ITEM_ICON_OFFSET_Y * scale,
                ITEM_ICON_SIZE * scale,
                ITEM_ICON_SIZE * scale
            }
        );

        const float fontSize   = ITEM_FONT_SIZE * scale;
        const float textPadX   = ITEM_TEXT_OFFSET_X * scale;
        const Vector2 textSize = MeasureTextEx(game->fonts[IVY_FONT_SECONDARY], itemName, fontSize, 1.0f);

        DrawTextEx(
            game->fonts[IVY_FONT_SECONDARY],
            itemName,
            (Vector2){ screenPos.x + textPadX, screenPos.y + (slotDst.height - textSize.y) * 0.15f },
            fontSize, 1.0f, WHITE
        );

        char countStr[16];
        snprintf(countStr, sizeof(countStr), "Stack: %d", bag->slot[bagIndex].quantity);

        DrawTextEx(
            game->fonts[IVY_FONT_SECONDARY],
            countStr,
            (Vector2){ screenPos.x + textPadX, screenPos.y + (slotDst.height - textSize.y) * 0.75f },
            fontSize, 1.0f, GRAY
        );
    }
}

void Ivy_Scene_GameplayDrawUI(IvyGame *game)
{
    const IvySceneGameplayData *gd = game->scenes->actionScene->data;
    if (gd->state == PAUSE_MENU_CLOSED) return;

    const IvyVirtualScreen *viewport = game->viewport;

    if (gd->state == PAUSE_MENU_INVENTORY || gd->menu.selected == 3)
    {
        DrawMenuBackground(gd, viewport);
        DrawMenuItems(game, gd, viewport);
        DrawInventoryLayout(gd, viewport);

        if (gd->inventoryUI.selectedSlot != INVENTORY_SLOT_NONE) {
            DrawEquipSlots(game, viewport);
        }

        DrawItemList(game, viewport);
        return;
    }

    if (gd->state == PAUSE_MENU_OPENED) {
        DrawMenuBackground(gd, viewport);
        DrawMenuItems(game, gd, viewport);
    }
}
