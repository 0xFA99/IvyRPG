#include "ivy/inventory_ui.h"
#include "ivy/player/player.h"
#include "ivy/utils.h"

#include <math.h>
#include <string.h>

#define POPUP_X         20.0f
#define POPUP_Y         20.0f
#define POPUP_W         (VIRTUAL_WIDTH  - 40.0f)
#define POPUP_H         (VIRTUAL_HEIGHT - 40.0f)

#define TAB_H           14.0f
#define TAB_PAD         8.0f
#define CONTENT_Y       (POPUP_Y + TAB_H + TAB_PAD * 2.0f + 4.0f)
#define CONTENT_H       (POPUP_H - TAB_H - TAB_PAD * 2.0f - 8.0f)

#define SLOT_SIZE       28.0f
#define SLOT_PAD        16.0f
#define COLS            6
#define TEXT_SIZE       11.0f
#define ITEM_NAME_SIZE  10.0f

#define COLOR_BG        (Color){ 20,  20,  28,  230 }
#define COLOR_PANEL     (Color){ 30,  30,  42,  255 }
#define COLOR_BORDER    (Color){ 80,  70,  60,  255 }
#define COLOR_SELECTED  (Color){ 200, 170, 80,  255 }
#define COLOR_SLOT_BG   (Color){ 40,  40,  55,  255 }
#define COLOR_SLOT_SEL  (Color){ 80,  70,  40,  255 }
#define COLOR_TEXT      WHITE
#define COLOR_SUBTEXT   (Color){ 160, 150, 130, 255 }
#define COLOR_EQUIPPED  (Color){ 80,  200, 120, 255 }

InventoryUI CreateInventoryUI(void)
{
    return (InventoryUI){
        .screenshot    = {0},
        .activeTab     = INV_TAB_BAG,
        .selectedIndex = 0,
        .isOpen        = false,
        .pendingOpen   = false
    };
}

void DestroyInventoryUI(InventoryUI *ui)
{
    if (!ui) return;
    if (ui->screenshot.id != 0) UnloadTexture(ui->screenshot);
    ui->screenshot = (Texture2D){0};
}

void InventoryUIOpen(InventoryUI *ui, const VirtualResolution *vr)
{
    if (ui->isOpen) return;

    if (ui->screenshot.id != 0) UnloadTexture(ui->screenshot);

    Image img          = LoadImageFromTexture(vr->target.texture);
    ImageFlipVertical(&img);
    ui->screenshot     = LoadTextureFromImage(img);
    UnloadImage(img);

    ui->isOpen         = true;
    ui->selectedIndex  = 0;
    ui->activeTab      = INV_TAB_BAG;
    ui->pendingOpen    = false;
}

void InventoryUIClose(InventoryUI *ui)
{
    ui->isOpen = false;
}

bool InventoryUIUpdate(InventoryUI *ui, Player *player)
{
    if (!ui->isOpen) return false;

    if (IsKeyPressed(KEY_TAB)) {
        ui->activeTab     = (ui->activeTab + 1) % INV_TAB_COUNT;
        ui->selectedIndex = 0;
    }

    u32 itemCount = 0;
    if (ui->activeTab == INV_TAB_BAG)
        itemCount = player->inventory->count;
    else
        itemCount = SLOT_MAX_SIZE;

    if (itemCount > 0) {
        if (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S))
            ui->selectedIndex = (ui->selectedIndex + COLS) % itemCount;
        if (IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W)) {
            if (ui->selectedIndex >= COLS) ui->selectedIndex -= COLS;
            else ui->selectedIndex = 0;
        }
        if (IsKeyPressed(KEY_RIGHT) || IsKeyPressed(KEY_D))
            ui->selectedIndex = (ui->selectedIndex + 1) % itemCount;
        if (IsKeyPressed(KEY_LEFT) || IsKeyPressed(KEY_A)) {
            if (ui->selectedIndex > 0) ui->selectedIndex--;
        }
    }

    if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE)) {
        if (ui->activeTab == INV_TAB_BAG && player->inventory->count > 0)
        {
            PlayerEquip(player, ui->selectedIndex);

            if (player->inventory->count > 0 &&
                ui->selectedIndex >= player->inventory->count)
                ui->selectedIndex = player->inventory->count - 1;

        }

        else if (ui->activeTab == INV_TAB_EQUIP) {
            const EquipmentSlot slot = (EquipmentSlot)ui->selectedIndex;
            if (player->equipment.slotMask & (1u << slot))
                PlayerUnequip(player, slot);
        }
    }

    if (IsKeyPressed(KEY_ESCAPE) || IsKeyPressed(KEY_I))
        return true;

    return false;
}

