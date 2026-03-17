#ifndef IVY_NPC_H
#define IVY_NPC_H

#include "ivy/types.h"
#include "ivy/item.h"
#include "ivy/npc/npc_internal.h"

#define NPC_SIZE_CAP 10

struct NPC {
    NPCGraphics  graphics;
    NPCMovement  movement;
    NPCEquipment equipment;
};

typedef struct {
    NPC npc[NPC_SIZE_CAP];
    u32 npcCount;
} NPCManager;

NPC         LoadNPC(Vector2 spawnPoint);
void        DrawNPC(const NPC *npc);
void        NPCRandomEquip(NPC *npc, const ItemTable *lut);

NPCManager *LoadNPCManager(void);
void        AddRandomNPC(NPCManager *manager, const ItemTable *lut, Vector2 spawnPoint);
void        UnloadNPCManager(NPCManager *manager);

#endif