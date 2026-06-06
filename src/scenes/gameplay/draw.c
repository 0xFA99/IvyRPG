#include "ivy/arena/linear.h"
#include "ivy/audio/buffer.h"
#include "ivy/core/game.h"
#include "ivy/core/types.h"
#include "ivy/core/virtual.h"
#include "ivy/entities/player.h"
#include "ivy/graphics/camera.h"
#include "ivy/graphics/collusion.h"
#include "ivy/graphics/gfx.h"
#include "ivy/graphics/tilemap.h"
#include "ivy/scenes/gameplay.h"
#include "ivy/scenes/types.h"
#include "ivy/systems/asset_manager.h"
#include "ivy/systems/inventory.h"
#include "ivy/systems/item_manager.h"
#include "ivy/systems/render_system.h"
#include "ivy/systems/scene_manager.h"
#include "ivy/systems/texture_manager.h"
#include "ivy/utils/file_ids.h"
#include "ivy/utils/forward.h"

#include <stdio.h>

#include "ivy/systems/object_manager.h"

enum {
    POPUP_WIDTH             = 200,
    POPUP_HEIGHT            = 160,

    EQUIP_SLOT_COUNT        = 13,
    ITEM_LIST_COUNT         = 12,

    EQUIP_SLOT_SIZE         = 24,
    ITEM_SLOT_WIDTH         = 144,
    ITEM_SLOT_HEIGHT        = 28,
    OPTIONS_ITEM_ICON_SIZE  = 24,
    ITEM_ICON_OFFSET_X      = 4,
    ITEM_ICON_OFFSET_Y      = 2,
    ITEM_TEXT_OFFSET_X      = 32,
    ITEM_FONT_SIZE          = 11
};

typedef struct {
    const IvyInventoryUI *ui;
    const IvyInventory   *inventory;
    const IvyItemManager *itemManager;
    Font font;
    float scale;
} IvyItemDrawCtx;

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

static const char *const SLOT_NAME[IVY_SLOT_MAX] = {
    "HEAD", "TOP EXT", "HAIR", "ACC", "TOP",
    "MID", "ACC-2", "M-ARM", "MID EXT", "S-ARM",
    "EXT1", "BOT", "EXT2"
};

IVY_INLINE void DrawAtlasRegion(const Texture2D *atlas, const Rectangle src, const Rectangle dst)
{
    DrawTexturePro(*atlas, src, dst, (Vector2){ 0 }, 0.0f, WHITE);
}

void Ivy_Scene_GameplayDrawWorld(IvyGame *game)
{
    const IvySceneGameplayData *gameplayData = game->sceneManager->actionScene->data;

    BeginMode2D(gameplayData->camera.view);
#ifdef IVY_DEBUG
    Ivy_Tilemap_Render(gameplayData->tilemap);
    Ivy_Collusion_Draw(gameplayData->collusionMap);
#else
    Ivy_Collusion_Draw(gameplayData->collusionMap);
    Ivy_Tilemap_Render(gameplayData->tilemap);
#endif

    Ivy_RenderSystem_SortAndDraw(game->objectManager);

    EndMode2D();
}

void Ivy_Scene_GameplayRebuildTextures(IvyGame *game)
{
    IvySceneGameplayData *gameplayData = game->sceneManager->actionScene->data;
    Ivy_Player_BakeAtlas(game, gameplayData);
}

static void DrawMenuBackground(const IvyTextureManager *restrict texManager, const IvyVirtualScreen *restrict viewport)
{
    DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), (Color){ 0, 0, 0, 255 });

    const Texture2D background = Ivy_TextureManager_Get(texManager, ASSET_TEXTURES_BACKGROUND_DDS);

    const float bgW = (float)background.width;
    const float bgH = (float)background.height;
    const float scale = viewport->scale;

    const Vector2 pos = Ivy_Gfx_GetScreenPos(viewport, (Vector2){
        (VIRTUAL_WIDTH  - bgW) * 0.5f,
        (VIRTUAL_HEIGHT - bgH) * 0.5f
    });

    DrawAtlasRegion(
        &background,
        (Rectangle){ 0, 0, bgW, bgH },
        (Rectangle){ pos.x, pos.y, bgW * scale, bgH * scale }
    );
}

