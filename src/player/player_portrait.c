#include "ivy/player/player_portrait.h"
#include "ivy/item.h"
#include "ivy/utils.h"

#include <math.h>


Portrait CreatePortrait(void)
{
    Portrait p = {0};
    p.canva    = LoadRenderTexture(PORTRAIT_CANVAS_W, PORTRAIT_CANVAS_H);
    p.dirty    = true;
    return p;
}

void DestroyPortrait(Portrait *p)
{
    if (!p) return;
    UnloadRenderTexture(p->canva);
}

static void DrawLayer(const Texture2D *tex, const float srcW, const float srcH, const float posX, const float posY)
{
    if (!tex || tex->id == 0) return;
    DrawTextureRec(*tex,
        (Rectangle){ 0.0f, 0.0f, srcW, srcH },
        (Vector2){ posX, posY },
        WHITE);
}

void RebuildPortrait(Portrait *p, const PlayerGraphics *graphics, const PlayerEquipment *equip)
{
    if (!p || !p->dirty) return;

    BeginTextureMode(p->canva);
    ClearBackground(BLANK);

    const Texture2D *basePortrait = &graphics->bodyPortrait;

    DrawLayer(basePortrait, (float)basePortrait->width, (float)basePortrait->height, 5.0f, 118.0f);

    for (u32 i = SLOT_MAX_SIZE; i-- > 0; )
    {
        if (!(equip->slotMask & 1u << i)) continue;

        const Item *item = equip->slots[i];
        if (!item) continue;

        const EquipmentSlot slot = item->data.equipment.slot;
        if (slot == SLOT_MID || slot == SLOT_MID_EXT || slot == SLOT_TOP || slot == SLOT_BOT) {
            const Texture2D *tex = &item->data.equipment.portraitTex;
            const Vector2 pos = item->data.equipment.position;
            DrawLayer(tex, (float)tex->width, (float)tex->height, pos.x, pos.y);
        }
    }

    basePortrait = &graphics->headPortrait;
    DrawLayer(basePortrait, (float)basePortrait->width, (float)basePortrait->height, 102.0f, 30.0f);

    basePortrait = &graphics->hairPortrait;
    DrawLayer(basePortrait, (float)basePortrait->width, (float)basePortrait->height, 100.0f, 31.0f);

    for (u32 i = SLOT_MAX_SIZE; i-- > 0; )
    {
        if (!(equip->slotMask & 1u << i)) continue;

        const Item *item = equip->slots[i];
        if (!item) continue;

        const EquipmentSlot slot = item->data.equipment.slot;
        if (slot == SLOT_HEAD || slot == SLOT_EXT_1) {
            const Texture2D *tex = &item->data.equipment.portraitTex;
            const Vector2 pos = item->data.equipment.position;
            DrawLayer(tex, (float)tex->width, (float)tex->height, pos.x, pos.y);
        }
    }

    basePortrait = &graphics->mouthPortrait;
    DrawLayer(basePortrait, (float)basePortrait->width, (float)basePortrait->height, 126.0f, 116.0f);

    basePortrait = &graphics->eyesPortrait;
    DrawLayer(basePortrait, (float)basePortrait->height, (float)basePortrait->height, 108.0f, 52.0f);

    EndTextureMode();

    p->dirty = false;
}

void DrawPortraitHUD(const Portrait *p, const VirtualResolution *vr)
{
    if (!p) return;

    const float margin = 6.0f;
    const float hw     = PORTRAIT_HUD_W;
    const float hh     = PORTRAIT_HUD_H;

    const Vector2 vPos = {
        VIRTUAL_WIDTH  - hw - margin,
        VIRTUAL_HEIGHT - hh - margin
    };

    const Vector2 sPos  = GetScreenPos(vr, vPos);
    const float   scale = vr->scale;

    const Rectangle dst = {
        sPos.x, sPos.y,
        floorf(hw * scale),
        floorf(hh * scale)
    };

    const Rectangle src = {
        0.0f, 0.0f,
        (float)PORTRAIT_CANVAS_W,
        -(float)PORTRAIT_CANVAS_H
    };

    DrawTexturePro(p->canva.texture, src, dst, (Vector2){0}, 0.0f, WHITE);
}