#include "ivy/entities/player.h"
#include "ivy/graphics/collusion.h"
#include "ivy/utils/utils.h"

#define IVY_PLAYER_DIRECTION_DELAY 6
#define IVY_PLAYER_MOVE_DURATION 0.25f
#define IVY_PLAYER_ANIM_SPEED 0.15f
#define IVY_PLAYER_FRAMES 3
#define IVY_PLAYER_ATTACK_ANIM_SPEED 0.08f
#define IVY_PLAYER_ATTACK_COOLDOWN 0.4f
#define IVY_PLAYER_HITBOX_W 24.0f
#define IVY_PLAYER_HITBOX_H 20.0f
#define IVY_PLAYER_HITBOX_REACH 20.0f

static bool GetMovementInput(Vector2 *direction, IvyDirection *facingDirection)
{
    direction->x = 0.0f;
    direction->y = 0.0f;

    const float vertical = (float)(IsKeyDown(KEY_DOWN) || IsKeyDown(KEY_S)) -
                           (float)(IsKeyDown(KEY_UP)   || IsKeyDown(KEY_W));

    if (vertical != 0.0f) {
        direction->y  = vertical;
        *facingDirection = (vertical > 0.0f) ? IVY_DIRECTION_DOWN : IVY_DIRECTION_UP;
        return true;
    }

    const float horizontal = (float)(IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D)) -
                             (float)(IsKeyDown(KEY_LEFT)  || IsKeyDown(KEY_A));

    if (horizontal != 0.0f) {
        direction->x  = horizontal;
        *facingDirection = (horizontal > 0.0f) ? IVY_DIRECTION_RIGHT : IVY_DIRECTION_LEFT;
        return true;
    }

    return false;
}

#ifdef IVY_DEBUG
static Rectangle Ivy_Player_GetAttackHitbox(const IvyPlayer *player)
{
    const float cx      = player->movement.position.x + (IVY_TILE_SIZE * 0.5f);
    const float cy      = player->movement.position.y + (IVY_TILE_SIZE * 0.5f);
    const float hw      = IVY_PLAYER_HITBOX_W * 0.5f;
    const float hh      = IVY_PLAYER_HITBOX_H * 0.5f;
    const float reach   = IVY_PLAYER_HITBOX_REACH;

    switch (player->graphics.direction) {
        case IVY_DIRECTION_DOWN:  return (Rectangle){ cx - hw, cy + reach - hh, IVY_PLAYER_HITBOX_W, IVY_PLAYER_HITBOX_H };
        case IVY_DIRECTION_UP:    return (Rectangle){ cx - hw, cy - reach - hh, IVY_PLAYER_HITBOX_W, IVY_PLAYER_HITBOX_H };
        case IVY_DIRECTION_LEFT:  return (Rectangle){ cx - reach - hw, cy - hh, IVY_PLAYER_HITBOX_W, IVY_PLAYER_HITBOX_H };
        case IVY_DIRECTION_RIGHT: return (Rectangle){ cx + reach - hw, cy - hh, IVY_PLAYER_HITBOX_W, IVY_PLAYER_HITBOX_H };
        default:                  return (Rectangle){ cx - hw, cy - hh, IVY_PLAYER_HITBOX_W, IVY_PLAYER_HITBOX_H };
    }
}
#endif

static void UpdateAttack(IvyPlayer *restrict player, const IvyCollusionMap *restrict collisionMap, const float deltaTime)
{
    IvyPlayerAnimation *anim = &player->animation;

    anim->attackFrameTimer += deltaTime;

    if (anim->attackFrameTimer >= IVY_PLAYER_ATTACK_ANIM_SPEED) {
        anim->attackFrameTimer = 0.0f;
        anim->attackFrame++;

        if (anim->attackFrame >= IVY_PLAYER_FRAMES) {
            // Attack selesai
            anim->attackFrame       = 0;
            anim->attackHitApplied  = false;
            anim->attackCooldown    = IVY_PLAYER_ATTACK_COOLDOWN;
            player->graphics.action = PLAYER_ACTION_IDLE;
            return;
        }
    }

#ifdef IVY_DEBUG
    if (anim->attackFrame == !anim->attackHitApplied) {
        anim->attackHitApplied = true;

        const Rectangle hitbox = Ivy_Player_GetAttackHitbox(player);

        if (collisionMap) {
            const int startX = (int)(hitbox.x / IVY_TILE_SIZE);
            const int startY = (int)(hitbox.y / IVY_TILE_SIZE);
            const int endX   = (int)((hitbox.x + hitbox.width) / IVY_TILE_SIZE);
            const int endY   = (int)((hitbox.y + hitbox.height) / IVY_TILE_SIZE);

            for (int y = startY; y <= endY; y++) {
                for (int x = startX; x <= endX; x++) {
                    if (Ivy_Collusion_IsTileSolid(collisionMap, x, y)) {
                        TraceLog(LOG_INFO, "ATTACK HIT at tile (%d, %d)", x, y);
                    }
                }
            }
        }
    }
#endif
}

