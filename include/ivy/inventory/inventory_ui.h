#ifndef IVY_INVENTORY_UI_H
#define IVY_INVENTORY_UI_H

#include "inventory.h"
#include "ivy/virtual.h"
#include "raylib/raylib.h"

typedef struct Player Player;

typedef enum {
    INV_TAB_BAG = 0,
    INV_TAB_EQUIP,
    INV_TAB_COUNT
} InventoryTab;

typedef struct {
    Texture2D       screenshot;
    InventoryTab    activeTab;
    u32             selectedIndex;
    bool            isOpen;
    bool            pendingOpen;
} InventoryUI;

InventoryUI     CreateInventoryUI(void);
void            DestroyInventoryUI(InventoryUI *ui);

void            InventoryUIOpen(InventoryUI *ui, const VirtualResolution *vr);
void            InventoryUIClose(InventoryUI *ui);

bool            InventoryUIUpdate(InventoryUI *ui, Player *player);
void            InventoryUIDraw(const InventoryUI *ui, const Player *player, const VirtualResolution *vr, const Font *font);

#endif