static void DrawItemSlot(const Rectangle slotRect, const Item *item,
                         bool selected, bool equipped, const Font *font)
{
    const Color bgColor  = selected ? COLOR_SLOT_SEL : COLOR_SLOT_BG;
    const Color rimColor = selected ? COLOR_SELECTED : COLOR_BORDER;

    DrawRectangleRec(slotRect, bgColor);
    DrawRectangleLinesEx(slotRect, selected ? 1.5f : 1.0f, rimColor);

    if (!item) return;

    const Texture2D *tex = (item->type == ITEM_EQUIPMENT)
        ? &item->data.equipment.iconTexture : NULL;

    if (tex && tex->id != 0) {
        const float pad  = 2.0f;
        const Rectangle dst = {
            slotRect.x + pad, slotRect.y + pad,
            slotRect.width - pad * 2.0f, slotRect.height - pad * 2.0f
        };
        const Rectangle src = { 0, 0, (float)tex->width, (float)tex->height };
        DrawTexturePro(*tex, src, dst, (Vector2){0}, 0.0f, WHITE);
    }

    if (equipped) {
        DrawRectangle((int)(slotRect.x + slotRect.width - 8),
                      (int)slotRect.y, 8, 8, COLOR_EQUIPPED);
        if (font && font->baseSize > 0)
            DrawTextEx(*font, "E",
                (Vector2){ slotRect.x + slotRect.width - 7.5f, slotRect.y + 0.5f },
                6.0f, 0, BLACK);
    }
}

static void DrawItemPreview(const Item *item, const Rectangle panel,
                            const Font *font, float scale)
{
    DrawRectangleRec(panel, COLOR_PANEL);
    DrawRectangleLinesEx(panel, 1.0f, COLOR_BORDER);

    if (!item) {
        if (font && font->baseSize > 0)
            DrawTextEx(*font, "No equipment selected",
                (Vector2){ panel.x + 6.0f * scale, panel.y + 6.0f * scale },
                TEXT_SIZE * scale, 0, COLOR_SUBTEXT);
        return;
    }

    const float previewSize = panel.width - 12.0f * scale;
    const Rectangle imgDst = {
        panel.x + 6.0f * scale,
        panel.y + 6.0f * scale,
        previewSize,
        previewSize
    };

    if (item->type == ITEM_EQUIPMENT) {
        const Texture2D *tex = &item->data.equipment.portraitTex;
        if (tex->id != 0)
            DrawTexturePro(*tex,
                (Rectangle){ 0, 0, (float)tex->width, (float)tex->height },
                imgDst, (Vector2){0}, 0.0f, WHITE);
    }

    float textY = imgDst.y + imgDst.height + 6.0f * scale;

    if (item->name && font && font->baseSize > 0) {
        DrawTextEx(*font, item->name,
            (Vector2){ panel.x + 6.0f * scale, textY },
            TEXT_SIZE * scale, 0, COLOR_TEXT);
        textY += TEXT_SIZE * scale + 3.0f * scale;
    }

    if (item->type == ITEM_EQUIPMENT && font && font->baseSize > 0) {
        char buf[64];
        snprintf(buf, sizeof(buf), "Slot: %s",
                 EquipmentSlotName(item->data.equipment.slot));
        DrawTextEx(*font, buf,
            (Vector2){ panel.x + 6.0f * scale, textY },
            ITEM_NAME_SIZE * scale, 0, COLOR_SUBTEXT);
    }
}

