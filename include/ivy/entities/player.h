#ifndef IVY_ENTITIES_PLAYER_H
#define IVY_ENTITIES_PLAYER_H

#include "ivy/audio/wav.h"
#include "ivy/core/types.h"
#include "ivy/entities/door.h"
#include "ivy/scenes/options_private.h"
#include "ivy/systems/inventory.h"
#include "ivy/utils/forward.h"

#include "raylib/raylib.h"

#define IVY_TILE_SIZE 32
#define IVY_PLAYER_FRAME_SIZE 64

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    PLAYER_ACTION_IDLE,
    PLAYER_ACTION_WALK,
    PLAYER_ACTION_ATTACK
} IvyPlayerAction;

typedef struct {
    Vector2  position;          // 8
    Vector2  tilePosition;      // 8
    Vector2  targetTile;        // 8
    float    moveTimer;         // 4
    int      dirInputCount;     // 4
    bool     justTurned;        // 1
    bool     isMoving;          // 1
    char     padding[2];        // 2
} IvyPlayerMovement;            // 36

typedef struct {
    float    frameTimer;        // 4
    u32      currentFrame;      // 4
    u32      frameStep;         // 4

    float    attackFrameTimer;  // 4
    u32      attackFrame;       // 4
    float    attackCooldown;    // 4
    bool     attackHitApplied;  // 1
    char     padding[3];        // 3
} IvyPlayerAnimation;           // 28

typedef struct {
    RenderTexture2D atlas;      // 44
    IvyPlayerAction action;     // 4
    IvyDirection    direction;  // 4
    bool            atlasReady; // 1
    char            padding[3]; // 3
} IvyPlayerGraphic;             // 56

struct IvyPlayer {
    IvyInventory        inventory;      // 344
    char                padding[6];     // 6
    IvySound            stepSound[4];   // 192

    IvyPlayerGraphic    graphics;       // 56
    IvyPlayerMovement   movement;       // 36
    IvyPlayerAnimation  animation;      // 28
};                                      // 664

IvyPlayer      *Ivy_Player_Init(IvyArenaLinear *restrict arena, IvyAssetManager *restrict assetManager, Vector2 position);
void            Ivy_Player_Update(IvyPlayer *restrict player, const IvyCollusionMap *restrict collisionMap, IvyDoor *restrict doors, float deltaTime);
void            Ivy_Player_Render(const IvyPlayer *player);
void            Ivy_Player_Unload(IvyPlayer *player);

void            Ivy_Player_BakeAtlas(IvyGame *restrict game, IvySceneGameplayData *restrict gameplayData);
void            Ivy_Player_EquipItem(IvyGame *restrict game, IvySceneGameplayData *restrict gameplayData, u8 bagIndex);
void            Ivy_Player_UnequipItem(IvyGame *restrict game, IvySceneGameplayData *restrict gameplayData, u8 equipSlot);

Vector2         Ivy_Player_GetPosition(const IvyPlayer *player);
IvyInventory   *Ivy_Player_GetInventory(IvyPlayer *player);

#ifdef __cplusplus
}
#endif

#endif