#ifndef IVY_PLAYER_INTERNAL_H
#define IVY_PLAYER_INTERNAL_H

#include "ivy/types.h"
#include "ivy/collision.h"
#include "raylib/raylib.h"

#define BASE_MOVE_DURATION      0.42f
#define RUN_SPEED_MULTIPLIER    0.595f
#define WALK_SPEED_MULTIPLIER   1.0f
#define DIR_INPUT_DELAY         6

// Attack config
#define ATTACK_ANIM_SPEED       0.08f   // Durasi per frame animasi attack (detik)
#define ATTACK_ANIM_START_COL   6       // Kolom mulai frame attack di spritesheet
#define ATTACK_ANIM_FRAMES      3       // Jumlah frame attack (col 6,7,8)
#define ATTACK_HIT_FRAME        1       // Frame ke-berapa hitbox aktif (0-based)
#define ATTACK_HITBOX_W         24.0f   // Lebar hitbox serangan
#define ATTACK_HITBOX_H         20.0f   // Tinggi hitbox serangan
#define ATTACK_HITBOX_REACH     20.0f   // Jarak hitbox dari pusat player
#define ATTACK_COOLDOWN         0.4f    // Cooldown setelah animasi selesai (detik)

typedef struct Player Player;

typedef enum {
    Z_BACK_HAIR = 0,
    Z_BODY,
    // Z_UNDERWEAR, // idk to throw this or nah
    Z_CLOTHES,
    Z_FACE,
    Z_FRON_HAIR,
    Z_ACCESSORY,
    Z_MAX
} PortraitZLayer;

typedef enum {
    ACTION_IDLE,
    ACTION_WALK,
    ACTION_RUN,
    ACTION_ATTACK
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

    // Attack animation state
    float   attackFrameTimer;
    u32     attackFrame;        // Frame saat ini dalam animasi attack (0..ATTACK_ANIM_FRAMES-1)
    bool    attackHitApplied;   // Sudah apply hit di frame ini?
    float   attackCooldown;     // Sisa waktu cooldown sebelum bisa attack lagi
} PlayerAnimation;


u32     GetSpriteRow(const Player *player);
u32     GetSpriteCol(const Player *player);         // Baru: kolom sprite (handle attack col offset)
float   GetMoveDuration(PlayerAction action);

bool    GetMovementInput(Vector2 *outDir, Direction *outFacing);
bool    DirectionKeyPressed(Direction dir);
bool    IsTileSolid(Vector2 tilePos, const Collision *collision, u32 tileSize);
bool    StartMoving(Player *player, Vector2 inputDir, Direction nextDir, bool isRunning, const Collision *collision, u32 tileSize);

void    UpdatePlayerMovement(Player *player, float frameTime, const Collision *collision, u32 tileSize);
void    UpdateAnimation(Player *player, float frameTime);
void    UpdateAttack(Player *player, float frameTime, const Collision *collision);

Rectangle GetAttackHitbox(const Player *player);    // Baru: hitbox area serangan

#endif