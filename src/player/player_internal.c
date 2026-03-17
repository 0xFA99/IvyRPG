#include "ivy/player/player.h"

u32 GetSpriteRow(const Player *player)
{
    if (player->graphics.action == ACTION_ATTACK)
    {
        switch (player->graphics.direction)
        {
            case DIRECTION_FRONT: return 4;
            case DIRECTION_LEFT:  return 5;
            case DIRECTION_RIGHT: return 6;
            case DIRECTION_BACK:  return 7;
            default:              return 4;
        }
    }

    u32 baseRow = 0;
    switch (player->graphics.action)
    {
        case ACTION_WALK: baseRow = 0; break;
        case ACTION_RUN:  baseRow = 4; break;
        case ACTION_IDLE: baseRow = 0; break;
        default:          baseRow = 0; break;
    }

    switch (player->graphics.direction)
    {
        case DIRECTION_FRONT: return baseRow + 0;
        case DIRECTION_LEFT:  return baseRow + 1;
        case DIRECTION_RIGHT: return baseRow + 2;
        case DIRECTION_BACK:  return baseRow + 3;
        default:              return baseRow;
    }
}

u32 GetSpriteCol(const Player *player)
{
    if (player->graphics.action == ACTION_ATTACK)
        return ATTACK_ANIM_START_COL + player->animation.attackFrame;

    return player->animation.currentFrame;
}

void UpdateAnimation(Player *player, const float frameTime)
{
    PlayerAnimation *anim    = &player->animation;
    const PlayerAction action = player->graphics.action;

    if (action == ACTION_ATTACK) return;

    if (action == ACTION_IDLE) {
        anim->currentFrame   = 1;
        anim->frameTimer     = 0.0f;
        return;
    }

    const float realMoveSpeed = player->movement.baseSpeed + (player->movement.movement == MOVEMENT_DASH ? 1.0f : 0.0f);
    const float threshold     = 18.0f - realMoveSpeed * 2.0f;
    const float animSpeed     = threshold / 90.0f;

    anim->frameTimer += frameTime;

    if (anim->frameTimer >= animSpeed)
    {
        anim->frameTimer = 0.0f;

        const int delta = (anim->frameDirection == 1) ? 1 : -1;
        const int next  = (int)anim->currentFrame + delta;

        if (next >= 2) {
            anim->currentFrame   = 2;
            anim->frameDirection = 0;
        } else if (next <= 0) {
            anim->currentFrame   = 0;
            anim->frameDirection = 1;
        } else {
            anim->currentFrame = (u32)next;
        }
    }
}

Rectangle GetAttackHitbox(const Player *player)
{
    const float cx    = player->movement.position.x;
    const float cy    = player->movement.position.y;
    const float hw    = ATTACK_HITBOX_W * 0.5f;
    const float hh    = ATTACK_HITBOX_H * 0.5f;
    const float reach = ATTACK_HITBOX_REACH;

    switch (player->graphics.direction)
    {
        case DIRECTION_FRONT:   return (Rectangle){ cx - hw, cy + reach - hh, ATTACK_HITBOX_W, ATTACK_HITBOX_H };
        case DIRECTION_BACK:    return (Rectangle){ cx - hw, cy - reach - hh, ATTACK_HITBOX_W, ATTACK_HITBOX_H };
        case DIRECTION_LEFT:    return (Rectangle){ cx - reach - hw, cy - hh, ATTACK_HITBOX_W, ATTACK_HITBOX_H };
        case DIRECTION_RIGHT:   return (Rectangle){ cx + reach - hw, cy - hh, ATTACK_HITBOX_W, ATTACK_HITBOX_H };
        default:                return (Rectangle){ cx - hw, cy - hh, ATTACK_HITBOX_W, ATTACK_HITBOX_H };
    }
}