void InventoryUIDraw(const InventoryUI *ui, const Player *player,
                     const VirtualResolution *vr, const Font *font)
{
    if (!ui->isOpen) return;

    const float scale = vr->scale;

    if (ui->screenshot.id != 0) {
        const Rectangle src = { 0, 0,
            (float)ui->screenshot.width, (float)ui->screenshot.height };
        DrawTexturePro(ui->screenshot, src, vr->destination,
                       (Vector2){0}, 0.0f, WHITE);
    }

    DrawRectangle(
        (int)vr->destination.x, (int)vr->destination.y,
        (int)vr->destination.width, (int)vr->destination.height,
        (Color){ 0, 0, 0, 160 }
    );

    const Vector2 popupOrigin = GetScreenPos(vr, (Vector2){ POPUP_X, POPUP_Y });
    const float   popupW      = POPUP_W * scale;
    const float   popupH      = POPUP_H * scale;

    DrawRectangle((int)popupOrigin.x, (int)popupOrigin.y,
                  (int)popupW, (int)popupH, COLOR_BG);
    DrawRectangleLinesEx(
        (Rectangle){ popupOrigin.x, popupOrigin.y, popupW, popupH },
        1.5f, COLOR_BORDER
    );

    const char *tabNames[INV_TAB_COUNT] = { "Bag", "Equipped" };
    float tabX = popupOrigin.x + TAB_PAD * scale;
    const float tabY = popupOrigin.y + TAB_PAD * scale;
    const float tabH = TAB_H * scale;

    for (u32 t = 0; t < INV_TAB_COUNT; t++) {
        const float tabW = 50.0f * scale;
        const bool  active = (ui->activeTab == (InventoryTab)t);

        DrawRectangle((int)tabX, (int)tabY, (int)tabW, (int)tabH,
                      active ? COLOR_PANEL : (Color){ 25, 25, 35, 255 });
        DrawRectangleLinesEx(
            (Rectangle){ tabX, tabY, tabW, tabH }, 1.0f,
            active ? COLOR_SELECTED : COLOR_BORDER
        );

        if (font && font->baseSize > 0)
            DrawTextEx(*font, tabNames[t],
                (Vector2){ tabX + 4.0f * scale, tabY + 2.0f * scale },
                TEXT_SIZE * scale, 0,
                active ? COLOR_SELECTED : COLOR_SUBTEXT);

        tabX += tabW + TAB_PAD * scale;
    }

    const Vector2 lineStart = GetScreenPos(vr, (Vector2){
        POPUP_X, POPUP_Y + TAB_H + TAB_PAD * 2.0f });
    DrawLineEx(lineStart,
        (Vector2){ lineStart.x + popupW, lineStart.y },
        1.0f, COLOR_BORDER);

    const Vector2 contentOrigin = GetScreenPos(vr, (Vector2){ POPUP_X, CONTENT_Y });
    const float   contentH      = CONTENT_H * scale;

    const float previewW = 90.0f * scale;
    const Rectangle previewPanel = {
        contentOrigin.x + 4.0f * scale,
        contentOrigin.y + 4.0f * scale,
        previewW,
        contentH - 8.0f * scale
    };

    const float gridStartX = previewPanel.x + previewW + 8.0f * scale;
    const float gridStartY = contentOrigin.y + 4.0f * scale;
    const float slotSz     = SLOT_SIZE * scale;
    const float slotPad    = SLOT_PAD  * scale;

    const Item *selectedItem = NULL;

    if (ui->activeTab == INV_TAB_BAG)
    {
        const Inventory *inv = player->inventory;

        if (inv->count > 0 && ui->selectedIndex < inv->count)
            selectedItem = inv->items[ui->selectedIndex];

        DrawItemPreview(selectedItem, previewPanel, font, scale);

        for (u32 i = 0; i < inv->count; i++) {
            const u32   col  = i % COLS;
            const u32   row  = i / COLS;
            const float sx   = gridStartX + (float)col * (slotSz + slotPad);
            const float sy   = gridStartY + (float)row * (slotSz + slotPad);

            const Rectangle slotRect = { sx, sy, slotSz, slotSz };
            const Item *item         = inv->items[i];
            const bool  sel          = (i == ui->selectedIndex);

            bool isEquipped = false;
            if (item && item->type == ITEM_EQUIPMENT) {
                const EquipmentSlot s = item->data.equipment.slot;
                isEquipped = (player->equipment.slotMask & (1u << s)) &&
                             (player->equipment.slots[s] == item);
            }

            DrawItemSlot(slotRect, item, sel, isEquipped, font);
        }

        if (inv->count == 0 && font && font->baseSize > 0)
            DrawTextEx(*font, "Bag is empty",
                (Vector2){ gridStartX, gridStartY },
                TEXT_SIZE * scale, 0, COLOR_SUBTEXT);

        const Vector2 hintPos = GetScreenPos(vr, (Vector2){
            POPUP_X + 4.0f, POPUP_Y + POPUP_H - 12.0f });
        if (font && font->baseSize > 0)
            DrawTextEx(*font, "[ENTER] Equip  [TAB] Switch  [ESC/I] Close",
                hintPos, ITEM_NAME_SIZE * scale, 0, COLOR_SUBTEXT);
    }

    else if (ui->activeTab == INV_TAB_EQUIP)
    {
        const u32 sel = ui->selectedIndex;
        if (sel < SLOT_MAX_SIZE) {
            const EquipmentSlot slot = (EquipmentSlot)sel;
            selectedItem = (player->equipment.slotMask & (1u << slot))
                         ? player->equipment.slots[slot] : NULL;
        }

        DrawItemPreview(selectedItem, previewPanel, font, scale);

        for (u32 s = 0; s < SLOT_MAX_SIZE; s++) {
            const u32   col  = s % COLS;
            const u32   row  = s / COLS;
            const float sx   = gridStartX + (float)col * (slotSz + slotPad);
            const float sy   = gridStartY + (float)row * (slotSz + slotPad);

            const Rectangle slotRect = { sx, sy, slotSz, slotSz };
            const bool      filled   = (player->equipment.slotMask & (1u << s)) != 0;
            const Item     *item     = filled ? player->equipment.slots[s] : NULL;
            const bool      isSel    = (s == sel);

            DrawItemSlot(slotRect, item, isSel, false, font);

            if (font && font->baseSize > 0)
                DrawTextEx(*font, EquipmentSlotName((EquipmentSlot)s),
                    (Vector2){ sx, sy + slotSz + 1.0f * scale },
                    ITEM_NAME_SIZE * scale, 0,
                    isSel ? COLOR_SELECTED : COLOR_SUBTEXT);
        }

        const Vector2 hintPos = GetScreenPos(vr, (Vector2){
            POPUP_X + 4.0f, POPUP_Y + POPUP_H - 12.0f });
        if (font && font->baseSize > 0)
            DrawTextEx(*font, "[ENTER] Unequip  [TAB] Switch  [ESC/I] Close",
                hintPos, ITEM_NAME_SIZE * scale, 0, COLOR_SUBTEXT);
    }
}