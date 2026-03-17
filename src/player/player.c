#include "ivy/player/player.h"
#include "ivy/utils.h"
#include "ivy/game.h"

#include <stdlib.h>
#include <math.h>

Player *InitPlayer(const Vector2 spawnPoint)
{
    Player *player = calloc(1, sizeof(Player));
    IVY_ASSERT(player, "Failed to allocate Player");

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

    PlayerMovement *m     = &player->movement;
    m->tilePosition       = spawnPoint;
    m->targetTilePosition = m->tilePosition;
    m->position           = (Vector2){
        .x = spawnPoint.x * DEFAULT_TILE_SIZE + DEFAULT_TILE_HALF,
        .y = spawnPoint.y * DEFAULT_TILE_SIZE + DEFAULT_TILE_HALF
    };

    m->collisionBox = (Rectangle){
        .x      = m->position.x - DEFAULT_TILE_SIZE,
        .y      = m->position.y - DEFAULT_TILE_SIZE,
        .width  = DEFAULT_TILE_SIZE,
        .height = DEFAULT_TILE_SIZE
    };
    
    m->moveDuration = BASE_MOVE_DURATION;
    m->movement      = MOVEMENT_NORMAL;
    m->baseSpeed     = 4.0f;
    m->isMoving      = false;
    m->justTurned    = false;
    m->isHoldingKey  = false;
    m->turnTimer     = 0.0f;
    m->moveTimer     = 0.0f;

    player->animation.frameDirection = 1;
    player->inventory = CreateInventory();
    player->portrait  = CreatePortrait();

    return player;
}

void UpdatePlayer(Player *player, const float frameTime,
                  const Collision *collision, const u32 tileSize)
{
    if (player->animation.attackCooldown > 0.0f)
    {
        player->animation.attackCooldown -= frameTime;
        if (player->animation.attackCooldown < 0.0f)
            player->animation.attackCooldown = 0.0f;
    }

    if (player->graphics.action != ACTION_ATTACK &&
        player->animation.attackCooldown <= 0.0f  &&
        !player->movement.isMoving)
    {
        if (IsKeyPressed(KEY_J) || IsKeyPressed(KEY_Z))
        {
            player->animation.attackFrame      = 0;
            player->animation.attackFrameTimer = 0.0f;
            player->animation.attackHitApplied = false;
            player->graphics.action            = ACTION_ATTACK;
        }
    }

    if (player->graphics.action == ACTION_ATTACK)
    {
        UpdateAttack(player, frameTime, collision);
    }
    else
    {
        UpdatePlayerMovement(player, frameTime, collision, tileSize);
        UpdateAnimation(player, frameTime);
    }

    UpdatePlayerCollision(player);
}

void UpdatePlayerCollision(Player *player)
{
    player->movement.collisionBox.x = player->movement.position.x - DEFAULT_TILE_HALF;
    player->movement.collisionBox.y = player->movement.position.y - DEFAULT_TILE_HALF;
}

void DrawPlayer(const Player *player, const VirtualResolution *vr)
{
    (void)vr;

    const Rectangle src = {
        .x      = (float)GetSpriteCol(player) * CHARACTER_FRAME_SIZE,
        .y      = (float)GetSpriteRow(player) * CHARACTER_FRAME_SIZE,
        .width  = CHARACTER_FRAME_SIZE,
        .height = CHARACTER_FRAME_SIZE
    };

    const Rectangle dst = {
        .x      = floorf(player->movement.position.x),
        .y      = floorf(player->movement.position.y),
        .width  = CHARACTER_FRAME_SIZE,
        .height = CHARACTER_FRAME_SIZE
    };

    const Vector2 origin = { CHARACTER_FRAME_SIZE * 0.5f, CHARACTER_FRAME_SIZE * 0.75f };

    DrawTexturePro(player->graphics.bodyTexture, src, dst, origin, 0.0f, WHITE);

    const EquipmentSlot drawOrder[] = {
        SLOT_BOT, SLOT_MID, SLOT_MID_EXT,
        SLOT_TOP, SLOT_TOP_EXT, SLOT_S_ARM, SLOT_M_ARM,
        SLOT_ACC, SLOT_EXT_1
    };
    const u32 orderCount = sizeof(drawOrder) / sizeof(drawOrder[0]);

    for (u32 i = 0; i < orderCount; i++) {
        const EquipmentSlot slot = drawOrder[i];
        if (!(player->equipment.slotMask & 1u << slot)) continue;

        const Item *item = player->equipment.slots[slot];
        if (!item || item->type != ITEM_EQUIPMENT) continue;
        if (item->data.equipment.charTexture.id == 0) continue;

        DrawTexturePro(item->data.equipment.charTexture, src, dst, origin, 0.0f, WHITE);
    }

    DrawTexturePro(player->graphics.headTexture, src, dst, origin, 0.0f, WHITE);
    DrawTexturePro(player->graphics.hairTexture, src, dst, origin, 0.0f, WHITE);

    if (player->equipment.slotMask & 1u << SLOT_HEAD) {
        const Item *item = player->equipment.slots[SLOT_HEAD];
        if (item && item->type == ITEM_EQUIPMENT && item->data.equipment.charTexture.id != 0)
            DrawTexturePro(item->data.equipment.charTexture, src, dst, origin, 0.0f, WHITE);
    }
}

void DrawPlayerDebug(const Player *player)
{
    DrawRectangleLinesEx(player->movement.collisionBox, 1.0f, RED);
    DrawCircleV(player->movement.position, 2.0f, BLUE);

    if (player->graphics.action == ACTION_ATTACK)
        DrawRectangleLinesEx(GetAttackHitbox(player), 1.0f, YELLOW);
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

    const PlayerGraphics *g = &player->graphics;
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