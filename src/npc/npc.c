#include "ivy/npc/npc.h"
#include "ivy/game.h"
#include "ivy/utils.h"

#include <stdlib.h>

#define NPC_FRAME_SIZE 64.0f


NPC LoadNPC(const Vector2 spawnPoint)
{
    const float half = DEFAULT_TILE_SIZE * 0.5f;

    NPC npc = {0};

    npc.movement.tilePosition = spawnPoint;
    npc.movement.position = (Vector2){
        .x = spawnPoint.x * DEFAULT_TILE_SIZE + half,
        .y = spawnPoint.y * DEFAULT_TILE_SIZE + half
    };
    npc.movement.collisionBox = (Rectangle){
        .x      = npc.movement.position.x - half,
        .y      = npc.movement.position.y - half,
        .width  = DEFAULT_TILE_SIZE,
        .height = DEFAULT_TILE_SIZE
    };

    NPCGraphics *g  = &npc.graphics;
    g->headTexture  = LoadTextureFromImageBin("assets/player/character/base/base_equip_head.bin");
    g->bodyTexture  = LoadTextureFromImageBin("assets/player/character/base/base_equip_body.bin");
    g->hairTexture  = LoadTextureFromImageBin("assets/player/character/base/base_equip_hair.bin");

    return npc;
}

void NPCRandomEquip(NPC *npc, const ItemTable *lut)
{
    IVY_ASSERT(npc, "[NPC] NPCRandomEquip: npc is NULL.");
    IVY_ASSERT(lut, "[NPC] NPCRandomEquip: lut is NULL.");

    static const EquipmentSlot targetSlots[] = {
        SLOT_HEAD,
        SLOT_TOP,
        SLOT_MID,
        SLOT_BOT,
    };
    static const u32 targetCount = sizeof(targetSlots) / sizeof(targetSlots[0]);

    npc->equipment.slotMask = 0;
    for (u32 i = 0; i < SLOT_MAX_SIZE; i++)
        npc->equipment.slots[i] = NULL;

    for (u32 i = 0; i < targetCount; i++) {
        const EquipmentSlot slot  = targetSlots[i];
        const SlotBucket   *b     = &lut->buckets[slot];

        if (b->count == 0) continue;

        const u32 pick = (u32)GetRandomValue(0, (int)b->count - 1);
        npc->equipment.slots[slot]  = b->entries[pick];
        npc->equipment.slotMask    |= (1u << slot);
    }
}

void DrawNPC(const NPC *npc)
{
    const Rectangle src = {
        .x      = 0.0f,
        .y      = 0.0f,
        .width  = NPC_FRAME_SIZE,
        .height = NPC_FRAME_SIZE
    };

    const Rectangle dst = {
        .x      = npc->movement.position.x,
        .y      = npc->movement.position.y,
        .width  = NPC_FRAME_SIZE,
        .height = NPC_FRAME_SIZE
    };

    const Vector2 origin = { NPC_FRAME_SIZE * 0.5f, NPC_FRAME_SIZE * 0.75f };

    DrawRectangleRec(npc->movement.collisionBox, RED);

    DrawTexturePro(npc->graphics.bodyTexture, src, dst, origin, 0.0f, WHITE);

    static const EquipmentSlot drawOrder[] = {
        SLOT_BOT, SLOT_MID, SLOT_MID_EXT,
        SLOT_TOP, SLOT_TOP_EXT, SLOT_S_ARM, SLOT_M_ARM,
        SLOT_ACC, SLOT_EXT_1
    };
    static const u32 drawOrderCount = sizeof(drawOrder) / sizeof(drawOrder[0]);

    for (u32 i = 0; i < drawOrderCount; i++) {
        const EquipmentSlot slot = drawOrder[i];
        if (!(npc->equipment.slotMask & (1u << slot))) continue;

        const Item *item = npc->equipment.slots[slot];
        if (!item || item->type != ITEM_EQUIPMENT)       continue;
        if (item->data.equipment.charTexture.id == 0)    continue;

        DrawTexturePro(item->data.equipment.charTexture, src, dst, origin, 0.0f, WHITE);
    }

    DrawTexturePro(npc->graphics.headTexture, src, dst, origin, 0.0f, WHITE);
    DrawTexturePro(npc->graphics.hairTexture, src, dst, origin, 0.0f, WHITE);

    if (npc->equipment.slotMask & 1u << SLOT_HEAD) {
        const Item *item = npc->equipment.slots[SLOT_HEAD];
        if (item && item->type == ITEM_EQUIPMENT && item->data.equipment.charTexture.id != 0)
            DrawTexturePro(item->data.equipment.charTexture, src, dst, origin, 0.0f, WHITE);
    }
}

NPCManager *LoadNPCManager(void)
{
    NPCManager *manager = calloc(1, sizeof(NPCManager));
    IVY_ASSERT(manager, "[NPCManager] Failed to allocate memory.");
    return manager;
}

void AddRandomNPC(NPCManager *manager, const ItemTable *lut, const Vector2 spawnPoint)
{
    IVY_ASSERT(manager, "[NPCManager] instance not exist.");

    if (manager->npcCount >= NPC_SIZE_CAP) {
        TraceLog(LOG_WARNING, "[NPCManager] FULL (%d NPC).", NPC_SIZE_CAP);
        return;
    }

    NPC *npc = &manager->npc[manager->npcCount++];
    *npc = LoadNPC(spawnPoint);
    NPCRandomEquip(npc, lut);
}

void UnloadNPCManager(NPCManager *manager)
{
    IVY_ASSERT(manager, "[NPCManager] instance not exist.");

    for (u32 i = 0; i < manager->npcCount; i++) {
        const NPCGraphics *g = &manager->npc[i].graphics;
        UnloadTexture(g->headTexture);
        UnloadTexture(g->bodyTexture);
        UnloadTexture(g->hairTexture);
    }

    manager->npcCount = 0;
    free(manager);
}