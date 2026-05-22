#include "ivy/entities/player.h"

#include "ivy/arena/linear.h"
#include "ivy/audio/buffer.h"
#include "ivy/graphics/gfx.h"
#include "ivy/graphics/collusion.h"
#include "ivy/audio/wav.h"
#include "ivy/utils/file_ids.h"

#include "raylib/rlgl.h"

struct IvyPlayer {
    IvyEquipment        equipment;
    IvyPlayerGraphic    graphics;
    IvyPlayerMovement   movement;
    IvyPlayerAnimation  animation;
    IvySound            stepSound[4];
};

IvyPlayer *Ivy_Player_Init(IvyArenaLinear *restrict arena, IvyAssetManager *restrict mgr, const Vector2 pos)
{
    IvyPlayer *p = Ivy_Arena_LinearAllocZero(arena, sizeof(IvyPlayer));

    p->movement.tilePosition = pos;
    p->movement.position     = (Vector2){ pos.x * IVY_TILE_SIZE, pos.y * IVY_TILE_SIZE };

    p->graphics.baseBody = Ivy_Gfx_LoadTextureDDS(mgr, ASSET_TEXTURES_SPRITESHEET_BASE_BODY_DDS);
    p->graphics.baseHead = Ivy_Gfx_LoadTextureDDS(mgr, ASSET_TEXTURES_SPRITESHEET_BASE_HEAD_DDS);
    p->graphics.baseHair = Ivy_Gfx_LoadTextureDDS(mgr, ASSET_TEXTURES_SPRITESHEET_BASE_HAIR_DDS);
    // p->graphics.baseHair = Ivy_Gfx_LoadTextureDDS(mgr, ASSET_HAIR_TWINBRAID_DDS);

    // p->graphics.shirt   = Ivy_Gfx_LoadTextureDDS(mgr, ASSET_BASIC_SHIRT_DDS);
    // p->graphics.pant = Ivy_Gfx_LoadTextureDDS(mgr, ASSET_BASIC_PANT_DDS);

    p->graphics.direction = IVY_DIRECTION_DOWN;
    p->graphics.atlasReady = false;

    p->stepSound[0] = Ivy_Audio_LoadSoundWav(arena, mgr, ASSET_AUDIO_STEP1_WAV);
    p->stepSound[1] = Ivy_Audio_LoadSoundWav(arena, mgr, ASSET_AUDIO_STEP2_WAV);
    p->stepSound[2] = Ivy_Audio_LoadSoundWav(arena, mgr, ASSET_AUDIO_STEP3_WAV);
    p->stepSound[3] = Ivy_Audio_LoadSoundWav(arena, mgr, ASSET_AUDIO_STEP4_WAV);

    return p;
}

void Ivy_Player_BakeAtlas(IvyPlayer *player)
{
    player->equipment.atlas = LoadRenderTexture(player->graphics.baseBody.width, player->graphics.baseBody.height);

    BeginTextureMode(player->equipment.atlas);
        ClearBackground(BLANK);
        DrawTexture(player->graphics.baseBody, 0, 0, WHITE);

        DrawTexture(player->equipment.outerBottom, 0, 0, WHITE);
        // DrawTexture(player->equipment.outerTop, 0, 0, WHITE);

        DrawTexture(player->graphics.baseHead, 0, 0, WHITE);
        DrawTexture(player->graphics.baseHair, 0, 0, WHITE);
    EndTextureMode();

    player->graphics.atlasReady = true;
}