void UpdateAttack(Player *player, const float frameTime, const Collision *collision)
{
    PlayerAnimation *anim = &player->animation;

    anim->attackFrameTimer += frameTime;

    if (anim->attackFrameTimer >= ATTACK_ANIM_SPEED)
    {
        anim->attackFrameTimer = 0.0f;
        anim->attackFrame++;

        if (anim->attackFrame >= ATTACK_ANIM_FRAMES)
        {
            anim->attackFrame       = 0;
            anim->attackHitApplied  = false;
            anim->attackCooldown    = ATTACK_COOLDOWN;
            player->graphics.action = ACTION_IDLE;
            return;
        }
    }

    if (anim->attackFrame == ATTACK_HIT_FRAME && !anim->attackHitApplied)
    {
        anim->attackHitApplied = true;

        const Rectangle hitbox = GetAttackHitbox(player);

        for (u32 i = 0; i < collision->rectCount; i++)
        {
            if (CheckCollisionRecs(hitbox, collision->rect[i]))
            {
                // TODO: Hit Something lol
                TraceLog(LOG_INFO, "ATTACK HIT: collision rect [%u]", i);
            }
        }
    }
}

bool DirectionKeyPressed(const Direction dir)
{
    switch (dir)
    {
        case DIRECTION_BACK:  return IsKeyPressed(KEY_W) || IsKeyPressed(KEY_UP);
        case DIRECTION_FRONT: return IsKeyPressed(KEY_S) || IsKeyPressed(KEY_DOWN);
        case DIRECTION_LEFT:  return IsKeyPressed(KEY_A) || IsKeyPressed(KEY_LEFT);
        case DIRECTION_RIGHT: return IsKeyPressed(KEY_D) || IsKeyPressed(KEY_RIGHT);
        default:              return false;
    }
}

bool GetMovementInput(Vector2 *outDir, Direction *outFacing)
{
    *outDir = (Vector2){0};

    if (IsKeyDown(KEY_W) || IsKeyDown(KEY_UP))    { outDir->y = -1; *outFacing = DIRECTION_BACK;  return true; }
    if (IsKeyDown(KEY_S) || IsKeyDown(KEY_DOWN))  { outDir->y =  1; *outFacing = DIRECTION_FRONT; return true; }
    if (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT))  { outDir->x = -1; *outFacing = DIRECTION_LEFT;  return true; }
    if (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT)) { outDir->x =  1; *outFacing = DIRECTION_RIGHT; return true; }

    return false;
}

bool IsTileSolid(const Vector2 tilePos, const Collision *collision, const u32 tileSize)
{
    const float ts = (float)tileSize;
    const Vector2 worldCenter = {
        tilePos.x * ts + ts * 0.5f,
        tilePos.y * ts + ts * 0.5f
    };

    for (u32 i = 0; i < collision->rectCount; i++) {
        if (CheckCollisionPointRec(worldCenter, collision->rect[i]))
            return true;
    }

    return false;
}

float GetMoveDuration(const MovementType movement, const float baseSpeed)
{
    float actualSpeed;

    switch (movement)
    {
        case MOVEMENT_DASH:
            actualSpeed = baseSpeed * SPEED_DASH;
            if (actualSpeed > MAX_SPEED_DASH)
                actualSpeed = MAX_SPEED_DASH;
            break;
        case MOVEMENT_NORMAL:
        default:
            actualSpeed = baseSpeed * SPEED_NORMAL;
            if (actualSpeed > MAX_SPEED_NORMAL)
                actualSpeed = MAX_SPEED_NORMAL;
            break;
    }

    return BASE_MOVE_DURATION / actualSpeed;
}

bool StartMoving(Player *player, const Vector2 inputDir, const Direction nextDir,
                 const MovementType movement, const Collision *collision, const u32 tileSize)
{
    const Vector2 target = {
        player->movement.tilePosition.x + inputDir.x,
        player->movement.tilePosition.y + inputDir.y
    };

    player->graphics.direction = nextDir;

    if (IsTileSolid(target, collision, tileSize)) {
        player->graphics.action = ACTION_IDLE;
        return false;
    }

    player->movement.movement     = movement;
    player->graphics.action       = movement == MOVEMENT_DASH ? ACTION_RUN : ACTION_WALK;
    player->movement.moveDuration = GetMoveDuration(movement, player->movement.baseSpeed);
    player->movement.targetTilePosition = target;
    player->movement.isMoving     = true;
    player->movement.moveTimer    = 0.0f;

    return true;
}

