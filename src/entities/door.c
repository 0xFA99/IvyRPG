#include "ivy/entities/door.h"
#include "ivy/entities/player.h"
#include "ivy/graphics/gfx.h"
#include "ivy/utils/file_ids.h"
#include "raylib/rlgl.h"

#ifndef IVY_TILE_SIZE
#define IVY_TILE_SIZE 32
#endif

static const Rectangle DOOR_RECTS[IVY_DOOR_MAX] = {
    [IVY_DOOR_LEFT]  = { .x = 0,  .y = 0, .width = 32, .height = 64 },
    [IVY_DOOR_RIGHT] = { .x = 32, .y = 0, .width = 32, .height = 64 },
    [IVY_DOOR_BOTH]  = { .x = 0,  .y = 0, .width = 64, .height = 64 }
};

IvyDoor *Ivy_Door_Init(IvyArenaLinear *restrict arena, IvyAssetManager *restrict assetManager,
                              const IvyDoorSide side, const Vector2 position)
{
    IvyDoor *door = Ivy_Arena_LinearAllocZero(arena, sizeof(IvyDoor));
    door->texture = Ivy_Gfx_LoadTextureDDS(assetManager, ASSET_TEXTURES_DOORS_DDS);

    door->side = side;
    door->position = (Vector2) {
        .x = position.x * IVY_TILE_SIZE,
        .y = position.y * IVY_TILE_SIZE
    };
    door->isOpened = false;

    Rectangle baseRect = DOOR_RECTS[door->side];
    door->rect = baseRect;

    door->sound[IVY_DOOR_STATE_CLOSED] = Ivy_Audio_LoadSoundWav(arena, assetManager, ASSET_AUDIO_DOOR_CLOSE_WAV);
    door->sound[IVY_DOOR_STATE_OPENED] = Ivy_Audio_LoadSoundWav(arena, assetManager, ASSET_AUDIO_DOOR_OPEN_WAV);

    return door;
}

void Ivy_Door_Update(IvyDoor *door)
{
    if (door == NULL) return;

    Rectangle baseRect = DOOR_RECTS[door->side];

    if (door->isOpened)
    {
        door->rect = (Rectangle) {
            .x = baseRect.x,
            .y = 64,
            .width = baseRect.width,
            .height = 76
        };
        Ivy_Audio_PlayAudioBuffer(door->sound[IVY_DOOR_STATE_OPENED].data.stream.buffer);
    }
    else
    {
        door->rect = baseRect;
        Ivy_Audio_PlayAudioBuffer(door->sound[IVY_DOOR_STATE_CLOSED].data.stream.buffer);
    }
}

void Ivy_Door_Draw(const IvyDoor *door)
{
    if (door == NULL || door->texture.id <= 0) return;

    DrawTexturePro(door->texture,
        door->rect,
        (Rectangle) {
            .x = door->position.x,
            .y = (door->isOpened) ? door->position.y - 12 : door->position.y,
            .width = door->rect.width,
            .height = door->rect.height
        },
        (Vector2){0},
        0.0f,
        WHITE);
}

void Ivy_Door_Open(IvyDoor *door)
{
    if (door != NULL) door->isOpened = true;
}

void Ivy_Door_Close(IvyDoor *door)
{
    if (door != NULL) door->isOpened = false;
}

void Ivy_Door_Toggle(IvyDoor *door)
{
    if (door != NULL) door->isOpened = !door->isOpened;
}

void Ivy_Door_SetPosition(IvyDoor *door, Vector2 newPos)
{
    if (door != NULL) door->position = newPos;
}

Rectangle Ivy_Door_GetCollisionRect(const IvyDoor *door)
{
    if (door == NULL) return (Rectangle){0};

    return (Rectangle) {
        .x = door->position.x,
        .y = door->position.y + IVY_TILE_SIZE,
        .width = IVY_TILE_SIZE,
        .height = IVY_TILE_SIZE
    };
}

void Ivy_Door_Unload(IvyDoor *door)
{
    if (door != NULL && door->texture.id > 0)
    {
        rlUnloadTexture(door->texture.id);
        door->texture.id = 0;
    }
}

bool Ivy_Door_IsSolid(const IvyDoor *door) {
    return (door != NULL && !door->isOpened);
}

void Ivy_Door_GetTileRect(const IvyDoor *door, int *outTileX, int *outTileY, int *outTileW, int *outTileH) {
    if (!door) return;
    *outTileX = (int)(door->position.x / IVY_TILE_SIZE);
    *outTileY = (int)(door->position.y / IVY_TILE_SIZE) + 1;

    const Rectangle baseRect = DOOR_RECTS[door->side];
    *outTileW = (int)(baseRect.width  / IVY_TILE_SIZE);
    *outTileH = (int)(baseRect.height / IVY_TILE_SIZE) - 1;
}

bool Ivy_Door_CanInteract(const IvyDoor *door, const IvyPlayer *player) {
    if (!door || !player) return false;

    int doorX, doorY, doorW, doorH;
    Ivy_Door_GetTileRect(door, &doorX, &doorY, &doorW, &doorH);

    Vector2 playerTile = player->movement.tilePosition;

    for (int dy = 0; dy < doorH; dy++) {
        for (int dx = 0; dx < doorW; dx++) {
            int tileX = doorX + dx;
            int tileY = doorY + dy;
            int deltaX = (int)playerTile.x - tileX;
            int deltaY = (int)playerTile.y - tileY;

            if (abs(deltaX) + abs(deltaY) == 1) {
                IvyDirection neededDir;
                if (deltaX == 1) neededDir = IVY_DIRECTION_LEFT;
                else if (deltaX == -1) neededDir = IVY_DIRECTION_RIGHT;
                else if (deltaY == 1) neededDir = IVY_DIRECTION_UP;
                else neededDir = IVY_DIRECTION_DOWN;

                if (player->graphics.direction == neededDir)
                    return true;
            }
        }
    }
    return false;
}

void Ivy_Door_Interact(IvyDoor *door) {
    if (door) Ivy_Door_Toggle(door);
}