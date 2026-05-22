#ifndef IVY_ENTITIES_PLAYER_H
#define IVY_ENTITIES_PLAYER_H

#include "ivy/core/types.h"
#include "ivy/utils/forward.h"

#include "raylib/raylib.h"

#ifdef __cplusplus
extern "C" {
#endif

#define IVY_MOVE_DURATION  0.25f
#define IVY_TILE_SIZE      32
#define IVY_FRAME_SIZE     64
#define IVY_WALK_FRAMES    3
#define IVY_ANIM_SPEED     0.15f
#define IVY_DIR_DELAY      6

typedef enum {
    PLAYER_ACTION_IDLE,
    PLAYER_ACTION_WALK
} IvyPlayerAction;

typedef struct {
    Vector2 position;       // 8
    Vector2 tilePosition;   // 8
    Vector2 targetTile;     // 8
    float   moveTimer;      // 4
    int     dirInputCount;  // 4
    bool    justTurned;     // 1
    bool    isMoving;       // 1
    char    padding[2];     // 2
} IvyPlayerMovement;

typedef struct {
    float frameTimer;       // 4
    u32   currentFrame;     // 4
    u32   frameStep;        // 4
} IvyPlayerAnimation;       // 12

typedef struct {
    RenderTexture2D atlas; // 44

    Texture2D hair;
    Texture2D innerTop;
    Texture2D innerBottom;
    Texture2D outerTop;
    Texture2D outerBottom;
    Texture2D armMain;
    Texture2D armSub;
    Texture2D accHead;
    Texture2D accBody;
    Texture2D extraBack;
    Texture2D extraFront;
} IvyEquipment;

typedef struct {
    Texture2D       baseBody;       // 20
    Texture2D       baseHead;       // 20
    Texture2D       baseHair;       // 20

    IvyPlayerAction action;         // 4
    IvyDirection    direction;      // 4
    bool            atlasReady;     // 1
    char            padding[3];     // 3
} IvyPlayerGraphic;                 // 72

IvyPlayer *Ivy_Player_Init(IvyArenaLinear *restrict arena, IvyAssetManager *restrict mgr, Vector2 pos);
void       Ivy_Player_Update(IvyPlayer *restrict player, float dt, const IvyCollusionMap *restrict collisionMap);
void       Ivy_Player_Render(const IvyPlayer *player);
void       Ivy_Player_Unload(IvyPlayer *player);

void       Ivy_Player_EquipItem(IvyPlayer *restrict player, IvyAssetManager *restrict assetManager, IvyItemManager *restrict itemManager);

void       Ivy_Player_BakeAtlas(IvyPlayer *player);
Vector2    Ivy_Player_GetPosition(const IvyPlayer *player);

#ifdef __cplusplus
}
#endif

#endif