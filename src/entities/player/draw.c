#include "ivy/entities/player.h"

void Ivy_Player_Render(const IvyPlayer *player)
{
    const float offset = (IVY_PLAYER_FRAME_SIZE - IVY_TILE_SIZE) * 0.5f;
    const Texture2D atlasTex = player->graphics.atlas.texture;

    u32 spriteRow = 0;
    u32 spriteCol = 0;

    if (player->graphics.action == PLAYER_ACTION_ATTACK) {
        switch (player->graphics.direction) {
            case IVY_DIRECTION_DOWN:    spriteRow = 4; break;
            case IVY_DIRECTION_LEFT:    spriteRow = 5; break;
            case IVY_DIRECTION_RIGHT:   spriteRow = 6; break;
            case IVY_DIRECTION_UP:      spriteRow = 7; break;
            default:                    spriteRow = 4; break;
        }

        spriteCol = 6 + player->animation.attackFrame;
    }
    else {
        switch (player->graphics.direction) {
            case IVY_DIRECTION_DOWN:  spriteRow = 0; break;
            case IVY_DIRECTION_LEFT:  spriteRow = 1; break;
            case IVY_DIRECTION_RIGHT: spriteRow = 2; break;
            case IVY_DIRECTION_UP:    spriteRow = 3; break;
            default:                  spriteRow = 0; break;
        }
        spriteCol = player->animation.currentFrame;
    }

    const float targetY = (float)spriteRow * IVY_PLAYER_FRAME_SIZE;

    const Rectangle src = {
        (float)spriteCol * IVY_PLAYER_FRAME_SIZE,
        (float)(atlasTex.height - IVY_PLAYER_FRAME_SIZE) - targetY,
        (float) IVY_PLAYER_FRAME_SIZE,
        (float)-IVY_PLAYER_FRAME_SIZE
    };

    const Rectangle dst = {
        player->movement.position.x - offset,
        player->movement.position.y - offset,
        (float)IVY_PLAYER_FRAME_SIZE,
        (float)IVY_PLAYER_FRAME_SIZE
    };

    const float halfTile = IVY_TILE_SIZE * 0.5f;

    DrawTexturePro(atlasTex, src, dst, (Vector2){0, halfTile}, 0.0f, WHITE);
}