static void DrawMenuSelectionCursor(const IvyGame *restrict game, const IvyVirtualScreen *restrict viewport,
                                    const float textVirtualX, const float itemVirtualY)
{
    const Texture2D cursorTex = Ivy_TextureManager_Get(game->textureManager, ASSET_TEXTURES_CURSOR_YELLOW_DDS);
    const float cursorScale = viewport->scale * 0.5f;

    const Vector2 pos = Ivy_Gfx_GetScreenPos(viewport, (Vector2){
        textVirtualX - ((float)cursorTex.width * 0.5f) - 4.0f,
        itemVirtualY + (14.0f - ((float)cursorTex.height * 0.5f)) * 0.5f
    });

    DrawAtlasRegion(
        &cursorTex,
        (Rectangle){ 0, 0, (float)cursorTex.width, (float)cursorTex.height },
        (Rectangle){ pos.x, pos.y, (float)cursorTex.width  * cursorScale, (float)cursorTex.height * cursorScale }
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

static void DrawEquippedItemIcons(IvyPlayer *restrict player, const IvySceneGameplayData *restrict gameplayData, const IvyTextureManager *restrict texManager, const IvyVirtualScreen *restrict viewport)
{
    const IvyInventory *inventory  = Ivy_Player_GetInventory(player);
    const float scale = viewport->scale;

    const Texture2D iconTex = Ivy_TextureManager_Get(texManager, ASSET_TEXTURES_ICONS_DDS);

    for (int i = 0; i < EQUIP_SLOT_COUNT; i++) {
        const u16 itemID = Ivy_Inventory_GetEquippedItemID(inventory, (u8)i);
        if (itemID == 0) continue;

        const IvyItemVisual *vis = Ivy_ItemManager_GetVisual(&gameplayData->itemManager, itemID);
        if (!vis) continue;

        const Vector2 screenPos = Ivy_Gfx_GetScreenPos(viewport, EQUIP_SLOT_POSITIONS[i]);

        DrawRectangleV(
            (Vector2){ screenPos.x, screenPos.y },
            (Vector2){ EQUIP_SLOT_SIZE * scale, EQUIP_SLOT_SIZE * scale },
            (Color){ 255, 255, 255, 30 }
        );

        DrawAtlasRegion(
            &iconTex,
            vis->icon,
            (Rectangle){
                screenPos.x,
                screenPos.y,
                EQUIP_SLOT_SIZE * scale,
                EQUIP_SLOT_SIZE * scale
            }
        );
    }
}

static void DrawEquipSlots(const IvyGame *restrict game, const IvyVirtualScreen *restrict viewport)
{
    const IvySceneGameplayData *gd = game->sceneManager->actionScene->data;
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

static void DrawItemList(const IvyItemDrawCtx *ctx, const IvyVirtualScreen *viewport, const Texture2D *iconTex)
{
    const IvyInventoryBag *bag = &ctx->inventory->bag;
    const u8 catOffset = bag->categoryOffset[IVY_ITEM_TYPE_EQUIPMENT];
    const u8 catCount  = bag->categoryCount [IVY_ITEM_TYPE_EQUIPMENT];

    const float fontSize     = ITEM_FONT_SIZE * ctx->scale;
    const float textPadX     = ITEM_TEXT_OFFSET_X * ctx->scale;
    const float textPadRight = 4.0f * ctx->scale;

    for (u8 i = 0; i < catCount; i++)
    {
        const Vector2 screenPos = Ivy_Gfx_GetScreenPos(viewport, ITEM_LIST_POSITIONS[i]);
        const Rectangle slotDst = { screenPos.x, screenPos.y, ITEM_SLOT_WIDTH * ctx->scale, ITEM_SLOT_HEIGHT * ctx->scale };

        // 1. Draw Highlight Background
        if (ctx->ui->focus == INVENTORY_FOCUS_ITEM_LIST && i == ctx->ui->selectedSlot) {
            DrawAtlasRegion(&ctx->ui->background, ITEM_SELECTED_SRC_L, slotDst);
            DrawAtlasRegion(&ctx->ui->background, ITEM_SELECTED_SRC_R, slotDst);
        }

        // Fetch Data Item
        const u8 bagIndex             = bag->categoryIndices[catOffset + i];
        const u16 itemID              = bag->slot[bagIndex].itemID;
        const IvyItemAttribute *attr  = Ivy_ItemManager_GetAttribute(ctx->itemManager, itemID);
        const IvyItemVisual *visual   = Ivy_ItemManager_GetVisual(ctx->itemManager, itemID);
        const char *itemName          = Ivy_ItemManager_GetName(ctx->itemManager, itemID);

        // 2. Draw Icon
        DrawAtlasRegion(iconTex, visual->icon, (Rectangle){
            screenPos.x + ITEM_ICON_OFFSET_X * ctx->scale,
            screenPos.y + ITEM_ICON_OFFSET_Y * ctx->scale,
            OPTIONS_ITEM_ICON_SIZE * ctx->scale,
            OPTIONS_ITEM_ICON_SIZE * ctx->scale
        });

        // Cek Status Equipped
        bool isEquipped = false;
        for (usize s = 0; s < IVY_SLOT_MAX; s++) {
            if (ctx->inventory->equipped.index[s] == bagIndex) {
                isEquipped = true;
                break;
            }
        }

        // 3. Draw Nama Item & Status [E]
        const Vector2 textSize = MeasureTextEx(ctx->font, itemName, fontSize, 1.0f);
        const float textRow1_Y = screenPos.y + (slotDst.height - textSize.y) * 0.15f;

        DrawTextEx(ctx->font, itemName, (Vector2){ screenPos.x + textPadX, textRow1_Y }, fontSize, 1.0f, isEquipped ? GREEN : WHITE);

        if (isEquipped) {
            const char *equipStr = "[E]";
            Vector2 equipTextSize = MeasureTextEx(ctx->font, equipStr, fontSize, 1.0f);
            DrawTextEx(ctx->font, equipStr, (Vector2){ (screenPos.x + slotDst.width) - equipTextSize.x - textPadRight, textRow1_Y }, fontSize, 1.0f, GREEN);
        }

        // 4. Draw Stack Count & Slot Type
        const float textRow2_Y = screenPos.y + (slotDst.height - textSize.y) * 0.75f;

        char countStr[16];
        snprintf(countStr, sizeof(countStr), "Stack: %d", bag->slot[bagIndex].quantity);
        DrawTextEx(ctx->font, countStr, (Vector2){ screenPos.x + textPadX, textRow2_Y }, fontSize, 1.0f, GRAY);

        char slotStr[16];
        snprintf(slotStr, sizeof(slotStr), "(%s)", SLOT_NAME[attr->slot]);
        Vector2 slotTextSize = MeasureTextEx(ctx->font, slotStr, fontSize, 1.0f);
        DrawTextEx(ctx->font, slotStr, (Vector2){ (screenPos.x + slotDst.width) - slotTextSize.x - textPadRight, textRow2_Y }, fontSize, 1.0f, GOLD);
    }
}

void Ivy_Scene_GameplayDrawUI(IvyGame *game)
{
    IvyPlayer *player = (IvyPlayer *)Ivy_ObjectManager_GetPlayer(game->objectManager)->data;
    const IvySceneGameplayData *gameplayData = game->sceneManager->actionScene->data;
    if (gameplayData->state == PAUSE_MENU_CLOSED) return;

    const IvyVirtualScreen *viewport = game->viewport;

    if (gameplayData->state == PAUSE_MENU_INVENTORY || gameplayData->menu.selected == 3)
    {
        DrawMenuBackground(game->textureManager, viewport);
        DrawMenuItems(game, gameplayData, viewport);
        DrawInventoryLayout(gameplayData, viewport);
        DrawEquippedItemIcons(player, gameplayData, game->textureManager, viewport);

        if (gameplayData->inventoryUI.selectedSlot != INVENTORY_SLOT_NONE) {
            DrawEquipSlots(game, viewport);
        }

        // Di fungsi wrapper utama lu (misal Ivy_Inventory_Draw):
        IvyItemDrawCtx drawCtx = {
            .ui          = &gameplayData->inventoryUI,
            .inventory   = Ivy_Player_GetInventory(player),
            .itemManager = &gameplayData->itemManager,
            .font        = game->fonts[IVY_FONT_SECONDARY], // Fix bug lu yang tadi ilang pointer game
            .scale       = viewport->scale
        };

        const Texture2D iconTex = Ivy_TextureManager_Get(game->textureManager, ASSET_TEXTURES_ICONS_DDS);
        DrawItemList(&drawCtx, viewport, &iconTex);

        return;
    }

    if (gameplayData->state == PAUSE_MENU_OPENED) {
        DrawMenuBackground(game->textureManager, viewport);
        DrawMenuItems(game, gameplayData, viewport);
    }
}
