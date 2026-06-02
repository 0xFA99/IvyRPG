#include "ivy/arena/linear.h"
#include "ivy/audio/buffer.h"
#include "ivy/audio/wav.h"
#include "ivy/core/game.h"
#include "ivy/core/types.h"
#include "ivy/entities/player.h"
#include "ivy/graphics/collusion.h"
#include "ivy/graphics/gfx.h"
#include "ivy/scenes/gameplay.h"
#include "ivy/systems/inventory.h"
#include "ivy/systems/item_manager.h"
#include "ivy/systems/texture_manager.h"
#include "ivy/utils/file_ids.h"
#include "ivy/utils/forward.h"

#include "raylib/raylib.h"
#include "raylib/rlgl.h"

#define PLAYER_SPRITESHEET_ATLAS_WIDTH  768
#define PLAYER_SPRITESHEET_ATLAS_HEIGHT 512

// Temporary Test
static const u8 BAKE_SLOT_ORDER[] = {
    IVY_SLOT_EXT_1,
    IVY_SLOT_BOT,
    IVY_SLOT_MID,
    IVY_SLOT_ACC,
    IVY_SLOT_HAIR,
};

static bool GetMovementInput(Vector2 *dir, IvyDirection *facing)
{
    dir->x = 0.0f;
    dir->y = 0.0f;

    const float v = (float)(IsKeyDown(KEY_DOWN) || IsKeyDown(KEY_S)) -
                    (float)(IsKeyDown(KEY_UP)   || IsKeyDown(KEY_W));

    if (v != 0.0f) {
        dir->y  = v;
        *facing = (v > 0.0f) ? IVY_DIRECTION_DOWN : IVY_DIRECTION_UP;
        return true;
    }

    const float h = (float)(IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D)) -
                    (float)(IsKeyDown(KEY_LEFT)  || IsKeyDown(KEY_A));

    if (h != 0.0f) {
        dir->x  = h;
        *facing = (h > 0.0f) ? IVY_DIRECTION_RIGHT : IVY_DIRECTION_LEFT;
        return true;
    }

    return false;
}

static u32 GetSpriteRow(const IvyPlayer *p)
{
    switch (p->graphics.direction) {
        case IVY_DIRECTION_DOWN:  return 0;
        case IVY_DIRECTION_LEFT:  return 1;
        case IVY_DIRECTION_RIGHT: return 2;
        case IVY_DIRECTION_UP:    return 3;
        default:                  return 0;
    }
}

IvyPlayer *Ivy_Player_Init(IvyArenaLinear *restrict arena, IvyAssetManager *restrict mgr, const Vector2 pos)
{
    IvyPlayer *p = Ivy_Arena_LinearAllocZero(arena, sizeof(IvyPlayer));

    p->movement.tilePosition = pos;
    p->movement.position     = (Vector2){ pos.x * IVY_TILE_SIZE, pos.y * IVY_TILE_SIZE };

    p->graphics.direction  = IVY_DIRECTION_DOWN;
    p->graphics.atlasReady = false;

    p->stepSound[0] = Ivy_Audio_LoadSoundWav(arena, mgr, ASSET_AUDIO_STEP1_WAV);
    p->stepSound[1] = Ivy_Audio_LoadSoundWav(arena, mgr, ASSET_AUDIO_STEP2_WAV);
    p->stepSound[2] = Ivy_Audio_LoadSoundWav(arena, mgr, ASSET_AUDIO_STEP3_WAV);
    p->stepSound[3] = Ivy_Audio_LoadSoundWav(arena, mgr, ASSET_AUDIO_STEP4_WAV);

    p->inventory = Ivy_Inventory_Init();

    return p;
}

