#ifndef IVY_ENTITIES_PLAYER_H
#define IVY_ENTITIES_PLAYER_H

#include "ivy/audio/wav.h"
#include "ivy/core/types.h"
#include "ivy/scenes/options_private.h"
#include "ivy/systems/inventory.h"
#include "ivy/utils/forward.h"

#include "raylib/raylib.h"

#ifdef __cplusplus
extern "C" {
#endif

#define IVY_MOVE_DURATION   0.25f
#define IVY_TILE_SIZE       32
#define IVY_FRAME_SIZE      64
#define IVY_WALK_FRAMES     3
#define IVY_ANIM_SPEED      0.15f
#define IVY_DIR_DELAY       6

typedef enum {
    PLAYER_ACTION_IDLE,
    PLAYER_ACTION_WALK
} IvyPlayerAction;

typedef struct {
    Vector2 position;       //  8
    Vector2 tilePosition;   //  8
    Vector2 targetTile;     //  8
    float   moveTimer;      //  4
    int     dirInputCount;  //  4
    bool    justTurned;     //  1
    bool    isMoving;       //  1
    char    padding[2];     //  2
} IvyPlayerMovement;        // 36

typedef struct {
    float   frameTimer;     //  4
    u32     currentFrame;   //  4
    u32     frameStep;      //  4
    char    _pad[4];        //  4
} IvyPlayerAnimation;       // 16

typedef struct {
    RenderTexture2D atlas;       // 44
    IvyPlayerAction action;      //  4
    IvyDirection    direction;   //  4
    bool            atlasReady;  //  1
    char            padding[3];  //  3
} IvyPlayerGraphic;              // 116

struct IvyPlayer {
    IvyPlayerGraphic    graphics;       // 116
    IvyPlayerMovement   movement;       //  36
    IvyPlayerAnimation  animation;      //  16
    IvyInventory        inventory;      // 344
    IvySound            stepSound[4];
};

IvyPlayer  *Ivy_Player_Init(IvyArenaLinear *restrict arena, IvyAssetManager *restrict mgr, Vector2 pos);
void        Ivy_Player_Update(IvyPlayer *restrict player, float dt, const IvyCollusionMap *restrict collisionMap);
void        Ivy_Player_Render(const IvyPlayer *player);
void        Ivy_Player_Unload(IvyPlayer *player);

void        Ivy_Player_BakeAtlas(IvyGame *restrict game, IvySceneGameplayData *restrict gameplayData);
void        Ivy_Player_EquipItem(IvyGame *game, IvySceneGameplayData *gameplayData, u8 bagIndex);
void        Ivy_Player_UnequipItem(IvyGame *game, IvySceneGameplayData *gameplayData, u8 equipSlot);

Vector2       Ivy_Player_GetPosition(const IvyPlayer *player);
IvyInventory *Ivy_Player_GetInventory(IvyPlayer *player);

#ifdef __cplusplus
}
#endif

#endif