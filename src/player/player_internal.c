#include "ivy/player/player.h"

u32 GetSpriteRow(const Player *player)
{
    u32 baseRow = 0;

    switch (player->graphics.action)
    {
        case ACTION_WALK: baseRow = 0; break;
        case ACTION_RUN:  baseRow = 4; break;
        case ACTION_IDLE: baseRow = 0; break;
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

void UpdateAnimation(Player *player, const float frameTime)
{
    PlayerAnimation *anim   = &player->animation;
    const PlayerAction action = player->graphics.action;

    if (action == ACTION_IDLE) {
        anim->currentFrame  = 1;
        anim->frameTimer    = 0.0f;
        return;
    }

    const float animSpeed = player->movement.moveDuration / 3.0f;
    anim->frameTimer += frameTime;

    if (anim->frameTimer >= animSpeed)
    {
        anim->frameTimer = 0.0f;

        const int delta = (anim->frameDirection == 1) ? 1 : -1;
        const int next  = (int)anim->currentFrame + delta;

        if (next >= 2) {
            anim->currentFrame  = 2;
            anim->frameDirection = 0;
        } else if (next <= 0) {
            anim->currentFrame  = 0;
            anim->frameDirection = 1;
        } else {
            anim->currentFrame = (u32)next;
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

float GetMoveDuration(const PlayerAction action)
{
    switch (action)
    {
        case ACTION_RUN:  return BASE_MOVE_DURATION * RUN_SPEED_MULTIPLIER;
        case ACTION_WALK: return BASE_MOVE_DURATION * WALK_SPEED_MULTIPLIER;
        default:          return BASE_MOVE_DURATION;
    }
}

bool StartMoving(Player *player, const Vector2 inputDir, const Direction nextDir, const bool isRunning,
                 const Collision *collision, const u32 tileSize)
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

    const PlayerAction action     = isRunning ? ACTION_RUN : ACTION_WALK;
    player->graphics.action       = action;
    player->movement.moveDuration = GetMoveDuration(action);
    player->movement.targetTilePosition = target;
    player->movement.isMoving     = true;
    player->movement.moveTimer    = 0.0f;

    return true;
}

void UpdatePlayerMovement(Player *player, const float frameTime,
                          const Collision *collision, const u32 tileSize)
{
    const float ts = (float)tileSize;

    Vector2   inputDir = {0};
    Direction nextDir  = player->graphics.direction;

    const bool hasInput = GetMovementInput(&inputDir, &nextDir);
    const bool isShift  = IsKeyDown(KEY_LEFT_SHIFT);

    player->movement.isHoldingKey = hasInput;

    if (!player->movement.isMoving)
    {
        if (!hasInput)
        {
            player->movement.dirInputCount  = 0;
            player->movement.justTurned     = false;
            player->graphics.action         = ACTION_IDLE;
        }
        else
        {
            const bool dirChanged = (player->graphics.direction != nextDir);

            if (dirChanged && DirectionKeyPressed(nextDir))
            {
                player->graphics.direction      = nextDir;
                player->graphics.action         = ACTION_IDLE;
                player->movement.dirInputCount  = 0;
                player->movement.justTurned     = true;
            }
            else
            {
                player->movement.dirInputCount++;
                const bool sameDir     = !dirChanged;
                const bool delayPassed = player->movement.dirInputCount > DIR_INPUT_DELAY;

                if (sameDir || (!player->movement.justTurned || delayPassed))
                {
                    player->movement.dirInputCount  = 0;
                    player->movement.justTurned     = false;
                    StartMoving(player, inputDir, nextDir, isShift, collision, tileSize);
                }
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

            Vector2   freshDir   = {0};
            Direction freshFacing = player->graphics.direction;
            const bool stillHolding = GetMovementInput(&freshDir, &freshFacing);

            if (stillHolding)
            {
                const Vector2 nextTarget = {
                    player->movement.tilePosition.x + freshDir.x,
                    player->movement.tilePosition.y + freshDir.y
                };

                if (!IsTileSolid(nextTarget, collision, tileSize))
                {
                    const PlayerAction nextAction   = isShift ? ACTION_RUN : ACTION_WALK;
                    player->movement.targetTilePosition = nextTarget;
                    player->movement.moveTimer      -= player->movement.moveDuration;
                    player->movement.moveDuration   = GetMoveDuration(nextAction);
                    player->graphics.action         = nextAction;
                    player->graphics.direction      = freshFacing;

                    t = player->movement.moveTimer / player->movement.moveDuration;
                }
                else
                {
                    player->movement.isMoving   = false;
                    player->graphics.action     = ACTION_IDLE;
                }
            }
            else
            {
                player->movement.isMoving       = false;
                player->movement.moveTimer      = 0.0f;
                player->movement.dirInputCount  = 0;
                player->graphics.action         = ACTION_IDLE;
                t = 1.0f;
            }
        }

        if (t > 1.0f) t = 1.0f;

        const float halfTs = ts * 0.5f;

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