void Ivy_Player_BakeAtlas(IvyGame *restrict game, IvySceneGameplayData *restrict gameplayData)
{
    IvyPlayer *player = gameplayData->player;
    IvyAssetManager *assetManager = game->assets;
    const IvyItemManager *itemManager = &gameplayData->itemManager;
    const IvyTextureManager *textureManager = game->texManager;

    if (player->graphics.atlasReady) {
        rlUnloadTexture(player->graphics.atlas.texture.id);
        rlUnloadFramebuffer(player->graphics.atlas.id);
    }

    player->graphics.atlas = LoadRenderTexture(
        PLAYER_SPRITESHEET_ATLAS_WIDTH,
        PLAYER_SPRITESHEET_ATLAS_HEIGHT
    );

    BeginTextureMode(player->graphics.atlas);
        ClearBackground(BLANK);
        DrawTexture(Ivy_TextureManager_Get(textureManager, ASSET_TEXTURES_SPRITESHEETS_BASE_BODY_DDS), 0, 0, WHITE);
        DrawTexture(Ivy_TextureManager_Get(textureManager, ASSET_TEXTURES_SPRITESHEETS_BASE_HEAD_DDS), 0, 0, WHITE);

        for (u32 i = 0; i < (u32)(sizeof(BAKE_SLOT_ORDER) / sizeof(BAKE_SLOT_ORDER[0])); i++) {
            const u8  eSlot  = BAKE_SLOT_ORDER[i];
            const u16 itemID = Ivy_Inventory_GetEquippedItemID(&player->inventory, eSlot);

            // if (itemID == 0) {
            //     if (eSlot == IVY_SLOT_HAIR) {
            //         DrawTexture(player->graphics.baseHair, 0, 0, WHITE);
            //     }
            //     continue;
            // }

            const IvyItemVisual *vis = Ivy_ItemManager_GetVisual(itemManager, itemID);
            if (!vis) continue;

            const Texture2D tex = Ivy_Gfx_LoadTextureDDS(assetManager, vis->spriteSheetID);
            DrawTexture(tex, 0, 0, WHITE);
        }

        DrawTexture(Ivy_TextureManager_Get(textureManager, ASSET_TEXTURES_SPRITESHEETS_BASE_HAIR_DDS), 0, 0, WHITE);

    EndTextureMode();

    player->graphics.atlasReady = true;
}

void Ivy_Player_EquipItem(IvyGame *game, IvySceneGameplayData *gameplayData, const u8 bagIndex)
{
    IvyPlayer *player = gameplayData->player;
    const IvyItemManager *itemManager = &gameplayData->itemManager;
    const IvyInventoryBag *bag = &player->inventory.bag;

    if (bagIndex >= bag->count) return;

    const u16 itemID = bag->slot[bagIndex].itemID;
    const IvyItemAttribute *attr = Ivy_ItemManager_GetAttribute(itemManager, itemID);
    if (!attr) return;

    Ivy_Inventory_Equip(&player->inventory, bagIndex, attr->slot);
    Ivy_Player_BakeAtlas(game, gameplayData);
}

void Ivy_Player_UnequipItem(IvyGame *game, IvySceneGameplayData *gameplayData, const u8 equipSlot)
{
    IvyPlayer *player = gameplayData->player;
    Ivy_Inventory_Unequip(&player->inventory, equipSlot);
    // Ivy_Player_BakeAtlas(player, assetManager, itemMgr);
    Ivy_Player_BakeAtlas(game, gameplayData);
}

