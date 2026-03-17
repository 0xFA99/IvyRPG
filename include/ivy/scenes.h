#ifndef IVY_SCENES_H
#define IVY_SCENES_H

#include "ivy/types.h"
#include "ivy/camera.h"
#include "ivy/collision.h"
#include "ivy/item.h"
#include "ivy/inventory/inventory_ui.h"
#include "ivy/npc/npc.h"

#include "raylib/raylib.h"

typedef struct Game     Game;
typedef struct Player   Player;
typedef struct Tilemap  Tilemap;
typedef struct Scene    Scene;

typedef enum {
    SCENE_TITLE,
    SCENE_GAMEPLAY,
    SCENE_OPTIONS,
    SCENE_EXIT
} SceneType;

typedef struct {
    Texture2D   background;
    u32         selectedIndex;
    float       cursorY;
} SceneTitleData;

typedef struct {
    GameCamera      gameCamera;
    Tilemap        *tilemap;
    Collision      *collision;
    Player         *player;
    ItemManager    *itemManager;
    InventoryUI     inventoryUI;
    NPCManager     *npcManager;
} SceneGameplayData;

typedef struct {
    int     selectedIndex;
    float   cursorY;
    u32     localeIndex;
} SceneOptionsData;

typedef struct Scene {
    SceneType type;

    union {
        SceneTitleData      *title;
        SceneGameplayData   *gameplay;
        SceneOptionsData    *options;
    } data;

    void (*Init)(Scene *s);
    void (*Update)(Game *game);
    void (*DrawWorld)(Game *game);
    void (*RebuildTextures)(Game *game);
    void (*DrawUI)(Game *game);
    void (*Unload)(Scene *s);
} Scene;

typedef struct SceneManager {
    Scene       activeScene;
    float       deltaTime;
    bool        sceneChanged;
    bool        isRunning;

    float       fadeAlpha;
    bool        fadingOut;
    bool        transitioning;
    SceneType   pendingScene;
} SceneManager;

void UpdateScene(SceneManager *sm);
void BeginSceneTransition(SceneManager *sm, SceneType nextScene);
void UpdateSceneTransition(SceneManager *sm);
void DrawSceneTransition(const SceneManager *sm);

void SceneTitleInit(Scene *s);
void SceneTitleUpdate(Game *game);
void SceneTitleDrawWorld(Game *game);
void SceneTitleRebuildTextures(Game *game);
void SceneTitleDrawUI(Game *game);
void SceneTitleUnload(Scene *s);

void SceneGameplayInit(Scene *s);
void SceneGameplayUpdate(Game *game);
void SceneGameplayDrawWorld(Game *game);
void SceneGameplayRebuildTextures(Game *game);
void SceneGameplayDrawUI(Game *game);
void SceneGameplayUnload(Scene *s);

void SceneOptionsInit(Scene *s);
void SceneOptionsUpdate(Game *game);
void SceneOptionsDrawWorld(Game *game);
void SceneOptionsRebuildTextures(Game *game);
void SceneOptionsDrawUI(Game *game);
void SceneOptionsUnload(Scene *s);

#endif