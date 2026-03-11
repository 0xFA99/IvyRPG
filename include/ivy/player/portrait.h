#ifndef IVY_PORTRAIT_H
#define IVY_PORTRAIT_H

#include "ivy/inventory.h"
#include "ivy/player/player_internal.h"
#include "ivy/virtual.h"
#include "raylib/raylib.h"

#define PORTRAIT_CANVAS_W   295
#define PORTRAIT_CANVAS_H   370

#define PORTRAIT_HUD_W      147.5f
#define PORTRAIT_HUD_H      185.0f

typedef struct {
    RenderTexture2D canva;
    bool            dirty;
} Portrait;

Portrait    CreatePortrait(void);
void        DestroyPortrait(Portrait *p);
void        RebuildPortrait(Portrait *p, const PlayerGraphics *graphics, const PlayerEquipment *equip);
void        DrawPortraitHUD(const Portrait *p, const VirtualResolution *vr);

#endif