void Ivy_Player_Update(IvyPlayer *player, const float dt,
                        const IvyCollusionMap *collisionMap)
{
    IvyPlayerMovement  *m = &player->movement;
    IvyPlayerAnimation *a = &player->animation;

    if (!m->isMoving) {
        Vector2      input   = {0};
        IvyDirection nextDir = player->graphics.direction;

        if (GetMovementInput(&input, &nextDir)) {
            if (player->graphics.direction != nextDir) {
                player->graphics.direction = nextDir;
                m->dirInputCount = 0;
                m->justTurned    = true;
            } else if (++m->dirInputCount > IVY_DIR_DELAY || !m->justTurned) {
                const Vector2 candidate = {
                    m->tilePosition.x + input.x,
                    m->tilePosition.y + input.y
                };
                const bool blocked = collisionMap && Ivy_Collusion_IsTileSolid(collisionMap, (int)candidate.x, (int)candidate.y);
                if (!blocked) {
                    m->targetTile           = candidate;
                    m->isMoving             = true;
                    m->moveTimer            = 0.0f;
                    m->justTurned           = false;
                    player->graphics.action = PLAYER_ACTION_WALK;
                }
            }
        } else {
            player->graphics.action = PLAYER_ACTION_IDLE;
            m->dirInputCount        = 0;
            m->justTurned           = false;
        }
    }

    if (m->isMoving) {
        m->moveTimer += dt;
        float t = m->moveTimer / IVY_MOVE_DURATION;
        if (t >= 1.0f) {
            t               = 1.0f;
            m->isMoving     = false;
            m->tilePosition = m->targetTile;
        }
        m->position.x = m->tilePosition.x * IVY_TILE_SIZE + (m->targetTile.x - m->tilePosition.x) * IVY_TILE_SIZE * t;
        m->position.y = m->tilePosition.y * IVY_TILE_SIZE + (m->targetTile.y - m->tilePosition.y) * IVY_TILE_SIZE * t;

        a->frameTimer += dt;
        if (a->frameTimer >= IVY_ANIM_SPEED) {
            a->frameTimer = 0;
            if (a->frameStep == 0) a->frameStep = 1;

            const u32 oldFrame = a->currentFrame;
            a->currentFrame   += a->frameStep;

            if (a->currentFrame >= IVY_WALK_FRAMES - 1) {
                a->currentFrame = IVY_WALK_FRAMES - 1;
                a->frameStep    = -1;
            } else if (a->currentFrame == 0) {
                a->frameStep    = 1;
            }

            if (a->currentFrame != oldFrame &&
                (a->currentFrame == 0 || a->currentFrame == 2)) {
                Ivy_Audio_PlayAudioBuffer(player->stepSound[GetRandomValue(0, 3)].data.stream.buffer);
            }
        }
    } else {
        a->currentFrame = 1;
        a->frameTimer   = 0;
        a->frameStep    = 1;
    }
}

void Ivy_Player_Render(const IvyPlayer *p)
{
    const float     offset   = (IVY_FRAME_SIZE - IVY_TILE_SIZE) * 0.5f;
    const Texture2D atlasTex = p->graphics.atlas.texture;
    const float     targetY  = (float)GetSpriteRow(p) * IVY_FRAME_SIZE;

    const Rectangle src = {
        (float)p->animation.currentFrame * IVY_FRAME_SIZE,
        (float)(atlasTex.height - IVY_FRAME_SIZE) - targetY,
        (float) IVY_FRAME_SIZE,
        (float)-IVY_FRAME_SIZE
    };
    const Rectangle dst = {
        p->movement.position.x - offset,
        p->movement.position.y - offset,
        (float)IVY_FRAME_SIZE,
        (float)IVY_FRAME_SIZE
    };

    DrawTexturePro(atlasTex, src, dst, (Vector2){0, 16}, 0.0f, WHITE);
}

void Ivy_Player_Unload(IvyPlayer *player)
{
    if (!player) return;

    if (player->graphics.atlas.texture.id > 0) {
        // rlUnloadTexture(player->graphics.atlas.texture.id);
        UnloadTexture(player->graphics.atlas.texture);
        player->graphics.atlas.texture.id = 0;
    }

    if (player->graphics.atlas.id > 0) {
        // rlUnloadFramebuffer(player->graphics.atlas.id);
        UnloadTexture(player->graphics.atlas.texture);
        player->graphics.atlas.id = 0;
    }

    player->graphics.atlasReady = false;

    for (int i = 0; i < 4; i++) {
        if (player->stepSound[i].data.stream.buffer != NULL) {
            Ivy_Audio_UnloadBuffer(player->stepSound[i].data.stream.buffer);
            player->stepSound[i].data.stream.buffer = NULL;
        }
    }
}

Vector2 Ivy_Player_GetPosition(const IvyPlayer *player)
{
    return player->movement.position;
}

IvyInventory *Ivy_Player_GetInventory(IvyPlayer *player)
{
    return &player->inventory;
}