void UpdatePlayerMovement(Player *player, const float frameTime,
                          const Collision *collision, const u32 tileSize)
{
    if (player->graphics.action == ACTION_ATTACK) return;

    const float ts = (float)tileSize;

    Vector2   inputDir = {0};
    Direction nextDir  = player->graphics.direction;

    const bool hasInput        = GetMovementInput(&inputDir, &nextDir);
    const bool isShift         = IsKeyDown(KEY_LEFT_SHIFT);
    const MovementType desired = isShift ? MOVEMENT_DASH : MOVEMENT_NORMAL;

    player->movement.isHoldingKey = hasInput;

    if (player->movement.isMoving && desired != player->movement.movement)
    {
        float progress = player->movement.moveTimer / player->movement.moveDuration;
        if (progress > 1.0f) progress = 1.0f;

        const float newDuration           = GetMoveDuration(desired, player->movement.baseSpeed);
        player->movement.moveTimer        = progress * newDuration;
        player->movement.moveDuration     = newDuration;
        player->movement.movement         = desired;
        player->graphics.action           = desired == MOVEMENT_DASH ? ACTION_RUN : ACTION_WALK;
    }

    if (!player->movement.isMoving)
    {
        if (!hasInput)
        {
            player->movement.turnTimer  = 0.0f;
            player->movement.justTurned = false;
            player->graphics.action     = ACTION_IDLE;
        }
        else
        {
            const bool dirChanged = (player->graphics.direction != nextDir);

            if (dirChanged)
            {
                player->graphics.direction  = nextDir;
                player->graphics.action     = ACTION_IDLE;
                player->movement.turnTimer  = 0.0f;
                player->movement.justTurned = true;
            }
            else if (player->movement.justTurned)
            {
                player->movement.turnTimer += frameTime;

                if (player->movement.turnTimer >= DIR_TURN_DELAY)
                {
                    player->movement.turnTimer  = 0.0f;
                    player->movement.justTurned = false;
                    StartMoving(player, inputDir, nextDir, desired, collision, tileSize);
                }
            }
            else
            {
                StartMoving(player, inputDir, nextDir, desired, collision, tileSize);
            }
        }
    }

    if (player->movement.isMoving)
    {
        player->movement.moveTimer += frameTime;
        float t = player->movement.moveTimer / player->movement.moveDuration;

        if (t >= 1.0f)
        {
            player->movement.tilePosition = player->movement.targetTilePosition;

            Vector2   freshDir    = {0};
            Direction freshFacing = player->graphics.direction;
            const bool stillHolding = GetMovementInput(&freshDir, &freshFacing);

            if (stillHolding)
            {
                const bool freshShift       = IsKeyDown(KEY_LEFT_SHIFT);
                const MovementType nextMove = freshShift ? MOVEMENT_DASH : MOVEMENT_NORMAL;
                const Vector2 nextTarget    = {
                    player->movement.tilePosition.x + freshDir.x,
                    player->movement.tilePosition.y + freshDir.y
                };

                if (!IsTileSolid(nextTarget, collision, tileSize))
                {
                    player->movement.targetTilePosition = nextTarget;
                    player->movement.moveTimer         -= player->movement.moveDuration;
                    player->movement.moveDuration       = GetMoveDuration(nextMove, player->movement.baseSpeed);
                    player->movement.movement           = nextMove;
                    player->graphics.action             = (nextMove == MOVEMENT_DASH) ? ACTION_RUN : ACTION_WALK;
                    player->graphics.direction          = freshFacing;

                    t = player->movement.moveTimer / player->movement.moveDuration;
                }
                else
                {
                    player->movement.isMoving = false;
                    player->graphics.action   = ACTION_IDLE;
                }
            }
            else
            {
                player->movement.isMoving  = false;
                player->movement.moveTimer = 0.0f;
                player->graphics.action    = ACTION_IDLE;
                t = 1.0f;
            }
        }

        if (t > 1.0f) t = 1.0f;

        const float halfTs     = ts * 0.5f;
        const Vector2 startPos = {
            player->movement.tilePosition.x * ts + halfTs,
            player->movement.tilePosition.y * ts + halfTs
        };
        const Vector2 endPos = {
            player->movement.targetTilePosition.x * ts + halfTs,
            player->movement.targetTilePosition.y * ts + halfTs
        };

        player->movement.position.x = startPos.x + (endPos.x - startPos.x) * t;
        player->movement.position.y = startPos.y + (endPos.y - startPos.y) * t;
    }
}