#include "ivy/player/player.h"
#include "ivy/utils.h"

#include <assert.h>
#include <stdlib.h>
#include <math.h>

#define PLAYER_FRAME_SIZE   64.0f
#define PLAYER_COL_W        20.0f
#define PLAYER_COL_H        30.0f
#define PLAYER_COL_OX       (-10.0f)
#define PLAYER_COL_OY       (-15.0f)

Player *InitPlayer(const u32 spawnX, const u32 spawnY, const u32 tileSize)
{
    Player *player = calloc(1, sizeof(Player));
    assert(player && "[ERROR] Failed to allocate memory for Player!");

    PlayerGraphics *g = &player->graphics;
    g->hairTexture    = LoadTextureFromImageBin("assets/player/character/base/base_equip_hair.bin");
    g->headTexture    = LoadTextureFromImageBin("assets/player/character/base/base_equip_head.bin");
    g->bodyTexture    = LoadTextureFromImageBin("assets/player/character/base/base_equip_body.bin");

    g->headPortrait   = LoadTextureFromImageBin("assets/player/character/base/base_portrait_head.bin");
    g->bodyPortrait   = LoadTextureFromImageBin("assets/player/character/base/base_portrait_body.bin");
    g->hairPortrait   = LoadTextureFromImageBin("assets/player/character/base/base_portrait_hair.bin");
    g->eyesPortrait   = LoadTextureFromImageBin("assets/player/character/base/base_portrait_eyes.bin");
    g->mouthPortrait  = LoadTextureFromImageBin("assets/player/character/base/base_portrait_mouth.bin");

    g->action    = ACTION_IDLE;
    g->direction = DIRECTION_FRONT;

    const float ts   = (float)tileSize;
    const float half = ts * 0.5f;

    PlayerMovement *m     = &player->movement;
    m->tilePosition       = (Vector2){ (float)spawnX, (float)spawnY };
    m->targetTilePosition = m->tilePosition;
    m->position           = (Vector2){ (float)spawnX * ts + half, (float)spawnY * ts + half };

    m->collisionBox = (Rectangle){
        m->position.x + PLAYER_COL_OX, m->position.y + PLAYER_COL_OY,
        PLAYER_COL_W, PLAYER_COL_H
    };
    m->moveDuration = BASE_MOVE_DURATION;

    player->animation.frameDirection = 1;
    player->inventory = CreateInventory();
    player->portrait  = CreatePortrait();

    return player;
}

void UpdatePlayer(Player *player, const float frameTime,
                  const Collision *collision, const u32 tileSize)
{
    UpdatePlayerMovement(player, frameTime, collision, tileSize);
    UpdateAnimation(player, frameTime);
    UpdatePlayerCollision(player);
}

void UpdatePlayerCollision(Player *player)
{
    player->movement.collisionBox.x = player->movement.position.x + PLAYER_COL_OX;
    player->movement.collisionBox.y = player->movement.position.y + PLAYER_COL_OY;
}

void DrawPlayer(const Player *player, const VirtualResolution *vr)
{
    (void)vr;

    const Rectangle src = {
        .x      = (float)player->animation.currentFrame * PLAYER_FRAME_SIZE,
        .y      = (float)GetSpriteRow(player)           * PLAYER_FRAME_SIZE,
        .width  = PLAYER_FRAME_SIZE,
        .height = PLAYER_FRAME_SIZE
    };

    const Rectangle dst = {
        .x      = floorf(player->movement.position.x),
        .y      = floorf(player->movement.position.y),
        .width  = PLAYER_FRAME_SIZE,
        .height = PLAYER_FRAME_SIZE
    };

    const Vector2 origin = { PLAYER_FRAME_SIZE * 0.5f, PLAYER_FRAME_SIZE * 0.75f };

    DrawTexturePro(player->graphics.bodyTexture, src, dst, origin, 0.0f, WHITE);

    const EquipmentSlot drawOrder[] = {
        SLOT_BOT, SLOT_MID, SLOT_MID_EXT,
        SLOT_TOP, SLOT_TOP_EXT, SLOT_S_ARM, SLOT_M_ARM,
        SLOT_ACC, SLOT_EXT_1
    };
    const u32 orderCount = sizeof(drawOrder) / sizeof(drawOrder[0]);

    for (u32 i = 0; i < orderCount; i++) {
        const EquipmentSlot slot = drawOrder[i];
        if (!(player->equipment.slotMask & (1u << slot))) continue;

        const Item *item = player->equipment.slots[slot];
        if (!item || item->type != ITEM_EQUIPMENT) continue;
        if (item->data.equipment.charTexture.id == 0) continue;

        DrawTexturePro(item->data.equipment.charTexture, src, dst, origin, 0.0f, WHITE);
    }

    DrawTexturePro(player->graphics.headTexture, src, dst, origin, 0.0f, WHITE);
    DrawTexturePro(player->graphics.hairTexture, src, dst, origin, 0.0f, WHITE);

    if (player->equipment.slotMask & (1u << SLOT_HEAD)) {
        const Item *item = player->equipment.slots[SLOT_HEAD];
        if (item && item->type == ITEM_EQUIPMENT && item->data.equipment.charTexture.id != 0)
            DrawTexturePro(item->data.equipment.charTexture, src, dst, origin, 0.0f, WHITE);
    }
}

void DrawPlayerDebug(const Player *player)
{
    DrawRectangleLinesEx(player->movement.collisionBox, 1.0f, RED);
    DrawCircleV(player->movement.position, 2.0f, BLUE);
}

void PlayerEquip(Player *player, const u32 inventoryIndex)
{
    EquipItem(&player->equipment, player->inventory, inventoryIndex);
    player->portrait.dirty = true;
}

void PlayerUnequip(Player *player, const EquipmentSlot slot)
{
    UnequipSlot(&player->equipment, player->inventory, slot);
    player->portrait.dirty = true;
}

void DestroyPlayer(Player *player)
{
    if (!player) return;

    PlayerGraphics *g = &player->graphics;
    UnloadTexture(g->hairTexture);
    UnloadTexture(g->headTexture);
    UnloadTexture(g->bodyTexture);
    UnloadTexture(g->headPortrait);
    UnloadTexture(g->bodyPortrait);
    UnloadTexture(g->hairPortrait);
    UnloadTexture(g->eyesPortrait);
    UnloadTexture(g->mouthPortrait);

    DestroyInventory(player->inventory);
    DestroyPortrait(&player->portrait);
    free(player);
}