static bool GetMovementInput(Vector2 *dir, IvyDirection *facing)
{
    // Reset
    dir->x = 0;
    dir->y = 0;

    // Check Vertical
    float v = (float)(IsKeyDown(KEY_DOWN) || IsKeyDown(KEY_S)) -
              (float)(IsKeyDown(KEY_UP)   || IsKeyDown(KEY_W));

    if (v != 0) {
        dir->y = v;
        *facing = (v > 0) ? IVY_DIRECTION_DOWN : IVY_DIRECTION_UP;
        return true;
    }

    // Check Horizontal
    float h = (float)(IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D)) -
              (float)(IsKeyDown(KEY_LEFT)  || IsKeyDown(KEY_A));

    if (h != 0) {
        dir->x = h;
        *facing = (h > 0) ? IVY_DIRECTION_RIGHT : IVY_DIRECTION_LEFT;
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

void Ivy_Player_Update(IvyPlayer *player, const float dt, const IvyCollusionMap *collisionMap)
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
                m->justTurned = true;
            } else {
                if (++m->dirInputCount > IVY_DIR_DELAY || !m->justTurned) {

                    /* ---- COLLISION CHECK ---- */
                    const Vector2 candidateTile = {
                        m->tilePosition.x + input.x,
                        m->tilePosition.y + input.y
                    };

                    bool blocked = false;

                    if (collisionMap != NULL) {
                        blocked = Ivy_Collusion_IsTileSolid(
                            collisionMap,
                            (int)candidateTile.x,
                            (int)candidateTile.y
                        );
                    }

                    if (!blocked) {
                        m->targetTile   = candidateTile;
                        m->isMoving     = true;
                        m->moveTimer    = 0.0f;
                        m->justTurned   = false;
                        player->graphics.action = PLAYER_ACTION_WALK;
                    }
                    /* If blocked, player stays in place – they can still
                     * change direction on the next frame.                  */
                }
            }
        } else {
            player->graphics.action = PLAYER_ACTION_IDLE;
            m->dirInputCount = 0;
            m->justTurned = false;
        }
    }

    if (m->isMoving) {
        m->moveTimer += dt;
        float t = m->moveTimer / IVY_MOVE_DURATION;

        if (t >= 1.0f) {
            t = 1.0f;
            m->isMoving    = false;
            m->tilePosition = m->targetTile;
        }

        m->position.x = m->tilePosition.x * IVY_TILE_SIZE + (m->targetTile.x - m->tilePosition.x) * IVY_TILE_SIZE * t;
        m->position.y = m->tilePosition.y * IVY_TILE_SIZE + (m->targetTile.y - m->tilePosition.y) * IVY_TILE_SIZE * t;

        a->frameTimer += dt;
        if (a->frameTimer >= IVY_ANIM_SPEED) {
            a->frameTimer = 0;

            if (a->frameStep == 0) a->frameStep = 1;

            const u32 oldFrame = a->currentFrame;

            a->currentFrame += a->frameStep;

            if (a->currentFrame >= (IVY_WALK_FRAMES - 1)) {
                a->currentFrame = IVY_WALK_FRAMES - 1;
                a->frameStep = -1; // Balik mundur
            }
            else if (a->currentFrame <= 0) {
                a->currentFrame = 0;
                a->frameStep = 1;  // Maju lagi
            }

            if (a->currentFrame != oldFrame && (a->currentFrame == 0 || a->currentFrame == 2)) {
                int randomIndex = GetRandomValue(0, 3);
                Ivy_Audio_PlayAudioBuffer(player->stepSound[randomIndex].data.stream.buffer);
            }
        }
    } else {
        // idle, back to normal frame
        a->currentFrame = 1;
        a->frameTimer = 0;
        a->frameStep = 1;
    }
}

void Ivy_Player_Render(const IvyPlayer *p)
{
    const float offset = (IVY_FRAME_SIZE - IVY_TILE_SIZE) * 0.5f;
    const Texture2D atlasTex = p->equipment.atlas.texture;

    const float targetY = (float)GetSpriteRow(p) * IVY_FRAME_SIZE;

    const Rectangle src = {
        (float)p->animation.currentFrame * IVY_FRAME_SIZE,
        (float)(atlasTex.height - IVY_FRAME_SIZE) - targetY,
        (float)IVY_FRAME_SIZE,
        (float)-IVY_FRAME_SIZE
    };

    Rectangle dst = {
        p->movement.position.x - offset,
        p->movement.position.y - offset,
        (float)IVY_FRAME_SIZE,
        (float)IVY_FRAME_SIZE
    };

    DrawTexturePro(atlasTex, src, dst, (Vector2){0, 16}, 0.0f, WHITE);
}

void Ivy_Player_Unload(IvyPlayer *player)
{
    if (player == NULL) return;

    if (player->graphics.atlasReady) {
        rlUnloadTexture(player->equipment.atlas.texture.id);
        rlUnloadFramebuffer(player->equipment.atlas.id);

        player->graphics.atlasReady = false;
    }

    Ivy_Gfx_UnloadTexture(&player->graphics.baseBody);
    Ivy_Gfx_UnloadTexture(&player->graphics.baseHead);
    Ivy_Gfx_UnloadTexture(&player->graphics.baseHair);

    Ivy_Audio_UnloadSound(&player->stepSound[0]);
    Ivy_Audio_UnloadSound(&player->stepSound[1]);
    Ivy_Audio_UnloadSound(&player->stepSound[2]);
    Ivy_Audio_UnloadSound(&player->stepSound[3]);
}

Vector2 Ivy_Player_GetPosition(const IvyPlayer *player)
{
    return player->movement.position;
}

void Ivy_Player_EquipItem(IvyPlayer *restrict player, IvyAssetManager *restrict assetManager, IvyItemManager *restrict itemManager)
{
    // player->graphics.shirt = &itemManager->items[0].spriteSheet;
    // player->graphics.pants = &itemManager->items[1].spriteSheet;

    // player->equipment.outerTop = Ivy_Gfx_LoadTextureDDS(assetManager, ASSET_TEXTURES_SPRITESHEETS_NOVICE_SHIRT_DDS);
    // player->equipment.outerBottom = Ivy_Gfx_LoadTextureDDS(assetManager, ASSET_TEXTURES_SPRITESHEETS_NOVICE_PANTS_DDS);
    player->equipment.outerBottom = Ivy_Gfx_LoadTextureDDS(assetManager, ASSET_TEXTURES_SPRITESHEETS_NOVICE_PANTY_DDS);

    Ivy_Player_BakeAtlas(player);
}
