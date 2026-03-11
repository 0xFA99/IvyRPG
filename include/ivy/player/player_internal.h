#ifndef IVY_PLAYER_INTERNAL_H
#define IVY_PLAYER_INTERNAL_H

#include "ivy/types.h"
#include "ivy/collision.h"
#include "raylib/raylib.h"

#define BASE_MOVE_DURATION      0.42f
#define RUN_SPEED_MULTIPLIER    0.595f
#define WALK_SPEED_MULTIPLIER   1.0f
#define DIR_INPUT_DELAY         6

typedef struct Player Player;

typedef enum {
    ACTION_IDLE,
    ACTION_WALK,
    ACTION_RUN
} PlayerAction;

typedef struct {
    Texture2D       hairTexture;
    Texture2D       headTexture;
    Texture2D       bodyTexture;

    Texture2D       headPortrait;
    Texture2D       bodyPortrait;
    Texture2D       hairPortrait;
    Texture2D       eyesPortrait;
    Texture2D       mouthPortrait;

    PlayerAction    action;
    Direction       direction;
} PlayerGraphics;

typedef struct {
    Vector2     position;
    Vector2     tilePosition;
    Vector2     targetTilePosition;
    Rectangle   collisionBox;
    float       moveDuration;
    float       moveTimer;
    u32         dirInputCount;
    bool        isMoving;
    bool        justTurned;
    bool        isHoldingKey;
} PlayerMovement;

typedef struct {
    float   frameTimer;
    u32     currentFrame;
    u32     frameDirection;
} PlayerAnimation;


u32     GetSpriteRow(const Player *player);
float   GetMoveDuration(PlayerAction action);

bool    GetMovementInput(Vector2 *outDir, Direction *outFacing);
bool    DirectionKeyPressed(Direction dir);
bool    IsTileSolid(Vector2 tilePos, const Collision *collision, u32 tileSize);
bool    StartMoving(Player *player, Vector2 inputDir, Direction nextDir, bool isRunning, const Collision *collision, u32 tileSize);

void    UpdatePlayerMovement(Player *player, float frameTime, const Collision *collision, u32 tileSize);
void    UpdateAnimation(Player *player, float frameTime);


#endif