static bool Ivy_Player_CanAttack(const IvyPlayer *player)
{
    return (player->animation.attackCooldown <= 0.0f &&
            player->graphics.action != PLAYER_ACTION_ATTACK);
}

static void Ivy_Player_Attack(IvyPlayer *player)
{
    if (!Ivy_Player_CanAttack(player)) return;

    player->animation.attackFrame       = 0;
    player->animation.attackFrameTimer  = 0.0f;
    player->animation.attackHitApplied  = false;
    player->graphics.action             = PLAYER_ACTION_ATTACK;
}

void Ivy_Player_Update(IvyPlayer *restrict player, const IvyCollusionMap *restrict collisionMap, const float deltaTime)
{
    IvyPlayerMovement *movement = &player->movement;
    IvyPlayerAnimation *animation = &player->animation;

    if (animation->attackCooldown > 0.0f) {
        animation->attackCooldown -= deltaTime;

        if (animation->attackCooldown < 0.0f) {
            animation->attackCooldown = 0.0f;
        }
    }

    if (player->graphics.action != PLAYER_ACTION_ATTACK && animation->attackCooldown <= 0.0f && !movement->isMoving) {
        if (IsKeyPressed(KEY_Z)) {
            Ivy_Player_Attack(player);
        }
    }

    if (player->graphics.action == PLAYER_ACTION_ATTACK) {
        UpdateAttack(player, collisionMap, deltaTime);
    }
    else {
        if (!movement->isMoving) {
            Vector2 input = {0};
            IvyDirection nextDirection = player->graphics.direction;

            if (GetMovementInput(&input, &nextDirection)) {
                if (player->graphics.direction != nextDirection) {
                    player->graphics.direction = nextDirection;
                    movement->dirInputCount = 0;
                    movement->justTurned = true;
                }
                else if (++movement->dirInputCount > IVY_PLAYER_DIRECTION_DELAY || !movement->justTurned) {
                    const Vector2 candidate = {
                        .x = movement->tilePosition.x + input.x,
                        .y = movement->tilePosition.y + input.y
                    };

                    const bool isSolid = collisionMap && Ivy_Collusion_IsTileSolid(collisionMap, (int)candidate.x, (int)candidate.y);
                    if (!isSolid) {
                        movement->targetTile = candidate;
                        movement->isMoving = true;
                        movement->moveTimer = 0.0f;
                        movement->justTurned = false;
                        player->graphics.action = PLAYER_ACTION_WALK;
                    }
                }
            }
            else {
                player->graphics.action = PLAYER_ACTION_IDLE;
                movement->dirInputCount = 0;
                movement->justTurned = false;
            }
        }

        if (movement->isMoving) {
            movement->moveTimer += deltaTime;
            float time = movement->moveTimer / IVY_PLAYER_MOVE_DURATION;
            if (time >= 1.0f) {
                time = 1.0f;
                movement->isMoving = false;
                movement->tilePosition = movement->targetTile;
            }

            movement->position = (Vector2) {
                .x = movement->tilePosition.x * IVY_TILE_SIZE + (movement->targetTile.x - movement->tilePosition.x) * IVY_TILE_SIZE * time,
                .y = movement->tilePosition.y * IVY_TILE_SIZE + (movement->targetTile.y - movement->tilePosition.y) * IVY_TILE_SIZE * time,
            };

            animation->frameTimer += deltaTime;
            if (animation->frameTimer >= IVY_PLAYER_ANIM_SPEED) {
                animation->frameTimer = 0;
                if (animation->frameStep == 0) {
                    animation->frameStep = 1;
                }

                const u32 oldFrame = animation->currentFrame;
                animation->currentFrame += animation->frameStep;

                if (animation->currentFrame >= IVY_PLAYER_FRAMES - 1) {
                    animation->currentFrame = IVY_PLAYER_FRAMES - 1;
                    animation->frameStep = -1;
                }
                else if (animation->currentFrame == 0) {
                    animation->frameStep = 1;
                }

                if (animation->currentFrame != oldFrame && (animation->currentFrame == 0 || animation->currentFrame == 2)) {
                    const u32 randomIdx = Ivy_Utils_RandomRange(0, 3);
                    Ivy_Audio_PlayAudioBuffer(player->stepSound[randomIdx].data.stream.buffer);
                }
            }
        }
        else {
            animation->currentFrame = 1;
            animation->frameTimer = 0;
            animation->frameStep = 1;
        }
    }
}
